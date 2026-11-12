// Minesweeper for the fx-CG50.
//   arrows: move   EXE: open (or chord on a number)   SHIFT: flag
//   ALPHA: new game   EXIT: quit
#include "cgx.h"
#include "mines.h"

#define CELL 15
#define BX ((CGX_W - MN_W * CELL) / 2)
#define BY 20

static uint16_t num_color(int n)
{
    static const uint16_t c[9] = {
        0, CGX_RGB(40, 70, 230), CGX_RGB(30, 140, 40), CGX_RGB(220, 40, 40),
        CGX_RGB(20, 20, 140), CGX_RGB(140, 20, 20), CGX_RGB(20, 140, 140),
        CGX_RGB(0, 0, 0), CGX_RGB(100, 100, 100),
    };
    return c[n];
}

static void draw_cell(const Mines *m, int x, int y, int cx, int cy)
{
    int px = BX + x * CELL, py = BY + y * CELL;
    int lost = m->state == MN_LOST;

    if (m->open[y][x])
    {
        int n = mines_adjacent(m, x, y);
        cgx_rect(px, py, CELL, CELL, CGX_RGB(200, 200, 205));
        cgx_frame(px, py, CELL, CELL, 1, CGX_RGB(160, 160, 168));
        if (n)
        {
            char s[2] = { (char) ('0' + n), 0 };
            cgx_text(px + 5, py + 4, 1, num_color(n), s);
        }
    }
    else if (lost && m->mine[y][x] && !m->flag[y][x])
    {
        cgx_rect(px, py, CELL, CELL, (x == m->hit_x && y == m->hit_y) ? C_RED : CGX_RGB(200, 200, 205));
        cgx_circle(px + CELL / 2, py + CELL / 2, 4, C_BLK);
    }
    else
    {
        cgx_rect(px, py, CELL, CELL, CGX_RGB(120, 130, 150));
        cgx_rect(px, py, CELL - 1, 2, CGX_RGB(190, 200, 215));
        cgx_rect(px, py, 2, CELL - 1, CGX_RGB(190, 200, 215));
        cgx_rect(px + 1, py + CELL - 2, CELL - 1, 2, CGX_RGB(70, 76, 92));
        cgx_rect(px + CELL - 2, py + 1, 2, CELL - 1, CGX_RGB(70, 76, 92));
        if (m->flag[y][x])
        {
            cgx_rect(px + 7, py + 3, 2, 9, C_BLK);
            cgx_rect(px + 3, py + 3, 5, 4, C_RED);
            if (lost && !m->mine[y][x])
                cgx_text(px + 4, py + 4, 1, C_BLK, "X");
        }
    }

    if (x == cx && y == cy)
        cgx_frame(px, py, CELL, CELL, 2, C_YEL);
}

static void draw(const Mines *m, int cx, int cy, uint32_t start_ms, uint32_t now_ms)
{
    int x, y, secs = (int) ((now_ms - start_ms) / 1000);

    cgx_clear(CGX_RGB(40, 44, 56));
    for (y = 0; y < MN_H; ++y)
        for (x = 0; x < MN_W; ++x)
            draw_cell(m, x, y, cx, cy);

    cgx_textf(BX, 6, 2, C_WHT, "MINES %d", MN_MINES - m->flags);
    cgx_textf(BX + MN_W * CELL - 80, 6, 2, C_WHT, "TIME %d", m->placed ? secs : 0);

    cgx_text(BX, BY + MN_H * CELL + 6, 1, C_GRY, "EXE OPEN   SHIFT FLAG   ALPHA NEW   EXIT QUIT");

    if (m->state != MN_PLAYING)
    {
        const char *msg = m->state == MN_WON ? "YOU WIN!" : "BOOM!";
        cgx_rect(CGX_W / 2 - 70, 90, 140, 36, C_BLK);
        cgx_frame(CGX_W / 2 - 70, 90, 140, 36, 2, m->state == MN_WON ? C_GRN : C_RED);
        cgx_text_center(CGX_W / 2, 96, 3, m->state == MN_WON ? C_GRN : C_RED, msg);
        cgx_text_center(CGX_W / 2, 118, 1, C_WHT, "PRESS ALPHA");
    }
}

int main(void)
{
    static Mines m;
    int cx = MN_W / 2, cy = MN_H / 2, started = 0;
    uint32_t start_ms = 0, end_ms = 0;

    cgx_init();
    mines_init(&m);

    for (;;)
    {
        cgx_poll();
        if (cgx_pressed(K_EXIT))
            break;

        if (!started && cgx_any_pressed())
        {
            cgx_seed(cgx_ms() * 2654435761u + 7u);
            started = 1;
        }

        if (cgx_pressed(K_UP) && cy > 0)            cy--;
        if (cgx_pressed(K_DOWN) && cy < MN_H - 1)   cy++;
        if (cgx_pressed(K_LEFT) && cx > 0)          cx--;
        if (cgx_pressed(K_RIGHT) && cx < MN_W - 1)  cx++;

        if (cgx_pressed(K_ALT2))
        {
            mines_init(&m);
            start_ms = 0;
        }

        if (m.state == MN_PLAYING)
        {
            if (cgx_pressed(K_OK))
            {
                if (m.open[cy][cx])
                    mines_chord(&m, cx, cy);
                else
                {
                    if (!m.placed)
                        start_ms = cgx_ms();
                    mines_reveal(&m, cx, cy);
                }
            }
            if (cgx_pressed(K_ALT))
                mines_toggle_flag(&m, cx, cy);
            end_ms = cgx_ms();
        }

        draw(&m, cx, cy, start_ms, end_ms);
        cgx_present();
    }

    return 0;
}
