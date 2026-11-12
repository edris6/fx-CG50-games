// cgx.c -- implementation of the arcade toolkit on top of gint.
#include "cgx.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/timer.h>

#ifndef DWIDTH
#define DWIDTH 396
#endif
#ifndef DHEIGHT
#define DHEIGHT 224
#endif

// Centered 384x216 window inside the 396x224 VRAM.
#define OX ((DWIDTH - CGX_W) / 2)
#define OY ((DHEIGHT - CGX_H) / 2)

extern const unsigned char cgx_font5x7[95][7];

// ---------------------------------------------------------------------------
// Time
// ---------------------------------------------------------------------------

static volatile uint32_t ms_counter;

static int tick_cb(volatile uint32_t *counter)
{
    *counter += 2;
    return TIMER_CONTINUE;
}

void cgx_init(void)
{
    int t = timer_configure(TIMER_ANY, 2000, GINT_CALL(tick_cb, &ms_counter));
    if (t >= 0)
        timer_start(t);
    cgx_seed(0x2545F491u);
    dclear(C_BLK);
}

uint32_t cgx_ms(void)
{
    return ms_counter;
}

int cgx_steps(uint32_t *last_ms, int *acc_ms, int step_ms, int max_steps)
{
    uint32_t now;
    int steps;

    for (;;)
    {
        now = cgx_ms();
        if (*last_ms == 0)
            *last_ms = now;
        *acc_ms += (int) (now - *last_ms);
        *last_ms = now;

        steps = *acc_ms / step_ms;
        if (steps > 0)
            break;
    }

    if (steps > max_steps)
    {
        // We fell behind (slow frame): drop time instead of spiralling.
        *acc_ms = 0;
        return max_steps;
    }

    *acc_ms -= steps * step_ms;
    return steps;
}

// ---------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------

void cgx_clear(uint16_t c)
{
    cgx_rect(0, 0, CGX_W, CGX_H, c);
}

void cgx_rect(int x, int y, int w, int h, uint16_t c)
{
    int x1 = x + w, y1 = y + h, yy, xx;

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x1 > CGX_W) x1 = CGX_W;
    if (y1 > CGX_H) y1 = CGX_H;

    for (yy = y; yy < y1; ++yy)
    {
        uint16_t *p = gint_vram + (yy + OY) * DWIDTH + OX;
        for (xx = x; xx < x1; ++xx)
            p[xx] = c;
    }
}

void cgx_frame(int x, int y, int w, int h, int t, uint16_t c)
{
    cgx_rect(x, y, w, t, c);
    cgx_rect(x, y + h - t, w, t, c);
    cgx_rect(x, y, t, h, c);
    cgx_rect(x + w - t, y, t, h, c);
}

void cgx_pixel(int x, int y, uint16_t c)
{
    if (x >= 0 && y >= 0 && x < CGX_W && y < CGX_H)
        gint_vram[(y + OY) * DWIDTH + x + OX] = c;
}

void cgx_circle(int cx, int cy, int r, uint16_t c)
{
    int dy, dx;

    for (dy = -r; dy <= r; ++dy)
        for (dx = -r; dx <= r; ++dx)
            if (dx * dx + dy * dy <= r * r + r / 2)
                cgx_pixel(cx + dx, cy + dy, c);
}

int cgx_text_w(const char *s, int scale)
{
    int n = (int) strlen(s);
    return n == 0 ? 0 : (n * 6 - 1) * scale;
}

void cgx_text(int x, int y, int scale, uint16_t c, const char *s)
{
    for (; *s; ++s, x += 6 * scale)
    {
        unsigned char ch = (unsigned char) *s;
        const unsigned char *g;
        int row, col;

        if (ch < 32 || ch > 126)
            ch = '?';
        g = cgx_font5x7[ch - 32];

        for (row = 0; row < 7; ++row)
            for (col = 0; col < 5; ++col)
                if (g[row] & (0x10 >> col))
                    cgx_rect(x + col * scale, y + row * scale, scale, scale, c);
    }
}

void cgx_textf(int x, int y, int scale, uint16_t c, const char *fmt, ...)
{
    char buf[96];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    cgx_text(x, y, scale, c, buf);
}

void cgx_text_center(int cx, int y, int scale, uint16_t c, const char *s)
{
    cgx_text(cx - cgx_text_w(s, scale) / 2, y, scale, c, s);
}

void cgx_present(void)
{
    dupdate();
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

static const int gint_keys[K_COUNT] = {
    [K_UP] = KEY_UP, [K_DOWN] = KEY_DOWN, [K_LEFT] = KEY_LEFT, [K_RIGHT] = KEY_RIGHT,
    [K_OK] = KEY_EXE, [K_ALT] = KEY_SHIFT, [K_ALT2] = KEY_ALPHA,
    [K_EXIT] = KEY_EXIT, [K_PAUSE] = KEY_OPTN,
    [K_F1] = KEY_F1, [K_F2] = KEY_F2, [K_F3] = KEY_F3,
    [K_F4] = KEY_F4, [K_F5] = KEY_F5, [K_F6] = KEY_F6,
};

static unsigned char now_state[K_COUNT], prev_state[K_COUNT];

void cgx_poll(void)
{
    int k;

    clearevents();
    memcpy(prev_state, now_state, sizeof(now_state));
    for (k = 0; k < K_COUNT; ++k)
        now_state[k] = keydown(gint_keys[k]) ? 1 : 0;
}

int cgx_down(int k)
{
    return now_state[k];
}

int cgx_pressed(int k)
{
    return now_state[k] && !prev_state[k];
}

int cgx_any_pressed(void)
{
    int k;

    for (k = 0; k < K_COUNT; ++k)
        if (cgx_pressed(k))
            return 1;
    return 0;
}

// ---------------------------------------------------------------------------
// Random (xorshift32)
// ---------------------------------------------------------------------------

static uint32_t rng_state = 0x2545F491u;

void cgx_seed(uint32_t s)
{
    rng_state = s ? s : 0x2545F491u;
}

uint32_t cgx_rng_state(void)
{
    return rng_state;
}

uint32_t cgx_rand(void)
{
    uint32_t x = rng_state;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}

int cgx_rand_n(int n)
{
    return n <= 0 ? 0 : (int) (cgx_rand() % (uint32_t) n);
}
