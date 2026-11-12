// Snake game logic (no hardware dependencies).
#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>

#define SN_W 32
#define SN_H 17
#define SN_MAX (SN_W * SN_H)

enum { SN_UP, SN_RIGHT, SN_DOWN, SN_LEFT };

typedef struct
{
    uint8_t x[SN_MAX], y[SN_MAX];   // ring buffer of body cells
    int head, len;                  // index of the head, number of cells
    int dir;                        // direction of the last move
    int queue[2], nq;               // buffered turns
    int fx, fy;                     // food
    int score, best;
    int over;
} Snake;

void snake_init(Snake *s);          // keeps `best`
void snake_turn(Snake *s, int dir); // buffer a turn (reversals are ignored)
void snake_step(Snake *s);          // advance one cell
int snake_step_ms(const Snake *s);  // time per step; shrinks as you grow
int snake_occupies(const Snake *s, int x, int y);

#endif
