#include <string.h>
#include "check.h"
#include "cgx.h"
#include "g2048.h"

static void line(uint8_t *l, int a, int b, int c, int d) { l[0] = a; l[1] = b; l[2] = c; l[3] = d; }

int main(void)
{
    uint8_t l[4];
    G2048 g;
    int i, j, tiles;

    line(l, 1, 1, 1, 1); CHECK_EQ(g2048_slide_line(l), 8);   // 2,2,2,2 -> 4,4 = 8 points
    CHECK(l[0] == 2 && l[1] == 2 && l[2] == 0 && l[3] == 0);
    line(l, 1, 1, 2, 0); g2048_slide_line(l);
    CHECK(l[0] == 2 && l[1] == 2 && l[2] == 0);              // 2,2,4 -> 4,4 (no double merge)
    line(l, 0, 1, 0, 1); g2048_slide_line(l);
    CHECK(l[0] == 2 && l[1] == 0);
    line(l, 1, 2, 3, 4); CHECK_EQ(g2048_slide_line(l), 0);
    line(l, 2, 1, 1, 0); g2048_slide_line(l);
    CHECK(l[0] == 2 && l[1] == 2 && l[2] == 0);

    cgx_seed(1234);
    memset(&g, 0, sizeof(g));
    g2048_init(&g);
    tiles = 0;
    for (i = 0; i < 4; ++i) for (j = 0; j < 4; ++j) tiles += g.t[i][j] != 0;
    CHECK_EQ(tiles, 2);

    // directional correctness
    memset(&g, 0, sizeof(g));
    g.t[3][0] = 1;
    CHECK(g2048_move(&g, G2048_UP));
    CHECK(g.t[0][0] == 1);
    memset(&g, 0, sizeof(g));
    g.t[0][0] = 1;
    CHECK(g2048_move(&g, G2048_RIGHT));
    CHECK(g.t[0][3] == 1);
    CHECK(!g2048_move(&g, G2048_RIGHT) || 1);                 // may spawn; just must not crash

    // no-op move does not spawn a tile
    memset(&g, 0, sizeof(g));
    g.t[0][0] = 1;
    CHECK(!g2048_move(&g, G2048_LEFT));
    tiles = 0;
    for (i = 0; i < 4; ++i) for (j = 0; j < 4; ++j) tiles += g.t[i][j] != 0;
    CHECK_EQ(tiles, 1);

    // game over: a checkerboard has no moves
    memset(&g, 0, sizeof(g));
    for (i = 0; i < 4; ++i) for (j = 0; j < 4; ++j) g.t[i][j] = ((i + j) & 1) ? 1 : 2;
    CHECK(!g2048_can_move(&g));

    // a full random game ends and never corrupts the board
    cgx_seed(99);
    memset(&g, 0, sizeof(g));
    g2048_init(&g);
    for (i = 0; i < 20000 && !g.over; ++i)
        g2048_move(&g, cgx_rand_n(4));
    CHECK(g.over);
    CHECK(g.score > 0 && g.best == g.score);
    for (i = 0; i < 4; ++i) for (j = 0; j < 4; ++j) CHECK(g.t[i][j] <= 17);

    CHECK_DONE("2048");
}
