// 2048 for the fx-CG50.  Arrows slide the tiles, EXE restarts, EXIT quits.
#include <stdio.h>

#include "cgx.h"
#include "g2048.h"

#define TILE 48
#define GAP 3
#define BOARD (4 * TILE + 5 * GAP)

static uint16_t tile_color(int e)
{
    static const uint16_t pal[12] = {
        CGX_RGB(205, 193, 180), CGX_RGB(238, 228, 218), CGX_RGB(237, 224, 200),
        CGX_RGB(242, 177, 121), CGX_RGB(245, 149, 99),  CGX_RGB(246, 124, 95),
        CGX_RGB(246, 94, 59),   CGX_RGB(237, 207, 114), CGX_RGB(237, 204, 97),
        CGX_RGB(237, 200, 80),  CGX_RGB(237, 197, 63),  CGX_RGB(237, 194, 46),
    };
    return pal[e < 12 ? e : 11];
}

static void draw(const G2048 *g)
{
    int bx = 8, by = (CGX_H - BOARD) / 2, i, j;
    int px = bx + BOARD + 16;

    cgx_clear(CGX_RGB(250, 248, 239));
    cgx_rect(bx, by, BOARD, BOARD, CGX_RGB(187, 173, 160));

    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
        {
            int e = g->t[i][j];
            int x = bx + GAP + j * (TILE + GAP), y = by + GAP + i * (TILE + GAP);

            cgx_rect(x, y, TILE, TILE, tile_color(e));
            if (e)
            {
                char s[8];
                int v = 1 << e, sc = v < 1000 ? 3 : (v < 10000 ? 2 : 1);
                uint16_t fg = e <= 2 ? CGX_RGB(119, 110, 101) : C_WHT;

                if (v < 100) sc = 4;
                snprintf(s, sizeof(s), "%d", v);
                cgx_text_center(x + TILE / 2, y + TILE / 2 - (7 * sc) / 2, sc, fg, s);
            }
        }

    cgx_text(px, 16, 5, CGX_RGB(119, 110, 101), "2048");
    cgx_text(px, 70, 2, CGX_RGB(119, 110, 101), "SCORE");
    cgx_textf(px, 86, 3, CGX_RGB(60, 55, 50), "%d", g->score);
    cgx_text(px, 120, 2, CGX_RGB(119, 110, 101), "BEST");
    cgx_textf(px, 136, 3, CGX_RGB(60, 55, 50), "%d", g->best);
    cgx_text(px, 176, 1, CGX_RGB(119, 110, 101), "ARROWS: MOVE");
    cgx_text(px, 188, 1, CGX_RGB(119, 110, 101), "EXE: NEW GAME");
    cgx_text(px, 200, 1, CGX_RGB(119, 110, 101), "EXIT: QUIT");

    if (g->over || g->won)
    {
        cgx_rect(bx, by + BOARD / 2 - 20, BOARD, 40, CGX_RGB(60, 55, 50));
        cgx_text_center(bx + BOARD / 2, by + BOARD / 2 - 14, 3, C_WHT, g->over ? "GAME OVER" : "YOU WIN!");
        cgx_text_center(bx + BOARD / 2, by + BOARD / 2 + 12, 1, C_WHT, g->over ? "PRESS EXE" : "KEEP GOING OR EXE");
    }
}

int main(void)
{
    static G2048 g;
    int started = 0;

    cgx_init();
    g2048_init(&g);

    for (;;)
    {
        cgx_poll();
        if (cgx_pressed(K_EXIT))
            break;

        if (!started && cgx_any_pressed())
        {
            // Seed from the time of the first key press so games differ.
            cgx_seed(cgx_ms() * 2654435761u + 12345u);
            g2048_init(&g);
            started = 1;
        }

        if (cgx_pressed(K_OK))
            g2048_init(&g);
        else if (!g.over)
        {
            if (cgx_pressed(K_UP))    g2048_move(&g, G2048_UP);
            if (cgx_pressed(K_DOWN))  g2048_move(&g, G2048_DOWN);
            if (cgx_pressed(K_LEFT))  g2048_move(&g, G2048_LEFT);
            if (cgx_pressed(K_RIGHT)) g2048_move(&g, G2048_RIGHT);
        }

        draw(&g);
        cgx_present();
    }

    return 0;
}
