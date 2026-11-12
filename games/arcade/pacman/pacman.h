// Pac-Man style game logic (no hardware dependencies). Fixed 60 Hz steps.
#ifndef PACMAN_H
#define PACMAN_H

#include <stdint.h>

#define PM_W 21
#define PM_H 19
#define PM_U 64             // position units per tile

enum { PM_UP, PM_RIGHT, PM_DOWN, PM_LEFT };
enum { PM_EMPTY, PM_WALL, PM_PELLET, PM_POWER };
enum { GH_NORMAL, GH_FRIGHT, GH_EYES };
enum { PM_READY, PM_PLAY, PM_DYING, PM_CLEARED, PM_GAMEOVER };

typedef struct
{
    int x, y;               // center, in units (PM_U per tile)
    int dir;                // -1 = stopped
    int mode;               // ghosts only
    int release;            // ghosts only: steps until it leaves its start
} PmActor;

typedef struct
{
    uint8_t maze[PM_H][PM_W];
    PmActor pac;
    PmActor gh[4];
    int want;               // buffered direction for Pac-Man
    int state, state_ticks;
    int score, best, lives, level;
    int pellets;            // pellets (incl. power) left
    int fright;             // steps of fright left
    int combo;              // ghosts eaten with the current power pellet
    int phase_ticks, chase; // scatter/chase timer
    int ticks;
    int last_eaten_score;   // for the pop-up
    int last_eaten_x, last_eaten_y, last_eaten_ticks;
} Pacman;

void pm_init(Pacman *p);        // new game, keeps best
void pm_step(Pacman *p);        // one 1/60 s step
void pm_turn(Pacman *p, int dir);
const char *const *pm_maze_rows(void);
int pm_passable(const Pacman *p, int tx, int ty);

#endif
