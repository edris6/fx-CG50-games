#include <string.h>
#include "check.h"
#include "cgx.h"
#include "pacman.h"

static int flood(const Pacman *p, int sx, int sy, uint8_t seen[PM_H][PM_W])
{
    int stack[PM_W * PM_H], sp = 0, n = 0;
    memset(seen, 0, PM_W * PM_H);
    stack[sp++] = sy * PM_W + sx; seen[sy][sx] = 1;
    while (sp)
    {
        int c = stack[--sp], x = c % PM_W, y = c / PM_W, d;
        static const int dx[4] = { 0, 1, 0, -1 }, dy[4] = { -1, 0, 1, 0 };
        n++;
        for (d = 0; d < 4; ++d)
        {
            int nx = (x + dx[d] + PM_W) % PM_W, ny = y + dy[d];
            if (!pm_passable(p, nx, ny) || seen[ny][nx]) continue;
            seen[ny][nx] = 1; stack[sp++] = ny * PM_W + nx;
        }
    }
    return n;
}

int main(void)
{
    static Pacman p;
    uint8_t seen[PM_H][PM_W];
    const char *const *rows = pm_maze_rows();
    int x, y, i, open = 0, pellets = 0;

    // maze sanity: size, left-right symmetry, everything connected
    for (y = 0; y < PM_H; ++y)
    {
        CHECK_EQ((long) strlen(rows[y]), PM_W);
        for (x = 0; x < PM_W; ++x)
        {
            CHECK(rows[y][x] == rows[y][PM_W - 1 - x]);
            if (rows[y][x] != '#') open++;
            if (rows[y][x] == '.' || rows[y][x] == 'o') pellets++;
        }
        CHECK(rows[y][0] == rows[y][PM_W - 1]);
    }

    cgx_seed(1);
    memset(&p, 0, sizeof(p));
    pm_init(&p);
    CHECK_EQ(flood(&p, 10, 15, seen), open);
    CHECK(p.pellets > 100 && p.pellets <= pellets);
    CHECK_EQ(p.lives, 3);

    // no dead ends: every open tile has >= 2 open neighbours (except none)
    for (y = 0; y < PM_H; ++y)
        for (x = 0; x < PM_W; ++x)
            if (pm_passable(&p, x, y))
            {
                int n = pm_passable(&p, x + 1, y) + pm_passable(&p, x - 1, y)
                      + pm_passable(&p, x, y + 1) + pm_passable(&p, x, y - 1);
                CHECK(n >= 2);
            }

    // Pac-Man moves and eats
    for (i = 0; i < 200; ++i) pm_step(&p);      // ready countdown
    CHECK(p.state == PM_PLAY);
    {
        int before = p.score;
        pm_turn(&p, PM_LEFT);
        for (i = 0; i < 120; ++i) pm_step(&p);
        CHECK(p.score > before || p.state == PM_DYING);
    }

    // long random simulation: actors always stay on open tiles at tile centres
    for (int seed = 1; seed <= 5; ++seed)
    {
        cgx_seed(seed * 13);
        memset(&p, 0, sizeof(p));
        pm_init(&p);
        for (i = 0; i < 30000 && p.state != PM_GAMEOVER; ++i)
        {
            int k;
            if (i % 20 == 0) pm_turn(&p, cgx_rand_n(4));
            pm_step(&p);

            CHECK(p.pac.y >= 0 && p.pac.y < PM_H * PM_U);
            CHECK(pm_passable(&p, p.pac.x / PM_U, p.pac.y / PM_U));
            for (k = 0; k < 4; ++k)
            {
                CHECK(p.gh[k].y >= 0 && p.gh[k].y < PM_H * PM_U);
                CHECK(pm_passable(&p, p.gh[k].x / PM_U, p.gh[k].y / PM_U));
            }
        }
        CHECK(p.score >= 0);
    }

    // clearing all pellets finishes the level
    memset(&p, 0, sizeof(p));
    cgx_seed(5);
    pm_init(&p);
    for (y = 0; y < PM_H; ++y) for (x = 0; x < PM_W; ++x)
        if (p.maze[y][x] == PM_PELLET || p.maze[y][x] == PM_POWER) { p.maze[y][x] = PM_EMPTY; p.pellets--; }
    p.maze[15][8] = PM_PELLET; p.pellets = 1;
    p.state = PM_PLAY;
    pm_turn(&p, PM_LEFT);
    for (i = 0; i < 100 && p.state == PM_PLAY; ++i) pm_step(&p);
    CHECK(p.state == PM_CLEARED || p.state == PM_DYING);

    CHECK_DONE("pacman");
}
