// Doodle Jump style game logic (no hardware dependencies). Fixed 60 Hz steps.
// World coordinates: x to the right, y UP (in pixels << 8 for the player).
#ifndef DOODLE_H
#define DOODLE_H

#include <stdint.h>

#define DJ_W 200            // playfield width in pixels
#define DJ_H 216            // playfield height
#define DJ_NPLAT 14
#define DJ_PW 40            // platform width
#define DJ_PH 8
#define DJ_PLW 18           // player width
#define DJ_PLH 22           // player height

enum { DJ_NORMAL, DJ_MOVING, DJ_BREAKABLE };
enum { DJ_PLAY, DJ_DEAD };

typedef struct
{
    int x, y;               // left edge, top surface (world, y up)
    int type;
    int dir;                // moving platforms: +1 / -1
    int broken;
    int spring;             // 1 = has a spring on it
} DjPlat;

typedef struct
{
    int x, y;               // player: x left edge (8.8), y feet (8.8, world y up)
    int vx, vy;             // 8.8 px / step
    int face;               // -1 left, +1 right
    int cam;                // world y of the bottom of the screen
    DjPlat plat[DJ_NPLAT];
    int top_y;              // y of the highest generated platform
    int last_safe_y;        // y of the highest non-breakable platform generated
    int state;
    int score, best;
    int ticks;
    int bounce_anim;
} Doodle;

void dj_init(Doodle *d);                // keeps best
void dj_step(Doodle *d, int steer);     // steer: -1 left, 0 none, +1 right
int dj_feet_y(const Doodle *d);         // pixels

#endif
