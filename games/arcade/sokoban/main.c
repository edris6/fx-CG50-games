// Sokoban for the fx-CG50.
//   arrows: move/push   SHIFT: undo   ALPHA: restart level
//   F1/F2: previous/next level   EXE: next level (when solved)   EXIT: quit
#include "cgx.h"
#include "sokoban.h"

static void draw(const Sokoban *s, int solved)
{
    int tile = 190 / s->h, x, y;
    int tw = 384 / s->w;
    int ox, oy;

    if (tw < tile) tile = tw;
    if (tile > 24) tile = 24;
    ox = (CGX_W - s->w * tile) / 2;
    oy = 20 + (190 - s->h * tile) / 2;

    cgx_clear(CGX_RGB(30, 28, 40));

    for (y = 0; y < s->h; ++y)
        for (x = 0; x < s->w; ++x)
        {
            int c = s->cell[y][x], px = ox + x * tile, py = oy + y * tile;

            if (c & SK_WALL)
            {
                cgx_rect(px, py, tile, tile, CGX_RGB(120, 70, 50));
                cgx_frame(px, py, tile, tile, 1, CGX_RGB(70, 40, 30));
                cgx_rect(px + 1, py + tile / 2, tile - 2, 1, CGX_RGB(70, 40, 30));
            }
            else
            {
                cgx_rect(px, py, tile, tile, CGX_RGB(70, 68, 80));
                if (c & SK_GOAL)
                    cgx_circle(px + tile / 2, py + tile / 2, tile / 6 + 1, C_YEL);
            }

            if (c & SK_BOX)
            {
                uint16_t col = (c & SK_GOAL) ? C_GRN : CGX_RGB(210, 150, 60);
                cgx_rect(px + 2, py + 2, tile - 4, tile - 4, col);
                cgx_frame(px + 2, py + 2, tile - 4, tile - 4, 2, CGX_RGB(110, 70, 20));
            }
        }

    // the player
    {
        int px = ox + s->px * tile, py = oy + s->py * tile;
        cgx_circle(px + tile / 2, py + tile / 2, tile / 2 - 2, C_BLU);
        cgx_rect(px + tile / 2 - 4, py + tile / 2 - 3, 2, 2, C_WHT);
        cgx_rect(px + tile / 2 + 2, py + tile / 2 - 3, 2, 2, C_WHT);
    }

    cgx_textf(6, 6, 1, C_WHT, "LEVEL %d/%d", s->level + 1, sk_level_count());
    cgx_textf(110, 6, 1, C_WHT, "MOVES %d", s->moves);
    cgx_textf(200, 6, 1, C_WHT, "PUSHES %d", s->pushes);
    cgx_text(6, 206, 1, C_GRY, "SHIFT UNDO  ALPHA RESTART  F1/F2 LEVEL  EXIT QUIT");

    if (solved)
    {
        cgx_rect(CGX_W / 2 - 80, 84, 160, 44, C_BLK);
        cgx_frame(CGX_W / 2 - 80, 84, 160, 44, 2, C_GRN);
        cgx_text_center(CGX_W / 2, 92, 3, C_GRN, "SOLVED!");
        cgx_text_center(CGX_W / 2, 116, 1, C_WHT, "PRESS EXE FOR NEXT");
    }
}

int main(void)
{
    static Sokoban s;
    int level = 0;

    cgx_init();
    sk_load(&s, level);

    for (;;)
    {
        int solved;

        cgx_poll();
        if (cgx_pressed(K_EXIT))
            break;

        solved = sk_solved(&s);

        if (!solved)
        {
            if (cgx_pressed(K_UP))    sk_move(&s, SK_UP);
            if (cgx_pressed(K_DOWN))  sk_move(&s, SK_DOWN);
            if (cgx_pressed(K_LEFT))  sk_move(&s, SK_LEFT);
            if (cgx_pressed(K_RIGHT)) sk_move(&s, SK_RIGHT);
            if (cgx_pressed(K_ALT))   sk_undo(&s);
            solved = sk_solved(&s);
        }

        if (cgx_pressed(K_ALT2))
            sk_load(&s, level);
        if (cgx_pressed(K_F2) || (solved && cgx_pressed(K_OK)))
        {
            level = (level + 1) % sk_level_count();
            sk_load(&s, level);
        }
        if (cgx_pressed(K_F1))
        {
            level = (level + sk_level_count() - 1) % sk_level_count();
            sk_load(&s, level);
        }

        draw(&s, solved);
        cgx_present();
    }

    return 0;
}
