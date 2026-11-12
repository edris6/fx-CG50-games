#include <stdlib.h>
#include <string.h>
#include "check.h"
#include "cgx.h"
#include "flappy.h"

// Autopilot: flap whenever the bird is below the middle of the next gap.
static void autopilot(Flappy *f)
{
    int i, target = 90, best = 100000;

    for (i = 0; i < FB_NPIPES; ++i)
        if (f->pipe[i].x + FB_PIPE_W > FB_BIRD_X - 4 && f->pipe[i].x < best)
        {
            best = f->pipe[i].x;
            target = f->pipe[i].gap_y + 46;   // flap point: the rise then peaks just under the top pipe
        }

    if (flappy_bird_y(f) + FB_BIRD_H / 2 > target && f->vy >= 0)
        flappy_flap(f);
}

int main(void)
{
    static Flappy f;
    int i, y0;

    cgx_seed(3);
    memset(&f, 0, sizeof(f));
    flappy_init(&f);
    CHECK(f.state == FB_READY);

    // gravity does nothing until the first flap
    y0 = flappy_bird_y(&f);
    for (i = 0; i < 30; ++i) flappy_step(&f);
    CHECK(f.state == FB_READY && abs(flappy_bird_y(&f) - y0) < 5);

    // a flap moves the bird up, then it falls
    flappy_flap(&f);
    CHECK(f.state == FB_PLAY);
    y0 = flappy_bird_y(&f);
    for (i = 0; i < 10; ++i) flappy_step(&f);
    CHECK(flappy_bird_y(&f) < y0);

    // doing nothing eventually kills the bird on the ground
    for (i = 0; i < 400 && f.state == FB_PLAY; ++i) flappy_step(&f);
    CHECK(f.state == FB_DEAD);
    for (i = 0; i < 100; ++i) flappy_step(&f);
    CHECK(flappy_bird_y(&f) + FB_BIRD_H <= FB_GROUND);

    // a competent player can pass pipes: the layout must be passable
    for (int seed = 1; seed <= 10; ++seed)
    {
        cgx_seed(seed * 31);
        memset(&f, 0, sizeof(f));
        flappy_init(&f);
        flappy_flap(&f);
        for (i = 0; i < 60 * 60 && f.state == FB_PLAY; ++i)
        {
            autopilot(&f);
            flappy_step(&f);
        }
        CHECK(f.state == FB_PLAY);
        CHECK(f.score >= 12);
    }

    CHECK_DONE("flappy");
}
