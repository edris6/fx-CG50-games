// Flappy Bird logic (no hardware dependencies). Fixed 60 Hz steps.
#ifndef FLAPPY_H
#define FLAPPY_H

#include <stdint.h>

#define FB_W 384
#define FB_H 216
#define FB_GROUND 196           // y of the ground
#define FB_BIRD_X 90
#define FB_BIRD_W 17            // collision box
#define FB_BIRD_H 13
#define FB_PIPE_W 36
#define FB_GAP 66
#define FB_PIPE_SPACING 140
#define FB_NPIPES 4

typedef struct { int x; int gap_y; int scored; } FbPipe;   // gap_y = top of gap

enum { FB_READY, FB_PLAY, FB_DEAD };

typedef struct
{
    int y;              // bird top, 8.8 fixed point
    int vy;             // 8.8 fixed point, px/step
    FbPipe pipe[FB_NPIPES];
    int state;
    int score, best;
    int scroll;         // ground animation
    int ticks;
} Flappy;

void flappy_init(Flappy *f);         // keeps best
void flappy_flap(Flappy *f);         // starts the game from READY
void flappy_step(Flappy *f);         // one 1/60 s step
int flappy_bird_y(const Flappy *f);  // pixels

#endif
