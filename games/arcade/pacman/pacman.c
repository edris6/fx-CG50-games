#include "pacman.h"

#include <string.h>

#include "cgx.h"

static const char *const maze_rows[PM_H] = {
    "#####################",
    "#o........#........o#",
    "#.###.###.#.###.###.#",
    "#...................#",
    "#.###.#.#####.#.###.#",
    "#.....#...#...#.....#",
    "#####.###.#.###.#####",
    "#####.#.......#.#####",
    "#####.#.##.##.#.#####",
    "                     ",
    "#####.#.##.##.#.#####",
    "#####.#.......#.#####",
    "#####.###.#.###.#####",
    "#.....#...#...#.....#",
    "#.###.#.#####.#.###.#",
    "#...................#",
    "#.###.###.#.###.###.#",
    "#o........#........o#",
    "#####################",
};

static const int dx[4] = { 0, 1, 0, -1 }, dy[4] = { -1, 0, 1, 0 };

static const int pac_start_x = 10, pac_start_y = 15;
static const int gh_start[4][2] = { { 10, 9 }, { 8, 7 }, { 10, 7 }, { 12, 7 } };
static const int home_x = 10, home_y = 7;
static const int scatter[4][2] = { { 19, 1 }, { 1, 1 }, { 19, 17 }, { 1, 17 } };

const char *const *pm_maze_rows(void)
{
    return maze_rows;
}

int pm_passable(const Pacman *p, int tx, int ty)
{
    if (ty < 0 || ty >= PM_H)
        return 0;
    // Only the tunnel row wraps around; elsewhere the border is solid.
    if (tx < 0)
        tx += PM_W;
    if (tx >= PM_W)
        tx -= PM_W;
    return p->maze[ty][tx] != PM_WALL;
}

static int at_center(const PmActor *a)
{
    return a->x % PM_U == PM_U / 2 && a->y % PM_U == PM_U / 2;
}

static int dist_to_center(const PmActor *a)
{
    int m, d;

    if (a->dir == PM_RIGHT || a->dir == PM_LEFT)
        m = a->x % PM_U;
    else
        m = a->y % PM_U;

    if (a->dir == PM_RIGHT || a->dir == PM_DOWN)
        d = (PM_U / 2 - m + PM_U) % PM_U;
    else
        d = (m - PM_U / 2 + PM_U) % PM_U;

    return d == 0 ? PM_U : d;
}

static void advance(PmActor *a, int step)
{
    a->x += dx[a->dir] * step;
    a->y += dy[a->dir] * step;

    if (a->x < 0)
        a->x += PM_W * PM_U;
    if (a->x >= PM_W * PM_U)
        a->x -= PM_W * PM_U;
}

static void place(PmActor *a, int tx, int ty)
{
    a->x = tx * PM_U + PM_U / 2;
    a->y = ty * PM_U + PM_U / 2;
}

static void reset_positions(Pacman *p)
{
    int i;

    place(&p->pac, pac_start_x, pac_start_y);
    p->pac.dir = -1;
    p->want = -1;

    for (i = 0; i < 4; ++i)
    {
        place(&p->gh[i], gh_start[i][0], gh_start[i][1]);
        p->gh[i].dir = -1;
        p->gh[i].mode = GH_NORMAL;
        p->gh[i].release = i * 150;
    }

    p->fright = 0;
    p->combo = 0;
    p->phase_ticks = 0;
    p->chase = 0;
}

static void load_maze(Pacman *p)
{
    int x, y;

    p->pellets = 0;
    for (y = 0; y < PM_H; ++y)
        for (x = 0; x < PM_W; ++x)
        {
            char c = maze_rows[y][x];
            uint8_t v = c == '#' ? PM_WALL : c == '.' ? PM_PELLET : c == 'o' ? PM_POWER : PM_EMPTY;

            p->maze[y][x] = v;
            if (v == PM_PELLET || v == PM_POWER)
                p->pellets++;
        }

    // keep the start tiles free of pellets
    if (p->maze[pac_start_y][pac_start_x] == PM_PELLET)
    {
        p->maze[pac_start_y][pac_start_x] = PM_EMPTY;
        p->pellets--;
    }
}

