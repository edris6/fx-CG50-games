#include "tetris.h"

#include <string.h>

#include "cgx.h"

static const int8_t base[TT_NTYPES][4][2] = {
    [TT_I] = { { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 } },
    [TT_O] = { { 1, 0 }, { 2, 0 }, { 1, 1 }, { 2, 1 } },
    [TT_T] = { { 1, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } },
    [TT_S] = { { 1, 0 }, { 2, 0 }, { 0, 1 }, { 1, 1 } },
    [TT_Z] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 2, 1 } },
    [TT_J] = { { 0, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } },
    [TT_L] = { { 2, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } },
};

static int box_size(int type)
{
    return (type == TT_I || type == TT_O) ? 4 : 3;
}

void tetris_cells(const TtPiece *p, int out[4][2])
{
    int i, r, size = box_size(p->type);

    for (i = 0; i < 4; ++i)
    {
        int x = base[p->type][i][0], y = base[p->type][i][1];

        if (p->type != TT_O)
            for (r = 0; r < (p->rot & 3); ++r)
            {
                int nx = size - 1 - y, ny = x;
                x = nx;
                y = ny;
            }

        out[i][0] = p->x + x;
        out[i][1] = p->y + y;
    }
}

int tetris_fits(const Tetris *t, const TtPiece *p)
{
    int c[4][2], i;

    tetris_cells(p, c);
    for (i = 0; i < 4; ++i)
    {
        if (c[i][0] < 0 || c[i][0] >= TT_W || c[i][1] >= TT_H)
            return 0;
        if (c[i][1] >= 0 && t->board[c[i][1]][c[i][0]])
            return 0;
    }
    return 1;
}

static int next_from_bag(Tetris *t)
{
    if (t->bag_pos >= TT_NTYPES)
    {
        int i;

        for (i = 0; i < TT_NTYPES; ++i)
            t->bag[i] = i;
        for (i = TT_NTYPES - 1; i > 0; --i)
        {
            int j = cgx_rand_n(i + 1), tmp = t->bag[i];
            t->bag[i] = t->bag[j];
            t->bag[j] = tmp;
        }
        t->bag_pos = 0;
    }
    return t->bag[t->bag_pos++];
}

static void spawn(Tetris *t, int type)
{
    t->cur.type = type;
    t->cur.rot = 0;
    t->cur.x = (TT_W - 4) / 2;
    t->cur.y = 0;
    t->gravity_acc = 0;
    t->lock_acc = 0;
    t->hold_used = 0;

    if (!tetris_fits(t, &t->cur))
        t->over = 1;
}

void tetris_init(Tetris *t)
{
    int best = t->best;

    memset(t, 0, sizeof(*t));
    t->best = best;
    t->bag_pos = TT_NTYPES;
    t->hold = -1;
    t->level = 1;
    t->next = next_from_bag(t);
    spawn(t, next_from_bag(t));
}

int tetris_gravity_ms(int level)
{
    int ms = 800 - (level - 1) * 70;
    return ms < 70 ? 70 : ms;
}

int tetris_move(Tetris *t, int dx)
{
    TtPiece p = t->cur;

    if (t->over)
        return 0;

    p.x += dx;
    if (!tetris_fits(t, &p))
        return 0;

    t->cur = p;
    t->lock_acc = 0;
    return 1;
}

int tetris_rotate(Tetris *t, int dir)
{
    static const int8_t kicks[][2] = { { 0, 0 }, { -1, 0 }, { 1, 0 }, { 0, -1 }, { -2, 0 }, { 2, 0 }, { -1, -1 }, { 1, -1 } };
    int i;

    if (t->over || t->cur.type == TT_O)
        return 0;

    for (i = 0; i < (int) (sizeof(kicks) / sizeof(kicks[0])); ++i)
    {
        TtPiece p = t->cur;

        p.rot = (p.rot + dir + 4) & 3;
        p.x += kicks[i][0];
        p.y += kicks[i][1];

        if (tetris_fits(t, &p))
        {
            t->cur = p;
            t->lock_acc = 0;
            return 1;
        }
    }
    return 0;
}

static void lock_piece(Tetris *t)
{
    int c[4][2], i, y, cleared = 0;

    tetris_cells(&t->cur, c);
    for (i = 0; i < 4; ++i)
        if (c[i][1] >= 0)
            t->board[c[i][1]][c[i][0]] = (uint8_t) (t->cur.type + 1);

    for (y = TT_H - 1; y >= 0; --y)
    {
        int x, full = 1;

        for (x = 0; x < TT_W; ++x)
            if (!t->board[y][x])
                full = 0;

        if (full)
        {
            memmove(&t->board[1], &t->board[0], (size_t) y * TT_W);
            memset(&t->board[0], 0, TT_W);
            cleared++;
            y++;        // re-check the same row index
        }
    }

    t->last_clear = cleared;
    if (cleared)
    {
        static const int pts[5] = { 0, 100, 300, 500, 800 };

        t->score += pts[cleared] * t->level;
        t->lines += cleared;
        t->level = 1 + t->lines / 10;
        if (t->score > t->best)
            t->best = t->score;
    }

    spawn(t, t->next);
    t->next = next_from_bag(t);
}

int tetris_soft_drop(Tetris *t)
{
    TtPiece p = t->cur;

    if (t->over)
        return 0;

    p.y++;
    if (!tetris_fits(t, &p))
        return 0;

    t->cur = p;
    t->score++;
    if (t->score > t->best)
        t->best = t->score;
    t->gravity_acc = 0;
    return 1;
}

int tetris_ghost_y(const Tetris *t)
{
    TtPiece p = t->cur;

    while (1)
    {
        p.y++;
        if (!tetris_fits(t, &p))
            return p.y - 1;
    }
}

void tetris_hard_drop(Tetris *t)
{
    int y;

    if (t->over)
        return;

    y = tetris_ghost_y(t);
    t->score += 2 * (y - t->cur.y);
    if (t->score > t->best)
        t->best = t->score;
    t->cur.y = y;
    lock_piece(t);
}

void tetris_hold(Tetris *t)
{
    int type;

    if (t->over || t->hold_used)
        return;

    type = t->cur.type;
    if (t->hold < 0)
    {
        spawn(t, t->next);
        t->next = next_from_bag(t);
    }
    else
        spawn(t, t->hold);

    t->hold = type;
    t->hold_used = 1;
}

void tetris_tick(Tetris *t, int ms)
{
    TtPiece p;

    if (t->over)
        return;

    t->gravity_acc += ms;

    while (t->gravity_acc >= tetris_gravity_ms(t->level) && !t->over)
    {
        t->gravity_acc -= tetris_gravity_ms(t->level);

        p = t->cur;
        p.y++;
        if (tetris_fits(t, &p))
            t->cur = p;
    }

    // Lock delay: once the piece rests on something, give 400 ms to slide it.
    p = t->cur;
    p.y++;
    if (!tetris_fits(t, &p))
    {
        t->lock_acc += ms;
        if (t->lock_acc >= 400)
            lock_piece(t);
    }
    else
        t->lock_acc = 0;
}
