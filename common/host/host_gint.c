// Host implementation of the gint subset used by this repo, so the games and
// the Doom port can be built, run headlessly and screenshotted on a desktop.
//
// Environment variables:
//   HOST_FRAMES=n      exit after n calls to dupdate()      (default 400)
//   HOST_DUMP=a,b,c    write PPM screenshots at those frames
//   HOST_OUT=dir       directory for screenshots            (default .)
//   HOST_KEYS=script   scripted keys: "frame:NAME:1,frame:NAME:0,..."
//                      NAME is a gint key without the KEY_ prefix (EXE, UP...)
//   HOST_DIR=dir       directory standing in for \\fls0\
//   HOST_TRACE_EXIT=1  print a backtrace on exit
// This file must be compiled WITHOUT the Doom compat header.
#include <gint/bfile.h>
#include <gint/display.h>
#include <gint/gint.h>
#include <gint/keyboard.h>
#include <gint/timer.h>

#include <execinfo.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static uint16_t vram_store[DWIDTH * DHEIGHT];
uint16_t *gint_vram = vram_store;
static int frame_no;

void dclear(int c) { for (int i = 0; i < DWIDTH * DHEIGHT; i++) vram_store[i] = c; }
void drect(int x1, int y1, int x2, int y2, int c)
{
    for (int y = y1; y <= y2; y++)
        for (int x = x1; x <= x2; x++)
            if (x >= 0 && y >= 0 && x < DWIDTH && y < DHEIGHT) vram_store[y * DWIDTH + x] = c;
}
void dtext(int x, int y, int fg, const char *s) { fprintf(stderr, "[dtext %d,%d] %s\n", x, y, s); }
void dprint(int x, int y, int fg, const char *fmt, ...) { (void) x; (void) y; (void) fg; (void) fmt; }

static void dump_ppm(const char *name)
{
    FILE *f = fopen(name, "wb");
    if (!f) { perror(name); return; }
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
    const char *d = getenv("HOST_DUMP");
    if (d) {
        char *copy = strdup(d), *save = NULL;
        for (char *tok = strtok_r(copy, ",", &save); tok; tok = strtok_r(NULL, ",", &save)) {
            if (atoi(tok) == frame_no) {
                char name[1024];
                const char *out = getenv("HOST_OUT") ? getenv("HOST_OUT") : ".";
                snprintf(name, sizeof name, "%s/frame%04d.ppm", out, frame_no);
                dump_ppm(name);
                fprintf(stderr, "[host] dumped %s\n", name);
            }
        }
        free(copy);
    }
    int max_frames = getenv("HOST_FRAMES") ? atoi(getenv("HOST_FRAMES")) : 400;
    if (frame_no >= max_frames) { fprintf(stderr, "[host] reached %d frames, exiting\n", frame_no); exit(0); }
}

void gint_osmenu(void) { fprintf(stderr, "[host] gint_osmenu()\n"); exit(0); }
int getkey(void) { return 0; }

// ---- scripted keyboard -----------------------------------------------------
static const struct { const char *name; int code; } key_names[] = {
    {"F1",KEY_F1},{"F2",KEY_F2},{"F3",KEY_F3},{"F4",KEY_F4},{"F5",KEY_F5},{"F6",KEY_F6},
    {"SHIFT",KEY_SHIFT},{"OPTN",KEY_OPTN},{"VARS",KEY_VARS},{"MENU",KEY_MENU},
    {"LEFT",KEY_LEFT},{"UP",KEY_UP},{"DOWN",KEY_DOWN},{"RIGHT",KEY_RIGHT},
    {"ALPHA",KEY_ALPHA},{"EXIT",KEY_EXIT},{"DEL",KEY_DEL},{"EXE",KEY_EXE},
    {"ADD",KEY_ADD},{"SUB",KEY_SUB},{"MUL",KEY_MUL},{"DIV",KEY_DIV},
    {"0",KEY_0},{"1",KEY_1},{"2",KEY_2},{"3",KEY_3},{"4",KEY_4},{"5",KEY_5},
    {"6",KEY_6},{"7",KEY_7},{"8",KEY_8},{"9",KEY_9},{"DOT",KEY_DOT},
};
static int key_state[KEY_COUNT_];

void clearevents(void)
{
    const char *k = getenv("HOST_KEYS");
    if (!k) return;
    char *copy = strdup(k), *save = NULL;
    for (char *tok = strtok_r(copy, ",", &save); tok; tok = strtok_r(NULL, ",", &save)) {
        int fr, v; char name[16];
        if (sscanf(tok, "%d:%15[A-Z0-9]:%d", &fr, name, &v) != 3 || fr > frame_no) continue;
        for (unsigned i = 0; i < sizeof key_names / sizeof key_names[0]; i++)
            if (!strcmp(name, key_names[i].name)) key_state[key_names[i].code] = v;
    }
    free(copy);
}
int keydown(int key) { return key >= 0 && key < KEY_COUNT_ ? key_state[key] : 0; }

// ---- timer thread ----------------------------------------------------------
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

// ---- BFile -----------------------------------------------------------------
static void path8(const uint16_t *p, char *out, size_t n)
{
    char tmp[256]; size_t k = 0;
    while (*p && k < sizeof tmp - 1) tmp[k++] = (char) *p++;
    tmp[k] = 0;
    const char *s = tmp;
    if (!strncmp(s, "\\\\fls0\\", 7)) s += 7;
    const char *dir = getenv("HOST_DIR") ? getenv("HOST_DIR") : ".";
    snprintf(out, n, "%s/%s", dir, s);
}
int BFile_Open(const uint16_t *path, int mode)
{ char p[512]; path8(path, p, sizeof p); return open(p, mode == BFile_ReadOnly ? O_RDONLY : O_WRONLY); }
int BFile_Close(int fd) { return close(fd); }
int BFile_Create(const uint16_t *path, int type, int *size)
{ char p[512]; path8(path, p, sizeof p); int fd = open(p, O_CREAT | O_WRONLY, 0644); if (fd < 0) return -1; close(fd); return 0; }
int BFile_Remove(const uint16_t *path) { char p[512]; path8(path, p, sizeof p); return unlink(p); }
int BFile_Size(int fd) { off_t cur = lseek(fd, 0, SEEK_CUR), e = lseek(fd, 0, SEEK_END); lseek(fd, cur, SEEK_SET); return (int) e; }
int BFile_Read(int fd, void *buf, int size, int whence) { return (int) pread(fd, buf, size, whence); }
int BFile_Write(int fd, const void *buf, int size) { return (int) write(fd, buf, size); }

static void bt_at_exit(void)
{
    void *frames[32];
    int n = backtrace(frames, 32);
    fprintf(stderr, "[host] exit backtrace:\n");
    backtrace_symbols_fd(frames, n, 2);
}
__attribute__((constructor)) static void install_bt(void)
{
    if (getenv("HOST_TRACE_EXIT")) atexit(bt_at_exit);
}