void pm_init(Pacman *p)
{
    int best = p->best;

    memset(p, 0, sizeof(*p));
    p->best = best;
    p->lives = 3;
    p->level = 1;
    load_maze(p);
    reset_positions(p);
    p->state = PM_READY;
    p->state_ticks = 120;
}

void pm_turn(Pacman *p, int dir)
{
    p->want = dir;
}

static void add_score(Pacman *p, int n)
{
    p->score += n;
    if (p->score > p->best)
        p->best = p->score;
}

static void eat_at(Pacman *p, int tx, int ty)
{
    uint8_t *c = &p->maze[ty][tx];
    int i;

    if (*c == PM_PELLET)
    {
        add_score(p, 10);
        p->pellets--;
    }
    else if (*c == PM_POWER)
    {
        add_score(p, 50);
        p->pellets--;
        p->fright = 420 - 30 * (p->level - 1);
        if (p->fright < 120)
            p->fright = 120;
        p->combo = 0;
        for (i = 0; i < 4; ++i)
            if (p->gh[i].mode == GH_NORMAL)
            {
                p->gh[i].mode = GH_FRIGHT;
                if (p->gh[i].dir >= 0)
                    p->gh[i].dir = (p->gh[i].dir + 2) & 3;   // they turn around
            }
    }
    *c = PM_EMPTY;
}

static void move_pac(Pacman *p, int speed)
{
    PmActor *a = &p->pac;
    int rem = speed;

    while (rem > 0)
    {
        int step;

        if (!at_center(a) && a->dir >= 0 && p->want == ((a->dir + 2) & 3))
            a->dir = p->want;       // instant reversal between tiles

        if (at_center(a))
        {
            int tx = a->x / PM_U, ty = a->y / PM_U;

            eat_at(p, tx, ty);

            if (p->want >= 0 && pm_passable(p, tx + dx[p->want], ty + dy[p->want]))
                a->dir = p->want;
            if (a->dir >= 0 && !pm_passable(p, tx + dx[a->dir], ty + dy[a->dir]))
                a->dir = -1;
        }

        if (a->dir < 0)
            break;

        step = dist_to_center(a);
        if (step > rem)
            step = rem;
        advance(a, step);
        rem -= step;
    }
}

static int dist2(int ax, int ay, int bx, int by)
{
    return (ax - bx) * (ax - bx) + (ay - by) * (ay - by);
}

static void ghost_target(const Pacman *p, int id, int *tx, int *ty)
{
    int px = p->pac.x / PM_U, py = p->pac.y / PM_U;
    int pd = p->pac.dir < 0 ? PM_LEFT : p->pac.dir;
    const PmActor *g = &p->gh[id];

    if (!p->chase)
    {
        *tx = scatter[id][0];
        *ty = scatter[id][1];
        return;
    }

    switch (id)
    {
        case 0:  *tx = px; *ty = py; break;                                  // chaser
        case 1:  *tx = px + 4 * dx[pd]; *ty = py + 4 * dy[pd]; break;        // ambusher
        case 2:  *tx = px + 2 * dx[pd]; *ty = py + 2 * dy[pd];               // flanker
                 *tx = 2 * *tx - p->gh[0].x / PM_U;
                 *ty = 2 * *ty - p->gh[0].y / PM_U; break;
        default:                                                              // shy
            if (dist2(g->x / PM_U, g->y / PM_U, px, py) > 64) { *tx = px; *ty = py; }
            else { *tx = scatter[id][0]; *ty = scatter[id][1]; }
            break;
    }
}

static void ghost_decide(Pacman *p, int id)
{
    static const int order[4] = { PM_UP, PM_LEFT, PM_DOWN, PM_RIGHT };
    PmActor *g = &p->gh[id];
    int tx = g->x / PM_U, ty = g->y / PM_U, cand[4], n = 0, i, best, bd = 0x7fffffff;
    int gx, gy;

    for (i = 0; i < 4; ++i)
    {
        int d = order[i];

        if (g->dir >= 0 && d == ((g->dir + 2) & 3))
            continue;       // no U-turns
        if (pm_passable(p, tx + dx[d], ty + dy[d]))
            cand[n++] = d;
    }

    if (n == 0)             // dead end: reverse
    {
        g->dir = g->dir < 0 ? PM_UP : (g->dir + 2) & 3;
        return;
    }

    if (g->mode == GH_FRIGHT)
    {
        g->dir = cand[cgx_rand_n(n)];
        return;
    }

    if (g->mode == GH_EYES)
    {
        gx = home_x;
        gy = home_y;
    }
    else
        ghost_target(p, id, &gx, &gy);

    best = cand[0];
    for (i = 0; i < n; ++i)
    {
        int d = cand[i], dd = dist2(tx + dx[d], ty + dy[d], gx, gy);

        if (dd < bd)
        {
            bd = dd;
            best = d;
        }
    }
    g->dir = best;
}

