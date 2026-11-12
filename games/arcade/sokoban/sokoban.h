// Sokoban logic (no hardware dependencies).
#ifndef SOKOBAN_H
#define SOKOBAN_H

#include <stdint.h>

#define SK_MAXW 21
#define SK_MAXH 12
#define SK_UNDO 4096

enum { SK_UP, SK_RIGHT, SK_DOWN, SK_LEFT };
enum { SK_WALL = 1, SK_GOAL = 2, SK_BOX = 4 };

typedef struct
{
    uint8_t cell[SK_MAXH][SK_MAXW];
    int w, h, px, py;
    int moves, pushes;
    int level;
    uint8_t undo[SK_UNDO];      // dir | (pushed << 2)
    int nundo;
} Sokoban;

int sk_level_count(void);
const char *const *sk_level_rows(int level, int *nrows);   // for the solver tests
int sk_load(Sokoban *s, int level);       // returns 0 on success
// Returns 0 if blocked, 1 for a plain step, 2 for a push.
int sk_move(Sokoban *s, int dir);
int sk_undo(Sokoban *s);                  // returns 1 if something was undone
int sk_solved(const Sokoban *s);

#endif
