#include "doodle.h"

#include <string.h>

#include "cgx.h"

#define GRAVITY 70          // 0.27 px/step^2
#define JUMP_V 2400         // 9.4 px/step -> ~160 px high
#define SPRING_V 3900       // ~15 px/step -> ~410 px high
#define MAX_X_SPEED 880     // 3.4 px/step

static int max_gap(const Doodle *d)
{
    int g = 55 + d->score / 120;      // gets harder as you climb
    return g > 90 ? 90 : g;
}

static void gen_platform(Doodle *d, DjPlat *p)
{
    int gap = 28 + cgx_rand_n(max_gap(d) - 28 + 1), r;

    p->x = cgx_rand_n(DJ_W - DJ_PW);
    p->y = d->top_y + gap;
    p->dir = cgx_rand_n(2) ? 1 : -1;
    p->broken = 0;
    p->spring = 0;

    r = cgx_rand_n(100);
    if (r < 18 + d->score / 400)
        p->type = DJ_MOVING;
    else if (r < 33 + d->score / 300)
        p->type = DJ_BREAKABLE;
    else
        p->type = DJ_NORMAL;

    if (p->type == DJ_NORMAL && cgx_rand_n(100) < 6)
        p->spring = 1;

    // Breakable platforms must never be the only way up: always keep a solid
    // platform within a comfortable jump of the previous solid one.
    if (p->type == DJ_BREAKABLE && p->y - d->last_safe_y > 70)
        p->type = DJ_NORMAL;
    if (p->type != DJ_BREAKABLE)
        d->last_safe_y = p->y;

    d->top_y = p->y;
}

void dj_init(Doodle *d)
{
    int best = d->best, i;

    memset(d, 0, sizeof(*d));
    d->best = best;
    d->face = 1;

    // The first platform sits right under the player.
    d->plat[0].x = DJ_W / 2 - DJ_PW / 2;
    d->plat[0].y = 20;
    d->top_y = 20;
    d->last_safe_y = 20;
    for (i = 1; i < DJ_NPLAT; ++i)
        gen_platform(d, &d->plat[i]);

    d->x = (DJ_W / 2 - DJ_PLW / 2) << 8;
    d->y = 20 << 8;
    d->vy = JUMP_V;
}

int dj_feet_y(const Doodle *d)
{
    return d->y >> 8;
}

void dj_step(Doodle *d, int steer)
{
    int i, old_feet, new_feet;

    d->ticks++;
    if (d->bounce_anim > 0)
        d->bounce_anim--;

    if (d->state == DJ_DEAD)
        return;

    // steering
    if (steer < 0)      { d->vx -= 140; d->face = -1; }
    else if (steer > 0) { d->vx += 140; d->face = 1; }
    else                  d->vx = d->vx * 7 / 8;
    if (d->vx > MAX_X_SPEED) d->vx = MAX_X_SPEED;
    if (d->vx < -MAX_X_SPEED) d->vx = -MAX_X_SPEED;

    d->x += d->vx;
    if ((d->x >> 8) + DJ_PLW / 2 < 0)
        d->x += DJ_W << 8;
    if ((d->x >> 8) + DJ_PLW / 2 >= DJ_W)
        d->x -= DJ_W << 8;

    // vertical motion
    old_feet = d->y >> 8;
    d->vy -= GRAVITY;
    d->y += d->vy;
    new_feet = d->y >> 8;

    for (i = 0; i < DJ_NPLAT; ++i)
    {
        DjPlat *p = &d->plat[i];

        if (p->type == DJ_MOVING)
        {
            p->x += p->dir;
            if (p->x < 0 || p->x > DJ_W - DJ_PW)
                p->dir = -p->dir;
        }

        if (p->broken || d->vy >= 0)
            continue;

        if (old_feet >= p->y && new_feet <= p->y)
        {
            int px = (d->x >> 8);

            if (px + DJ_PLW > p->x + 2 && px < p->x + DJ_PW - 2)
            {
                if (p->type == DJ_BREAKABLE)
                {
                    p->broken = 1;      // falls apart, no bounce
                    continue;
                }

                d->y = p->y << 8;
                d->vy = p->spring ? SPRING_V : JUMP_V;
                d->bounce_anim = 12;
            }
        }
    }

    // camera follows the player upward only
    if ((d->y >> 8) - d->cam > 120)
        d->cam = (d->y >> 8) - 120;

    d->score = d->cam / 2;
    if (d->score > d->best)
        d->best = d->score;

    // recycle platforms that scrolled off the bottom
    for (i = 0; i < DJ_NPLAT; ++i)
        if (d->plat[i].y < d->cam - 30)
            gen_platform(d, &d->plat[i]);

    if ((d->y >> 8) < d->cam - 30)
        d->state = DJ_DEAD;
}
