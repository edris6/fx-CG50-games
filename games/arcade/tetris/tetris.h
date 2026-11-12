// Tetris logic (no hardware dependencies).
#ifndef TETRIS_H
#define TETRIS_H

#include <stdint.h>

#define TT_W 10
#define TT_H 20

enum { TT_I, TT_O, TT_T, TT_S, TT_Z, TT_J, TT_L, TT_NTYPES };

typedef struct { int type, rot, x, y; } TtPiece;

typedef struct
{
    uint8_t board[TT_H][TT_W];  // 0 = empty, else piece type + 1
    TtPiece cur;
    int next, hold;             // piece types; hold = -1 when empty
    int hold_used;
    int bag[TT_NTYPES], bag_pos;
    int score, lines, level, best;
    int over;
    int gravity_acc;            // ms since the last gravity step
    int lock_acc;               // ms the piece has been resting
    int last_clear;             // lines cleared by the last lock (for effects)
} Tetris;

void tetris_init(Tetris *t);                 // keeps best
// Cells of a piece: returns 4 (x, y) pairs relative to the board.
void tetris_cells(const TtPiece *p, int out[4][2]);
int tetris_fits(const Tetris *t, const TtPiece *p);
int tetris_move(Tetris *t, int dx);          // returns 1 if moved
int tetris_rotate(Tetris *t, int dir);       // dir = +1 cw, -1 ccw, with wall kicks
int tetris_soft_drop(Tetris *t);             // one row; +1 point
void tetris_hard_drop(Tetris *t);            // drop and lock
void tetris_hold(Tetris *t);
void tetris_tick(Tetris *t, int ms);         // gravity + lock delay
int tetris_ghost_y(const Tetris *t);         // y where the piece would land
int tetris_gravity_ms(int level);

#endif
