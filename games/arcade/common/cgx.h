// cgx.h -- tiny shared toolkit for the fx-CG50 arcade games.
//
// Graphics are drawn straight into gint's VRAM using the centered 384x216
// area (the OS-visible part of the 396x224 VRAM). A built-in 5x7 pixel font
// keeps text identical on the calculator and in the host test harness.
#ifndef CGX_TOOLKIT_H
#define CGX_TOOLKIT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CGX_W 384
#define CGX_H 216

#define CGX_RGB(r, g, b) \
    ((uint16_t) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3)))

// Common colors
#define C_BLK CGX_RGB(0, 0, 0)
#define C_WHT CGX_RGB(255, 255, 255)
#define C_GRY CGX_RGB(128, 128, 128)
#define C_DGR CGX_RGB(48, 48, 56)
#define C_RED CGX_RGB(230, 50, 50)
#define C_GRN CGX_RGB(60, 200, 80)
#define C_BLU CGX_RGB(60, 110, 240)
#define C_YEL CGX_RGB(250, 220, 40)
#define C_ORG CGX_RGB(250, 150, 30)
#define C_CYN CGX_RGB(60, 220, 230)
#define C_MAG CGX_RGB(220, 70, 220)

// ---- setup / timing ------------------------------------------------------
void cgx_init(void);                 // starts the millisecond timer
uint32_t cgx_ms(void);               // milliseconds since cgx_init
// Fixed-timestep helper: returns how many logic steps of `step_ms` are due.
// `acc` is per-game state (initialize to 0). Waits (spins) if none are due.
int cgx_steps(uint32_t *last_ms, int *acc_ms, int step_ms, int max_steps);

// ---- drawing (all coordinates in 0..CGX_W-1 / 0..CGX_H-1) ----------------
void cgx_clear(uint16_t c);
void cgx_rect(int x, int y, int w, int h, uint16_t c);
void cgx_frame(int x, int y, int w, int h, int thick, uint16_t c);
void cgx_pixel(int x, int y, uint16_t c);
void cgx_circle(int cx, int cy, int r, uint16_t c);   // filled
void cgx_text(int x, int y, int scale, uint16_t c, const char *s);
void cgx_textf(int x, int y, int scale, uint16_t c, const char *fmt, ...);
int cgx_text_w(const char *s, int scale);
void cgx_text_center(int cx, int y, int scale, uint16_t c, const char *s);
void cgx_present(void);              // push to the screen

// ---- input ----------------------------------------------------------------
enum {
    K_UP, K_DOWN, K_LEFT, K_RIGHT,
    K_OK,       // EXE
    K_ALT,      // SHIFT
    K_ALT2,     // ALPHA
    K_EXIT,     // EXIT
    K_PAUSE,    // OPTN
    K_F1, K_F2, K_F3, K_F4, K_F5, K_F6,
    K_COUNT
};
void cgx_poll(void);                 // call once per frame
int cgx_down(int k);                 // currently held
int cgx_pressed(int k);              // went down since the previous poll
int cgx_any_pressed(void);

// ---- random ---------------------------------------------------------------
void cgx_seed(uint32_t s);
uint32_t cgx_rng_state(void);        // lets tests snapshot/restore the generator
uint32_t cgx_rand(void);
int cgx_rand_n(int n);               // 0 .. n-1

#ifdef __cplusplus
}
#endif
#endif
