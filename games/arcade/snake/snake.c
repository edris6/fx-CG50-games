#include "snake.h"

#include <string.h>

#include "cgx.h"

static int idx(const Snake *s, int back)   // back = 0 is the head
{
    return (s->head - back + SN_MAX) % SN_MAX;
}

int snake_occupies(const Snake *s, int x, int y)
{
    int i;

    for (i = 0; i < s->len; ++i)
    {
        int k = idx(s, i);
        if (s->x[k] == x && s->y[k] == y)
            return 1;
    }
    return 0;
}

static void place_food(Snake *s)
{
    int free_cells = SN_W * SN_H - s->len, n, x, y;

    if (free_cells <= 0)
    {
        s->over = 1;        // the board is full: you win, effectively
        return;
    }

    n = cgx_rand_n(free_cells);
    for (y = 0; y < SN_H; ++y)
        for (x = 0; x < SN_W; ++x)
            if (!snake_occupies(s, x, y) && n-- == 0)
            {
                s->fx = x;
                s->fy = y;
                return;
            }
}

void snake_init(Snake *s)
{
    int best = s->best, i;

    memset(s, 0, sizeof(*s));
    s->best = best;
    s->len = 3;
    s->head = 2;
    for (i = 0; i < 3; ++i)
    {
        s->x[i] = (uint8_t) (SN_W / 2 - 2 + i);
        s->y[i] = SN_H / 2;
    }
    s->dir = SN_RIGHT;
    place_food(s);
}

void snake_turn(Snake *s, int dir)
{
    int last = s->nq > 0 ? s->queue[s->nq - 1] : s->dir;

    if (dir == last || dir == (last + 2) % 4 || s->nq >= 2)
        return;

    s->queue[s->nq++] = dir;
}

int snake_step_ms(const Snake *s)
{
    int ms = 150 - (s->len - 3) * 3;
    return ms < 60 ? 60 : ms;
}

void snake_step(Snake *s)
{
    static const int dx[4] = { 0, 1, 0, -1 }, dy[4] = { -1, 0, 1, 0 };
    int nx, ny, tail, grow, i;

    if (s->over)
        return;

    if (s->nq > 0)
    {
        s->dir = s->queue[0];
        s->queue[0] = s->queue[1];
        s->nq--;
    }

    nx = s->x[s->head] + dx[s->dir];
    ny = s->y[s->head] + dy[s->dir];

    if (nx < 0 || ny < 0 || nx >= SN_W || ny >= SN_H)
    {
        s->over = 1;
        return;
    }

    grow = (nx == s->fx && ny == s->fy);
    tail = idx(s, s->len - 1);

    // The tail cell is vacated this step unless we are growing.
    for (i = 0; i < s->len; ++i)
    {
        int k = idx(s, i);
        if (s->x[k] == nx && s->y[k] == ny && !(k == tail && !grow))
        {
            s->over = 1;
            return;
        }
    }

    s->head = (s->head + 1) % SN_MAX;
    s->x[s->head] = (uint8_t) nx;
    s->y[s->head] = (uint8_t) ny;

    if (grow)
    {
        s->len++;
        s->score += 10;
        if (s->score > s->best)
            s->best = s->score;
        place_food(s);
    }
}
