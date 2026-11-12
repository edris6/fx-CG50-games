#include "flappy.h"

#include <string.h>

#include "cgx.h"

#define GRAVITY 64          // 0.25 px/step^2 in 8.8
#define FLAP_V (-1100)      // -4.3 px/step
#define MAX_FALL 1536       // 6 px/step
#define SPEED 2

static int random_gap(void)
{
    return 30 + cgx_rand_n(FB_GROUND - FB_GAP - 60);
}

void flappy_init(Flappy *f)
{
    int best = f->best, i;

    memset(f, 0, sizeof(*f));
    f->best = best;
    f->y = 90 << 8;
    f->state = FB_READY;

    for (i = 0; i < FB_NPIPES; ++i)
    {
        f->pipe[i].x = FB_W + 60 + i * FB_PIPE_SPACING;
        f->pipe[i].gap_y = random_gap();
    }
}

int flappy_bird_y(const Flappy *f)
{
    return f->y >> 8;
}

void flappy_flap(Flappy *f)
{
    if (f->state == FB_READY)
        f->state = FB_PLAY;
    if (f->state == FB_PLAY)
        f->vy = FLAP_V;
}

static int collides(const Flappy *f)
{
    int by = f->y >> 8, i;

    if (by + FB_BIRD_H >= FB_GROUND || by < -20)
        return 1;

    for (i = 0; i < FB_NPIPES; ++i)
    {
        const FbPipe *p = &f->pipe[i];

        if (FB_BIRD_X + FB_BIRD_W > p->x && FB_BIRD_X < p->x + FB_PIPE_W)
            if (by < p->gap_y || by + FB_BIRD_H > p->gap_y + FB_GAP)
                return 1;
    }
    return 0;
}

void flappy_step(Flappy *f)
{
    int i;

    f->ticks++;

    if (f->state == FB_READY)
    {
        // Bob up and down while waiting.
        f->y = (90 << 8) + ((f->ticks / 8) % 2 ? 512 : -512);
        f->scroll = (f->scroll + SPEED) % 24;
        return;
    }

    if (f->state == FB_DEAD)
    {
        // Fall to the ground.
        f->vy += GRAVITY;
        f->y += f->vy;
        if ((f->y >> 8) + FB_BIRD_H > FB_GROUND)
            f->y = (FB_GROUND - FB_BIRD_H) << 8;
        return;
    }

    f->vy += GRAVITY;
    if (f->vy > MAX_FALL)
        f->vy = MAX_FALL;
    f->y += f->vy;
    f->scroll = (f->scroll + SPEED) % 24;

    for (i = 0; i < FB_NPIPES; ++i)
    {
        FbPipe *p = &f->pipe[i];

        p->x -= SPEED;

        if (!p->scored && p->x + FB_PIPE_W < FB_BIRD_X)
        {
            p->scored = 1;
            f->score++;
            if (f->score > f->best)
                f->best = f->score;
        }

        if (p->x + FB_PIPE_W < 0)
        {
            // Recycle behind the furthest pipe.
            int far = 0, j;
            for (j = 0; j < FB_NPIPES; ++j)
                if (f->pipe[j].x > far)
                    far = f->pipe[j].x;
            p->x = far + FB_PIPE_SPACING;
            p->gap_y = random_gap();
            p->scored = 0;
        }
    }

    if (collides(f))
        f->state = FB_DEAD;
}
