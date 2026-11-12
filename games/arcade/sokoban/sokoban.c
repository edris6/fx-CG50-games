#include "sokoban.h"

#include <string.h>

// '#' wall, ' ' floor, '.' goal, '$' box, '*' box on goal, '@' player, '+' player on goal.
// All levels are checked by tests/test_logic.c, which solves them with a BFS.
static const char *const L1[] = {
    "#######",
    "#     #",
    "# $.  #",
    "#  @  #",
    "#######",
};
static const char *const L2[] = {
    "########",
    "#      #",
    "# $ .  #",
    "# $ .@ #",
    "#      #",
    "########",
};
static const char *const L3[] = {
    "#########",
    "#   #   #",
    "# $ # . #",
    "#@$ ..  #",
    "#   #   #",
    "#########",
};
static const char *const L4[] = {
    "  #####  ",
    "###   ###",
    "#  $ $  #",
    "# #. .# #",
    "#   @   #",
    "#########",
};
static const char *const L5[] = {
    "#########",
    "#       #",
    "# .$#$. #",
    "#   @   #",
    "# .$ $. #",
    "#       #",
    "#########",
};
static const char *const L6[] = {
    "#########",
    "#  .    #",
    "# #$ #  #",
    "#  $@ . #",
    "# $ #.  #",
    "#       #",
    "#########",
};
static const char *const L7[] = {
    "##########",
    "#  .  #  #",
    "# #$#  $ #",
    "#  .$ .# #",
    "# @$ #   #",
    "#   .    #",
    "##########",
};
static const char *const L8[] = {
    "###########",
    "#    #    #",
    "# $  $  $ #",
    "#  ..#..  #",
    "#  .. ..  #",
    "# $  @  $ #",
    "#    #    #",
    "###########",
};

typedef struct { const char *const *rows; int n; } LevelDef;

#define LEVEL(x) { x, (int) (sizeof(x) / sizeof(x[0])) }
static const LevelDef levels[] = {
    LEVEL(L1), LEVEL(L2), LEVEL(L3), LEVEL(L4), LEVEL(L5), LEVEL(L6), LEVEL(L7), LEVEL(L8),
};

int sk_level_count(void)
{
    return (int) (sizeof(levels) / sizeof(levels[0]));
}

const char *const *sk_level_rows(int level, int *nrows)
{
    *nrows = levels[level].n;
    return levels[level].rows;
}

int sk_load(Sokoban *s, int level)
{
    int y, x, n, found = 0;
    const char *const *rows;

    if (level < 0 || level >= sk_level_count())
        return -1;

    memset(s, 0, sizeof(*s));
    s->level = level;
    rows = sk_level_rows(level, &n);
    s->h = n;

    for (y = 0; y < n; ++y)
    {
        int len = (int) strlen(rows[y]);

        if (len > s->w)
            s->w = len;

        for (x = 0; x < len; ++x)
        {
            switch (rows[y][x])
            {
                case '#': s->cell[y][x] = SK_WALL; break;
                case '.': s->cell[y][x] = SK_GOAL; break;
                case '$': s->cell[y][x] = SK_BOX; break;
                case '*': s->cell[y][x] = SK_BOX | SK_GOAL; break;
                case '@': s->px = x; s->py = y; found = 1; break;
                case '+': s->px = x; s->py = y; s->cell[y][x] = SK_GOAL; found = 1; break;
                default: break;
            }
        }
    }

    return found ? 0 : -1;
}

int sk_move(Sokoban *s, int dir)
{
    static const int dx[4] = { 0, 1, 0, -1 }, dy[4] = { -1, 0, 1, 0 };
    int nx = s->px + dx[dir], ny = s->py + dy[dir], pushed = 0;

    if (nx < 0 || ny < 0 || nx >= s->w || ny >= s->h || (s->cell[ny][nx] & SK_WALL))
        return 0;

    if (s->cell[ny][nx] & SK_BOX)
    {
        int bx = nx + dx[dir], by = ny + dy[dir];

        if (bx < 0 || by < 0 || bx >= s->w || by >= s->h)
            return 0;
        if (s->cell[by][bx] & (SK_WALL | SK_BOX))
            return 0;

        s->cell[ny][nx] &= (uint8_t) ~SK_BOX;
        s->cell[by][bx] |= SK_BOX;
        s->pushes++;
        pushed = 1;
    }

    s->px = nx;
    s->py = ny;
    s->moves++;

    if (s->nundo < SK_UNDO)
        s->undo[s->nundo++] = (uint8_t) (dir | (pushed << 2));

    return pushed ? 2 : 1;
}

int sk_undo(Sokoban *s)
{
    static const int dx[4] = { 0, 1, 0, -1 }, dy[4] = { -1, 0, 1, 0 };
    int e, dir, pushed;

    if (s->nundo == 0)
        return 0;

    e = s->undo[--s->nundo];
    dir = e & 3;
    pushed = e >> 2;

    if (pushed)
    {
        int bx = s->px + dx[dir], by = s->py + dy[dir];

        s->cell[by][bx] &= (uint8_t) ~SK_BOX;
        s->cell[s->py][s->px] |= SK_BOX;
        s->pushes--;
    }

    s->px -= dx[dir];
    s->py -= dy[dir];
    s->moves--;
    return 1;
}

int sk_solved(const Sokoban *s)
{
    int x, y, boxes = 0;

    for (y = 0; y < s->h; ++y)
        for (x = 0; x < s->w; ++x)
        {
            if (s->cell[y][x] & SK_BOX)
            {
                boxes++;
                if (!(s->cell[y][x] & SK_GOAL))
                    return 0;
            }
        }
    return boxes > 0;
}
