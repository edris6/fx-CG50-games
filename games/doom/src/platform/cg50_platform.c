// cg50_platform.c -- fx-CG50 hardware layer for the Doom port (uses gint).
//
//   video  : 8-bit 320x200 frame -> RGB565 -> gint VRAM, centered
//   input  : keypad state diffed each poll into Doom key press/release events
//   time   : 4 ms hardware timer feeding a millisecond counter
//   memory : zone heap allocation with fallback sizes
//   errors : message on screen, then back to the OS menu

#define CG50_NO_REMAP
#include "cg50_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gint/display.h>
#include <gint/gint.h>
#include <gint/keyboard.h>
#include <gint/timer.h>

#include "cg50_platform.h"

#include "doomgeneric.h"

// Doom key codes. These mirror src/doom/doomkeys.h, which cannot be included
// here: it defines KEY_F1..KEY_F12 and would collide with gint's key names.
// tests/host/check_keys.c verifies that the values below match doomkeys.h.
#define DK_RIGHTARROW 0xae
#define DK_LEFTARROW  0xac
#define DK_UPARROW    0xad
#define DK_DOWNARROW  0xaf
#define DK_STRAFE_L   0xa0
#define DK_STRAFE_R   0xa1
#define DK_USE        0xa2
#define DK_FIRE       0xa3
#define DK_ESCAPE     27
#define DK_ENTER      13
#define DK_TAB        9
#define DK_BACKSPACE  0x7f
#define DK_PAUSE      0xff
#define DK_EQUALS     0x3d
#define DK_MINUS      0x2d
#define DK_RSHIFT     (0x80 + 0x36)
#define DK_RALT       (0x80 + 0x38)

#ifndef DWIDTH
#define DWIDTH 396
#endif
#ifndef DHEIGHT
#define DHEIGHT 224
#endif

#define DOOM_W 320
#define DOOM_H 200
// Center the Doom frame in the 396x224 VRAM.
#define X0 ((DWIDTH - DOOM_W) / 2)
#define Y0 ((DHEIGHT - DOOM_H) / 2)

pixel_t *DG_ScreenBuffer = NULL;

// ---------------------------------------------------------------------------
// Video
// ---------------------------------------------------------------------------

static char last_log[96];
static uint32_t fps_frames, fps_last_ms;
static int fps_value;

void cg_log(const char *msg)
{
    strncpy(last_log, msg, sizeof(last_log) - 1);
    last_log[sizeof(last_log) - 1] = '\0';
}

void cg_video_init(void)
{
    dclear(C_BLACK);
    dupdate();
}

void cg_video_present(const uint8_t *frame, const uint16_t *pal)
{
    int y, x;

    for (y = 0; y < DOOM_H; ++y)
    {
        const uint8_t *src = frame + y * DOOM_W;
        // X0 and DWIDTH are even, so each row start is 4-byte aligned; this
        // lets us write two pixels per 32-bit store.
        uint32_t *dst = (uint32_t *) (gint_vram + (y + Y0) * DWIDTH + X0);

        for (x = 0; x < DOOM_W; x += 4)
        {
            uint32_t p0 = pal[src[x]];
            uint32_t p1 = pal[src[x + 1]];
            uint32_t p2 = pal[src[x + 2]];
            uint32_t p3 = pal[src[x + 3]];

            // SH-4 is little-endian: the lower address is the low half.
            dst[0] = p0 | (p1 << 16);
            dst[1] = p2 | (p3 << 16);
            dst += 2;
        }
    }

#ifdef DOOM_SHOW_FPS
    fps_frames++;
    if (cg_millis() - fps_last_ms >= 1000)
    {
        fps_value = (int) fps_frames;
        fps_frames = 0;
        fps_last_ms = cg_millis();
    }
    drect(0, 0, 80, Y0 - 1, C_BLACK);
    dprint(2, 1, C_WHITE, "%d fps", fps_value);
#endif

    dupdate();
}

// ---------------------------------------------------------------------------
// Time
// ---------------------------------------------------------------------------

static volatile uint32_t millis_counter;

#define TICK_US 4000

static int timer_tick(volatile uint32_t *counter)
{
    *counter += TICK_US / 1000;
    return TIMER_CONTINUE;
}

void cg_time_init(void)
{
    int t = timer_configure(TIMER_ANY, TICK_US, GINT_CALL(timer_tick, &millis_counter));
    if (t < 0)
        cg_fatal("No free hardware timer.");
    timer_start(t);
}

uint32_t cg_millis(void)
{
    return millis_counter;
}

uint32_t DG_GetTicksMs(void)
{
    return cg_millis();
}

void DG_SleepMs(uint32_t ms)
{
    uint32_t end = cg_millis() + ms;
    while ((int32_t) (cg_millis() - end) < 0)
        ;   // spin; the 4 ms timer interrupt advances cg_millis()
}

