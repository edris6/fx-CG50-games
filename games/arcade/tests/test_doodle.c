#include <string.h>
#include "check.h"
#include "cgx.h"
#include "doodle.h"

// Autopilot: at every bounce pick the highest safe platform within comfortable
// jump height, then steer toward it for the whole jump.
static int hdist(const Doodle *d, const DjPlat *p)
{
    int px = (d->x >> 8) + DJ_PLW / 2, dxl = p->x + DJ_PW / 2 - px;
    if (dxl > DJ_W / 2) dxl -= DJ_W;
    if (dxl < -DJ_W / 2) dxl += DJ_W;
    return dxl;
}

static int steer_toward(const Doodle *d, int plat_index)
{
    int dxl = hdist(d, &d->plat[plat_index]);
    return dxl > 4 ? 1 : (dxl < -4 ? -1 : 0);
}

// Simulate the real game forward, steering toward platform `idx`, until the
// next landing. Returns the feet height after landing, or -1 if we die.
static int rollout(const Doodle *start, int idx)
{
    Doodle c = *start;
    uint32_t rng = cgx_rng_state();
    int t, result = -1, prev_vy;

    for (t = 0; t < 200; ++t)
    {
        prev_vy = c.vy;
        dj_step(&c, steer_toward(&c, idx));
        if (c.state == DJ_DEAD)
            break;
        if (c.vy > prev_vy + 100 && t > 0)     // bounced: we landed on something
        {
            result = dj_feet_y(&c);
            break;
        }
    }
    cgx_seed(rng);          // the rollout must not disturb the game's RNG
    return result;
}

// Pick the platform whose rollout lands highest; steer toward it.
static int steer_to(const Doodle *d)
{
    int i, best = -1, besth = -1;

    for (i = 0; i < DJ_NPLAT; ++i)
    {
        const DjPlat *p = &d->plat[i];
        int h;

        if (p->broken || p->type == DJ_BREAKABLE)
            continue;
        if (p->y > dj_feet_y(d) + 200 || p->y < dj_feet_y(d) - 220)
            continue;
        h = rollout(d, i);
        if (h > besth)
        {
            besth = h;
            best = i;
        }
    }
    return best < 0 ? 0 : steer_toward(d, best);
}

int main(void)
{
    static Doodle d;
    int seed, i, y0;

    cgx_seed(1);
    memset(&d, 0, sizeof(d));
    dj_init(&d);
    CHECK(d.state == DJ_PLAY);

    // bounces off the first platform and rises
    y0 = dj_feet_y(&d);
    for (i = 0; i < 20; ++i) dj_step(&d, 0);
    CHECK(dj_feet_y(&d) > y0 + 20);

    // falling without platforms in reach is fatal
    memset(&d, 0, sizeof(d));
    dj_init(&d);
    for (i = 0; i < DJ_NPLAT; ++i) d.plat[i].y = 100000;
    for (i = 0; i < 600 && d.state == DJ_PLAY; ++i) dj_step(&d, 0);
    CHECK(d.state == DJ_DEAD);

    // wrap-around keeps the player on screen
    memset(&d, 0, sizeof(d));
    dj_init(&d);
    for (i = 0; i < 300; ++i)
    {
        int px;
        dj_step(&d, -1);
        px = (d.x >> 8) + DJ_PLW / 2;
        CHECK(px >= -DJ_PLW && px <= DJ_W + DJ_PLW);
    }

    // the generated levels are climbable: an autopilot survives a long climb
    for (seed = 1; seed <= 8; ++seed)
    {
        cgx_seed(seed * 101);
        memset(&d, 0, sizeof(d));
        dj_init(&d);
        for (i = 0; i < 60 * 120 && d.state == DJ_PLAY; ++i)
            dj_step(&d, steer_to(&d));
        printf("  seed %d: score %d after %d steps (%s)\n", seed, d.score, i, d.state == DJ_PLAY ? "alive" : "dead");
        CHECK(d.state == DJ_PLAY);
        CHECK(d.score > 1000);
    }

    CHECK_DONE("doodle");
}
