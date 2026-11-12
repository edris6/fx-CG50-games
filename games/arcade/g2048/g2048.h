// 2048 game logic (no hardware dependencies).
#ifndef G2048_H
#define G2048_H

#include <stdint.h>

enum { G2048_UP, G2048_DOWN, G2048_LEFT, G2048_RIGHT };

typedef struct
{
    uint8_t t[4][4];    // 0 = empty, n = tile with value 2^n
    int score;
    int best;
    int won;            // reached 2048 at least once
    int over;           // no moves left
} G2048;

void g2048_init(G2048 *g);          // keeps `best`
// Slide the board. Returns 1 if anything moved (and a new tile spawned).
int g2048_move(G2048 *g, int dir);
int g2048_can_move(const G2048 *g);

// Slide/merge one line toward index 0. Returns points gained. Exposed for tests.
int g2048_slide_line(uint8_t line[4]);

#endif
