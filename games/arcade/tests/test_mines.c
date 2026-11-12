#include <string.h>
#include "check.h"
#include "cgx.h"
#include "mines.h"

static int count_mines(const Mines *m)
{
    int x, y, n = 0;
    for (y = 0; y < MN_H; ++y) for (x = 0; x < MN_W; ++x) n += m->mine[y][x];
    return n;
}

int main(void)
{
    static Mines m;
    int x, y, t;

    for (t = 0; t < 200; ++t)
    {
        cgx_seed(1000 + t);
        mines_init(&m);
        mines_reveal(&m, 5 + t % 10, 3 + t % 6);
        if (count_mines(&m) != MN_MINES) { CHECK_EQ(count_mines(&m), MN_MINES); break; }
        if (m.state != MN_PLAYING) { CHECK(m.state == MN_PLAYING); break; }   // first click is safe
        if (m.revealed < 1) { CHECK(m.revealed >= 1); break; }
    }
    CHECK(t == 200);

    // first reveal opens an area (zero cell), neighbours of the click are clear
    cgx_seed(77);
    mines_init(&m);
    mines_reveal(&m, 10, 6);
    CHECK_EQ(mines_adjacent(&m, 10, 6), 0);
    CHECK(m.revealed > 1);

    // flags
    mines_toggle_flag(&m, 0, 0);
    if (!m.open[0][0]) { CHECK_EQ(m.flags, 1); mines_toggle_flag(&m, 0, 0); CHECK_EQ(m.flags, 0); }

    // win: reveal every safe cell
    for (y = 0; y < MN_H; ++y)
        for (x = 0; x < MN_W; ++x)
            if (!m.mine[y][x]) mines_reveal(&m, x, y);
    CHECK(m.state == MN_WON);

    // lose: step on a mine
    cgx_seed(78);
    mines_init(&m);
    mines_reveal(&m, 10, 6);
    for (y = 0; y < MN_H && m.state == MN_PLAYING; ++y)
        for (x = 0; x < MN_W && m.state == MN_PLAYING; ++x)
            if (m.mine[y][x]) mines_reveal(&m, x, y);
    CHECK(m.state == MN_LOST);
    CHECK(m.hit_x >= 0);

    // a flagged cell is protected from a reveal
    cgx_seed(79);
    mines_init(&m);
    mines_reveal(&m, 10, 6);
    for (y = 0; y < MN_H; ++y) for (x = 0; x < MN_W; ++x)
        if (m.mine[y][x] && !m.open[y][x]) { mines_toggle_flag(&m, x, y); mines_reveal(&m, x, y); goto done; }
done:
    CHECK(m.state == MN_PLAYING);

    CHECK_DONE("mines");
}