static void move_ghost(Pacman *p, int id, int speed)
{
    PmActor *g = &p->gh[id];
    int rem = speed;

    while (rem > 0)
    {
        int step;

        if (at_center(g))
        {
            if (g->mode == GH_EYES && g->x / PM_U == home_x && g->y / PM_U == home_y)
                g->mode = GH_NORMAL;

            ghost_decide(p, id);
        }

        if (g->dir < 0)
            break;

        step = dist_to_center(g);
        if (step > rem)
            step = rem;
        advance(g, step);
        rem -= step;
    }
}

static void lose_life(Pacman *p)
{
    p->state = PM_DYING;
    p->state_ticks = 100;
}

static void check_collisions(Pacman *p)
{
    int i;

    for (i = 0; i < 4; ++i)
    {
        PmActor *g = &p->gh[i];
        int ddx = p->pac.x - g->x, ddy = p->pac.y - g->y;

        if (ddx < 0) ddx = -ddx;
        if (ddy < 0) ddy = -ddy;
        if (ddx > 36 || ddy > 36 || g->mode == GH_EYES)
            continue;

        if (g->mode == GH_FRIGHT)
        {
            p->combo++;
            p->last_eaten_score = 100 << p->combo;     // 200, 400, 800, 1600
            p->last_eaten_x = g->x;
            p->last_eaten_y = g->y;
            p->last_eaten_ticks = 60;
            add_score(p, p->last_eaten_score);
            g->mode = GH_EYES;
        }
        else
        {
            lose_life(p);
            return;
        }
    }
}

void pm_step(Pacman *p)
{
    int i, ghost_speed;

    p->ticks++;
    if (p->last_eaten_ticks > 0)
        p->last_eaten_ticks--;

    switch (p->state)
    {
        case PM_READY:
            if (--p->state_ticks <= 0)
                p->state = PM_PLAY;
            return;

        case PM_DYING:
            if (--p->state_ticks <= 0)
            {
                if (--p->lives <= 0)
                    p->state = PM_GAMEOVER;
                else
                {
                    reset_positions(p);
                    p->state = PM_READY;
                    p->state_ticks = 90;
                }
            }
            return;

        case PM_CLEARED:
            if (--p->state_ticks <= 0)
            {
                p->level++;
                load_maze(p);
                reset_positions(p);
                p->state = PM_READY;
                p->state_ticks = 90;
            }
            return;

        case PM_GAMEOVER:
            return;

        default:
            break;
    }

    // scatter 7 s / chase 20 s
    p->phase_ticks++;
    if (p->phase_ticks >= (p->chase ? 1200 : 420))
    {
        p->phase_ticks = 0;
        p->chase = !p->chase;
        for (i = 0; i < 4; ++i)
            if (p->gh[i].mode == GH_NORMAL && p->gh[i].dir >= 0)
                p->gh[i].dir = (p->gh[i].dir + 2) & 3;
    }

    if (p->fright > 0 && --p->fright == 0)
        for (i = 0; i < 4; ++i)
            if (p->gh[i].mode == GH_FRIGHT)
                p->gh[i].mode = GH_NORMAL;

    move_pac(p, 7);

    ghost_speed = 5 + (p->level > 4 ? 2 : p->level / 2);    // 5..7
    for (i = 0; i < 4; ++i)
    {
        PmActor *g = &p->gh[i];

        if (g->release > 0)
        {
            g->release--;
            continue;
        }
        move_ghost(p, i, g->mode == GH_EYES ? 14 : (g->mode == GH_FRIGHT ? 3 : ghost_speed));
    }

    check_collisions(p);

    if (p->state == PM_PLAY && p->pellets <= 0)
    {
        p->state = PM_CLEARED;
        p->state_ticks = 120;
    }
}
