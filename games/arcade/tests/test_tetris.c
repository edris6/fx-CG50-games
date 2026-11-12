#include <string.h>
#include "check.h"
#include "cgx.h"
#include "tetris.h"

static int same(const int a[4][2], const int b[4][2])
{
    int i, j, found = 0;
    for (i = 0; i < 4; ++i)
    {
        for (j = 0; j < 4; ++j)
            if (a[i][0] == b[j][0] && a[i][1] == b[j][1]) { found++; break; }
    }
    return found == 4;
}

int main(void)
{
    static Tetris t;
    int type, r, i, x, y;

    cgx_seed(42);
    memset(&t, 0, sizeof(t));
    tetris_init(&t);
    CHECK(!t.over);
    CHECK(tetris_fits(&t, &t.cur));

    // every piece: four distinct cells, spawns inside the board, and four
    // rotations bring it back to the start
    for (type = 0; type < TT_NTYPES; ++type)
    {
        TtPiece p = { type, 0, 3, 5 };
        int c0[4][2], c[4][2];

        tetris_cells(&p, c0);
        for (i = 0; i < 4; ++i)
            for (r = i + 1; r < 4; ++r)
                CHECK(c0[i][0] != c0[r][0] || c0[i][1] != c0[r][1]);

        for (r = 0; r < 4; ++r) p.rot = r;
        p.rot = 4 & 3;
        tetris_cells(&p, c);
        CHECK(same(c0, c));
    }

    // rotation: T piece rotates and stays valid; rotating 4x returns
    memset(&t, 0, sizeof(t));
    tetris_init(&t);
    t.cur.type = TT_T; t.cur.x = 3; t.cur.y = 5; t.cur.rot = 0;
    {
        int before[4][2], after[4][2];
        tetris_cells(&t.cur, before);
        for (i = 0; i < 4; ++i) CHECK(tetris_rotate(&t, 1));
        tetris_cells(&t.cur, after);
        CHECK(same(before, after));
    }

    // walls stop movement
    t.cur.type = TT_O; t.cur.x = 0; t.cur.y = 5; t.cur.rot = 0;
    for (i = 0; i < 20; ++i) tetris_move(&t, -1);
    {
        int c[4][2]; int minx = 99;
        tetris_cells(&t.cur, c);
        for (i = 0; i < 4; ++i) if (c[i][0] < minx) minx = c[i][0];
        CHECK_EQ(minx, 0);
    }

    // line clear: bottom row full except columns 0..3, drop a horizontal I there
    memset(&t, 0, sizeof(t));
    tetris_init(&t);
    memset(t.board, 0, sizeof(t.board));
    for (x = 4; x < TT_W; ++x) t.board[TT_H - 1][x] = 1;
    t.cur.type = TT_I; t.cur.rot = 0; t.cur.x = 0; t.cur.y = 0;
    tetris_hard_drop(&t);
    CHECK_EQ(t.lines, 1);
    CHECK(t.score >= 100);
    for (y = 0; y < TT_H; ++y) for (x = 0; x < TT_W; ++x) CHECK(t.board[y][x] == 0);

    // four lines at once
    memset(&t, 0, sizeof(t));
    tetris_init(&t);
    memset(t.board, 0, sizeof(t.board));
    for (y = TT_H - 4; y < TT_H; ++y) for (x = 0; x < TT_W - 1; ++x) t.board[y][x] = 2;
    t.cur.type = TT_I; t.cur.rot = 1; t.cur.x = TT_W - 3; t.cur.y = 0;     // vertical I
    {
        int c[4][2]; tetris_cells(&t.cur, c);
        CHECK(tetris_fits(&t, &t.cur));
        for (i = 0; i < 4; ++i) CHECK(c[i][0] == c[0][0]);
        // shift so that it is in the last column
        while (tetris_move(&t, 1)) {}
    }
    tetris_hard_drop(&t);
    CHECK_EQ(t.lines, 4);
    CHECK(t.score >= 800);

    // hold works once per piece
    memset(&t, 0, sizeof(t));
    tetris_init(&t);
    {
        int first = t.cur.type;
        tetris_hold(&t);
        CHECK_EQ(t.hold, first);
        i = t.cur.type;
        tetris_hold(&t);
        CHECK_EQ(t.cur.type, i);       // second hold ignored
    }

    // gravity moves pieces down and locking eventually ends the game
    memset(&t, 0, sizeof(t));
    cgx_seed(8);
    tetris_init(&t);
    for (i = 0; i < 100000 && !t.over; ++i) tetris_tick(&t, 50);
    CHECK(t.over);

    // random play never corrupts the board
    memset(&t, 0, sizeof(t));
    cgx_seed(9);
    tetris_init(&t);
    for (i = 0; i < 20000 && !t.over; ++i)
    {
        switch (cgx_rand_n(6))
        {
            case 0: tetris_move(&t, -1); break;
            case 1: tetris_move(&t, 1); break;
            case 2: tetris_rotate(&t, 1); break;
            case 3: tetris_hard_drop(&t); break;
            case 4: tetris_soft_drop(&t); break;
            default: tetris_tick(&t, 100); break;
        }
    }
    for (y = 0; y < TT_H; ++y) for (x = 0; x < TT_W; ++x) CHECK(t.board[y][x] <= TT_NTYPES);

    CHECK_DONE("tetris");
}