void DG_Init(void)
{
}

void DG_DrawFrame(void)
{
}

void DG_SetWindowTitle(const char *title)
{
    (void) title;
}

// ---------------------------------------------------------------------------
// Input
//
// Default layout (see README for the full table):
//   arrows  move / turn      EXE   fire (and menu "Enter")
//   ALPHA   use / open       SHIFT run
//   OPTN    strafe modifier  F1/F2 strafe left/right
//   F3      automap          1..7   weapons
//   EXIT    menu / back      DEL    menu backspace
//   F5/F6   "y" / "n" for confirmation prompts
// ---------------------------------------------------------------------------

typedef struct
{
    int gint_key;
    unsigned char doom_key;
} keymap_t;

static const keymap_t keymap[] = {
    { KEY_UP,    DK_UPARROW },
    { KEY_DOWN,  DK_DOWNARROW },
    { KEY_LEFT,  DK_LEFTARROW },
    { KEY_RIGHT, DK_RIGHTARROW },
    { KEY_EXE,   DK_FIRE },
    { KEY_EXE,   DK_ENTER },
    { KEY_ALPHA, DK_USE },
    { KEY_SHIFT, DK_RSHIFT },
    { KEY_OPTN,  DK_RALT },
    { KEY_F1,    DK_STRAFE_L },
    { KEY_F2,    DK_STRAFE_R },
    { KEY_F3,    DK_TAB },
    { KEY_EXIT,  DK_ESCAPE },
    { KEY_DEL,   DK_BACKSPACE },
    { KEY_F5,    'y' },
    { KEY_F6,    'n' },
    { KEY_1, '1' }, { KEY_2, '2' }, { KEY_3, '3' }, { KEY_4, '4' },
    { KEY_5, '5' }, { KEY_6, '6' }, { KEY_7, '7' },
    { KEY_ADD,   DK_EQUALS },
    { KEY_SUB,   DK_MINUS },
    { KEY_0,     DK_PAUSE },
};

#define NUM_KEYS ((int) (sizeof(keymap) / sizeof(keymap[0])))
#define QUEUE_SIZE 64

static unsigned char key_down_state[NUM_KEYS];
static unsigned short queue[QUEUE_SIZE];
static int q_head, q_tail;
static int need_poll = 1;

static void queue_push(int pressed, unsigned char key)
{
    int next = (q_head + 1) % QUEUE_SIZE;

    if (next == q_tail)
        return;     // full: drop the event

    queue[q_head] = (unsigned short) ((pressed << 8) | key);
    q_head = next;
}

static void poll_keys(void)
{
    int i;

    clearevents();

    for (i = 0; i < NUM_KEYS; ++i)
    {
        int down = keydown(keymap[i].gint_key) ? 1 : 0;

        if (down != key_down_state[i])
        {
            key_down_state[i] = (unsigned char) down;
            queue_push(down, keymap[i].doom_key);
        }
    }
}

int DG_GetKey(int *pressed, unsigned char *key)
{
    if (q_head == q_tail && need_poll)
    {
        poll_keys();
        need_poll = 0;
    }

    if (q_head == q_tail)
    {
        need_poll = 1;
        return 0;
    }

    *pressed = (queue[q_tail] >> 8) & 1;
    *key = (unsigned char) (queue[q_tail] & 0xFF);
    q_tail = (q_tail + 1) % QUEUE_SIZE;
    return 1;
}

// ---------------------------------------------------------------------------
// Memory
// ---------------------------------------------------------------------------

#define HEADROOM (96 * 1024)    // left free for Doom's small mallocs

void *cg_alloc_zone(size_t max_bytes, size_t min_bytes, size_t *got)
{
    size_t size = max_bytes;

    while (size >= min_bytes)
    {
        void *p = malloc(size);

        if (p != NULL)
        {
            void *probe = malloc(HEADROOM);

            if (probe != NULL)
            {
                free(probe);
                *got = size;
                return p;
            }

            free(p);
        }

        size -= 64 * 1024;
    }

    return NULL;
}

// ---------------------------------------------------------------------------
// Errors and exit
// ---------------------------------------------------------------------------

void cg_quit(void)
{
    gint_osmenu();
    for (;;)
        ;
}

void cg_fatal(const char *msg)
{
    enum { COLS = 44 };
    int y = 22;
    size_t i, n = strlen(msg);

    dclear(C_BLACK);
    dtext(4, 4, C_WHITE, "DOOM stopped:");

    for (i = 0; i < n && y < 190; i += COLS, y += 12)
    {
        char line[COLS + 1];
        size_t len = n - i < COLS ? n - i : COLS;

        memcpy(line, msg + i, len);
        line[len] = '\0';
        dtext(4, y, C_WHITE, line);
    }

    dtext(4, 205, C_WHITE, "Press any key to return to the menu");
    dupdate();
    getkey();
    cg_quit();
}
