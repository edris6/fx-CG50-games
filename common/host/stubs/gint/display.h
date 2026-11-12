// Host-side stub of <gint/display.h> for headless testing.
#pragma once
#include <stdint.h>
#define DWIDTH 396
#define DHEIGHT 224
#define C_WHITE 0xffff
#define C_BLACK 0x0000
extern uint16_t *gint_vram;
void dclear(int c);
void dupdate(void);
void drect(int x1, int y1, int x2, int y2, int c);
void dtext(int x, int y, int fg, const char *s);
void dprint(int x, int y, int fg, const char *fmt, ...);
