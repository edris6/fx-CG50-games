#include "mines.h"

#include <string.h>

#include "cgx.h"

void mines_init(Mines *m)
{
    memset(m, 0, sizeof(*m));
    m->hit_x = m->hit_y = -1;
}

int mines_adjacent(const Mines *m, int x, int y)
{
    int dx, dy, n = 0;

    for (dy = -1; dy <= 1; ++dy)
        for (dx = -1; dx <= 1; ++dx)
        {
            int nx = x + dx, ny = y + dy;
            if ((dx || dy) && nx >= 0 && ny >= 0 && nx < MN_W && ny < MN_H && m->mine[ny][nx])
                n++;
        }
    return n;
}

static void place_mines(Mines *m, int sx, int sy)
{
    int placed = 0;

    while (placed < MN_MINES)
    {
        int x = cgx_rand_n(MN_W), y = cgx_rand_n(MN_H);

        // Keep the first click and its neighbours clear so it opens an area.
        if (m->mine[y][x] || (x >= sx - 1 && x <= sx + 1 && y >= sy - 1 && y <= sy + 1))
            continue;

        m->mine[y][x] = 1;
        placed++;
    }
    m->placed = 1;
}

static void check_win(Mines *m)
{
    if (m->state == MN_PLAYING && m->revealed == MN_W * MN_H - MN_MINES)
        m->state = MN_WON;
}

void mines_reveal(Mines *m, int x, int y)
{
    static int stack[MN_W * MN_H];
    int sp = 0;

    if (m->state != MN_PLAYING || x < 0 || y < 0 || x >= MN_W || y >= MN_H)
        return;
    if (m->open[y][x] || m->flag[y][x])
        return;

    if (!m->placed)
        place_mines(m, x, y);

    if (m->mine[y][x])
    {
        m->state = MN_LOST;
        m->hit_x = x;
        m->hit_y = y;
        return;
    }

    stack[sp++] = y * MN_W + x;
    m->open[y][x] = 1;
    m->revealed++;

    while (sp > 0)
    {
        int cur = stack[--sp], cx = cur % MN_W, cy = cur / MN_W, dx, dy;

        if (mines_adjacent(m, cx, cy) != 0)
            continue;

        for (dy = -1; dy <= 1; ++dy)
            for (dx = -1; dx <= 1; ++dx)
            {
                int nx = cx + dx, ny = cy + dy;
                if (nx < 0 || ny < 0 || nx >= MN_W || ny >= MN_H)
                    continue;
                if (m->open[ny][nx] || m->flag[ny][nx] || m->mine[ny][nx])
                    continue;
                m->open[ny][nx] = 1;
                m->revealed++;
                stack[sp++] = ny * MN_W + nx;
            }
    }

    check_win(m);
}

void mines_toggle_flag(Mines *m, int x, int y)
{
    if (m->state != MN_PLAYING || m->open[y][x])
        return;

    m->flag[y][x] ^= 1;
    m->flags += m->flag[y][x] ? 1 : -1;
}

void mines_chord(Mines *m, int x, int y)
{
    int dx, dy, nf = 0;

    if (m->state != MN_PLAYING || !m->open[y][x] || mines_adjacent(m, x, y) == 0)
        return;

    for (dy = -1; dy <= 1; ++dy)
        for (dx = -1; dx <= 1; ++dx)
        {
            int nx = x + dx, ny = y + dy;
            if (nx >= 0 && ny >= 0 && nx < MN_W && ny < MN_H && m->flag[ny][nx])
                nf++;
        }

    if (nf != mines_adjacent(m, x, y))
        return;

    for (dy = -1; dy <= 1; ++dy)
        for (dx = -1; dx <= 1; ++dx)
            mines_reveal(m, x + dx, y + dy);
}
