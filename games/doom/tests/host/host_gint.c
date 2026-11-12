// Host implementation of the gint subset used by the port.
//   - BFile maps "\\fls0\NAME" to $DOOM_HOST_DIR/NAME on the host FS.
//   - The screen is dumped as PPM files at the frame numbers in $DOOM_DUMP.
//   - Keys can be scripted: $DOOM_KEYS="frame:key:down,..." (see below).
#include <gint/display.h>
#include <gint/gint.h>
#include <gint/keyboard.h>
#include <gint/timer.h>
#include <gint/bfile.h>

#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static uint16_t vram_store[DWIDTH * DHEIGHT];
uint16_t *gint_vram = vram_store;
static int frame_no;
static int max_frames = 400;

void dclear(int c) { for (int i = 0; i < DWIDTH * DHEIGHT; i++) vram_store[i] = c; }
void drect(int x1, int y1, int x2, int y2, int c)
{ for (int y = y1; y <= y2; y++) for (int x = x1; x <= x2; x++) vram_store[y * DWIDTH + x] = c; }
void dtext(int x, int y, int fg, const char *s) { fprintf(stderr, "[dtext %d,%d] %s\n", x, y, s); }
void dprint(int x, int y, int fg, const char *fmt, ...) { (void) x; (void) y; (void) fg; (void) fmt; }

static void dump_ppm(const char *name)
{
    FILE *f = fopen(name, "wb");
    fprintf(f, "P6\n%d %d\n255\n", DWIDTH, DHEIGHT);
    for (int i = 0; i < DWIDTH * DHEIGHT; i++) {
        uint16_t p = vram_store[i];
        fputc(((p >> 11) & 31) << 3, f);
        fputc(((p >> 5) & 63) << 2, f);
        fputc((p & 31) << 3, f);
    }
    fclose(f);
}

void dupdate(void)
{
    frame_no++;
    const char *d = getenv("DOOM_DUMP");   // "10,50,200"
    if (d) {
        char *copy = strdup(d), *tok = strtok(copy, ",");
        while (tok) {
            if (atoi(tok) == frame_no) {
                char name[128];
                snprintf(name, sizeof name, "%s/frame%04d.ppm", getenv("DOOM_OUT") ? getenv("DOOM_OUT") : ".", frame_no);
                dump_ppm(name);
                fprintf(stderr, "[host] dumped %s\n", name);
            }
            tok = strtok(NULL, ",");
        }
        free(copy);
    }
    if (getenv("DOOM_FRAMES")) max_frames = atoi(getenv("DOOM_FRAMES"));
    if (frame_no >= max_frames) { fprintf(stderr, "[host] reached %d frames, exiting\n", frame_no); exit(0); }
}

void gint_osmenu(void) { fprintf(stderr, "[host] gint_osmenu()\n"); exit(0); }
int getkey(void) { return 0; }

// Scripted keys: "frame:keyname:1" meaning from that frame on key is down(1)/up(0)
static int key_state[64];
void clearevents(void)
{
    const char *k = getenv("DOOM_KEYS");   // e.g. "30:EXE:1,32:EXE:0"
    if (!k) return;
    char *copy = strdup(k), *save = NULL;
    for (char *tok = strtok_r(copy, ",", &save); tok; tok = strtok_r(NULL, ",", &save)) {
        int fr, v; char name[16];
        if (sscanf(tok, "%d:%15[A-Z0-9]:%d", &fr, name, &v) != 3) continue;
        if (fr > frame_no) continue;
        int code = -1;
        if (!strcmp(name, "EXE")) code = KEY_EXE;
        else if (!strcmp(name, "UP")) code = KEY_UP;
        else if (!strcmp(name, "DOWN")) code = KEY_DOWN;
        else if (!strcmp(name, "LEFT")) code = KEY_LEFT;
        else if (!strcmp(name, "RIGHT")) code = KEY_RIGHT;
        else if (!strcmp(name, "EXIT")) code = KEY_EXIT;
        else if (!strcmp(name, "ALPHA")) code = KEY_ALPHA;
        if (code >= 0) key_state[code] = v;
    }
    free(copy);
}
int keydown(int key) { return key_state[key]; }

// --- timer: a thread that bumps the callback every delay_us ---------------
struct tm_args { uint64_t us; gint_call_t cb; };
static void *tm_thread(void *p)
{
    struct tm_args *a = p;
    for (;;) { usleep((useconds_t) a->us); a->cb.fn(a->cb.arg); }
    return NULL;
}
static struct tm_args tm_a;
int timer_configure(int timer, uint64_t delay_us, gint_call_t cb)
{ setvbuf(stdout, NULL, _IONBF, 0); tm_a.us = delay_us; tm_a.cb = cb; return 0; }
int timer_start(int timer)
{ pthread_t t; pthread_create(&t, NULL, tm_thread, &tm_a); return 0; }

// --- BFile ----------------------------------------------------------------
static void path8(const uint16_t *p, char *out, size_t n)
{
    char tmp[256]; size_t k = 0;
    while (*p && k < sizeof tmp - 1) tmp[k++] = (char) *p++;
    tmp[k] = 0;
    const char *s = tmp;
    if (!strncmp(s, "\\\\fls0\\", 7)) s += 7;
    const char *dir = getenv("DOOM_HOST_DIR") ? getenv("DOOM_HOST_DIR") : ".";
    snprintf(out, n, "%s/%s", dir, s);
}
int BFile_Open(const uint16_t *path, int mode)
{ char p[512]; path8(path, p, sizeof p); return open(p, mode == BFile_ReadOnly ? O_RDONLY : O_WRONLY); }
int BFile_Close(int fd) { return close(fd); }
int BFile_Create(const uint16_t *path, int type, int *size)
{ char p[512]; path8(path, p, sizeof p); int fd = open(p, O_CREAT | O_WRONLY, 0644); if (fd < 0) return -1; close(fd); return 0; }
int BFile_Remove(const uint16_t *path) { char p[512]; path8(path, p, sizeof p); return unlink(p); }
int BFile_Size(int fd) { off_t cur = lseek(fd, 0, SEEK_CUR), e = lseek(fd, 0, SEEK_END); lseek(fd, cur, SEEK_SET); return (int) e; }
int BFile_Read(int fd, void *buf, int size, int whence)
{ return (int) pread(fd, buf, size, whence); }
int BFile_Write(int fd, const void *buf, int size) { return (int) write(fd, buf, size); }

// Debug aid: print a backtrace whenever the process exits.
#include <execinfo.h>
static void bt_at_exit(void)
{
    void *frames[32];
    int n = backtrace(frames, 32);
    fprintf(stderr, "[host] exit backtrace:\n");
    backtrace_symbols_fd(frames, n, 2);
}
__attribute__((constructor)) static void install_bt(void)
{
    if (getenv("DOOM_TRACE_EXIT")) atexit(bt_at_exit);
}
