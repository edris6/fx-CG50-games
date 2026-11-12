// Minesweeper logic (no hardware dependencies).
#ifndef MINES_H
#define MINES_H

#include <stdint.h>

#define MN_W 24
#define MN_H 13
#define MN_MINES 45

enum { MN_PLAYING, MN_WON, MN_LOST };

typedef struct
{
    uint8_t mine[MN_H][MN_W];
    uint8_t open[MN_H][MN_W];
    uint8_t flag[MN_H][MN_W];
    int state;
    int placed;          // mines are placed on the first reveal
    int revealed;        // number of opened safe cells
    int flags;
    int hit_x, hit_y;    // the mine that was stepped on
} Mines;

void mines_init(Mines *m);
// Open a cell (flood-fills zeros). The first reveal is always safe.
void mines_reveal(Mines *m, int x, int y);
void mines_toggle_flag(Mines *m, int x, int y);
// If the number of adjacent flags matches the number, open the neighbours.
void mines_chord(Mines *m, int x, int y);
int mines_adjacent(const Mines *m, int x, int y);

#endif
