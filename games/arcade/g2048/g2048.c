#include "g2048.h"

#include <string.h>

#include "cgx.h"

static void spawn(G2048 *g)
{
    int empty[16], n = 0, i;

    for (i = 0; i < 16; ++i)
        if (g->t[i / 4][i % 4] == 0)
            empty[n++] = i;

    if (n == 0)
        return;

    i = empty[cgx_rand_n(n)];
    g->t[i / 4][i % 4] = cgx_rand_n(10) == 0 ? 2 : 1;   // 10% fours
}

void g2048_init(G2048 *g)
{
    int best = g->best;

    memset(g, 0, sizeof(*g));
    g->best = best;
    spawn(g);
    spawn(g);
}

int g2048_slide_line(uint8_t line[4])
{
    uint8_t out[4] = { 0, 0, 0, 0 };
    int n = 0, i, points = 0, merged_last = 0;

    for (i = 0; i < 4; ++i)
    {
        if (line[i] == 0)
            continue;

        if (n > 0 && out[n - 1] == line[i] && !merged_last)
        {
            out[n - 1]++;
            points += 1 << out[n - 1];
            merged_last = 1;
        }
        else
        {
            out[n++] = line[i];
            merged_last = 0;
        }
    }

    memcpy(line, out, 4);
    return points;
}

int g2048_move(G2048 *g, int dir)
{
    uint8_t before[4][4];
    int i, j, gained = 0;

    memcpy(before, g->t, sizeof(before));

    for (i = 0; i < 4; ++i)
    {
        uint8_t line[4];

        // Read line i oriented so that sliding "toward index 0" is the move.
        for (j = 0; j < 4; ++j)
        {
            switch (dir)
            {
                case G2048_LEFT:  line[j] = g->t[i][j]; break;
                case G2048_RIGHT: line[j] = g->t[i][3 - j]; break;
                case G2048_UP:    line[j] = g->t[j][i]; break;
                default:          line[j] = g->t[3 - j][i]; break;
            }
        }

        gained += g2048_slide_line(line);

        for (j = 0; j < 4; ++j)
        {
            switch (dir)
            {
                case G2048_LEFT:  g->t[i][j] = line[j]; break;
                case G2048_RIGHT: g->t[i][3 - j] = line[j]; break;
                case G2048_UP:    g->t[j][i] = line[j]; break;
                default:          g->t[3 - j][i] = line[j]; break;
            }
        }
    }

    if (memcmp(before, g->t, sizeof(before)) == 0)
        return 0;

    g->score += gained;
    if (g->score > g->best)
        g->best = g->score;

    spawn(g);

    for (i = 0; i < 16; ++i)
        if (g->t[i / 4][i % 4] >= 11)
            g->won = 1;

    g->over = !g2048_can_move(g);
    return 1;
}

int g2048_can_move(const G2048 *g)
{
    int i, j;

    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
        {
            if (g->t[i][j] == 0)
                return 1;
            if (j < 3 && g->t[i][j] == g->t[i][j + 1])
                return 1;
            if (i < 3 && g->t[i][j] == g->t[i + 1][j])
                return 1;
        }
    return 0;
}
