// Tetris for the fx-CG50.
//   LEFT/RIGHT move   DOWN soft drop   UP hard drop
//   EXE rotate        SHIFT rotate back   ALPHA hold
//   OPTN pause        EXIT quit
#include "cgx.h"
#include "tetris.h"

#define CELL 10
#define BX 142
#define BY 8

static const uint16_t colors[TT_NTYPES] = {
    CGX_RGB(60, 220, 230), CGX_RGB(250, 220, 40), CGX_RGB(180, 70, 220), CGX_RGB(70, 210, 80),
    CGX_RGB(230, 60, 60),  CGX_RGB(60, 100, 240), CGX_RGB(250, 150, 30),
};

static void block(int px, int py, uint16_t c)
{
    cgx_rect(px, py, CELL, CELL, c);
    cgx_rect(px, py, CELL, 1, C_WHT);
    cgx_rect(px, py, 1, CELL, C_WHT);
    cgx_rect(px + 1, py + CELL - 1, CELL - 1, 1, C_DGR);
    cgx_rect(px + CELL - 1, py + 1, 1, CELL - 1, C_DGR);
}

static void mini_piece(int cx, int cy, int type)
{
    TtPiece p = { type, 0, 0, 0 };
    int c[4][2], i, minx = 9, maxx = 0, miny = 9, maxy = 0;

    tetris_cells(&p, c);
    for (i = 0; i < 4; ++i)
    {
        if (c[i][0] < minx) minx = c[i][0];
        if (c[i][0] > maxx) maxx = c[i][0];
        if (c[i][1] < miny) miny = c[i][1];
        if (c[i][1] > maxy) maxy = c[i][1];
    }
    for (i = 0; i < 4; ++i)
        block(cx - ((maxx - minx + 1) * CELL) / 2 + (c[i][0] - minx) * CELL,
              cy - ((maxy - miny + 1) * CELL) / 2 + (c[i][1] - miny) * CELL, colors[type]);
}

static void draw(const Tetris *t, int paused)
{
    int x, y, i, c[4][2];

    cgx_clear(CGX_RGB(16, 18, 30));
    cgx_rect(BX - 2, BY - 2, TT_W * CELL + 4, TT_H * CELL + 4, C_GRY);
    cgx_rect(BX, BY, TT_W * CELL, TT_H * CELL, C_BLK);

    for (y = 0; y < TT_H; ++y)
        for (x = 0; x < TT_W; ++x)
            if (t->board[y][x])
                block(BX + x * CELL, BY + y * CELL, colors[t->board[y][x] - 1]);

    if (!t->over)
    {
        TtPiece g = t->cur;

        g.y = tetris_ghost_y(t);
        tetris_cells(&g, c);
        for (i = 0; i < 4; ++i)
            if (c[i][1] >= 0)
                cgx_frame(BX + c[i][0] * CELL, BY + c[i][1] * CELL, CELL, CELL, 1, colors[t->cur.type]);

        tetris_cells(&t->cur, c);
        for (i = 0; i < 4; ++i)
            if (c[i][1] >= 0)
                block(BX + c[i][0] * CELL, BY + c[i][1] * CELL, colors[t->cur.type]);
    }

    // left panel: hold
    cgx_text_center(70, 16, 2, C_WHT, "HOLD");
    cgx_frame(30, 36, 80, 50, 2, C_GRY);
    if (t->hold >= 0)
        mini_piece(70, 61, t->hold);

    cgx_text(8, 110, 1, C_GRY, "ARROWS MOVE");
    cgx_text(8, 122, 1, C_GRY, "UP: HARD DROP");
    cgx_text(8, 134, 1, C_GRY, "EXE/SHIFT: ROTATE");
    cgx_text(8, 146, 1, C_GRY, "ALPHA: HOLD");
    cgx_text(8, 158, 1, C_GRY, "OPTN: PAUSE");

    // right panel: next + stats
    cgx_text_center(314, 16, 2, C_WHT, "NEXT");
    cgx_frame(274, 36, 80, 50, 2, C_GRY);
    mini_piece(314, 61, t->next);

    cgx_text(262, 104, 1, C_GRY, "SCORE");
    cgx_textf(262, 114, 2, C_WHT, "%d", t->score);
    cgx_text(262, 138, 1, C_GRY, "LINES");
    cgx_textf(262, 148, 2, C_WHT, "%d", t->lines);
    cgx_text(262, 172, 1, C_GRY, "LEVEL");
    cgx_textf(262, 182, 2, C_WHT, "%d", t->level);
    cgx_textf(262, 202, 1, C_YEL, "BEST %d", t->best);

    if (t->over || paused)
    {
        cgx_rect(BX + 5, BY + 80, 90, 40, C_BLK);
        cgx_frame(BX + 5, BY + 80, 90, 40, 2, t->over ? C_RED : C_YEL);
        cgx_text_center(BX + 50, BY + 88, 2, t->over ? C_RED : C_YEL, t->over ? "GAME OVER" : "PAUSED");
        cgx_text_center(BX + 50, BY + 108, 1, C_WHT, t->over ? "PRESS EXE" : "OPTN");
    }
}

int main(void)
{
    static Tetris t;
    uint32_t last = 0;
    int acc = 0, started = 0, paused = 0;
    int dir_held_ms = 0, dir_prev = 0, down_acc = 0;

    cgx_init();
    tetris_init(&t);

    for (;;)
    {
        int n = cgx_steps(&last, &acc, 10, 10);
        int ms = n * 10;

        cgx_poll();
        if (cgx_pressed(K_EXIT))
            break;

        if (!started && cgx_any_pressed())
        {
            cgx_seed(cgx_ms() * 2654435761u + 5u);
            tetris_init(&t);
            started = 1;
        }

        if (t.over)
        {
            if (cgx_pressed(K_OK))
                tetris_init(&t);
        }
        else
        {
            if (cgx_pressed(K_PAUSE))
                paused = !paused;

            if (!paused)
            {
                int dir = cgx_down(K_LEFT) ? -1 : (cgx_down(K_RIGHT) ? 1 : 0);

                // Delayed auto-shift: first move at once, then repeat after 170 ms.
                if (dir != dir_prev)
                {
                    dir_held_ms = 0;
                    if (dir)
                        tetris_move(&t, dir);
                }
                else if (dir)
                {
                    dir_held_ms += ms;
                    while (dir_held_ms >= 170 + 50)
                    {
                        dir_held_ms -= 50;
                        tetris_move(&t, dir);
                    }
                }
                dir_prev = dir;

                if (cgx_down(K_DOWN))
                {
                    down_acc += ms;
                    while (down_acc >= 40)
                    {
                        down_acc -= 40;
                        tetris_soft_drop(&t);
                    }
                }
                else
                    down_acc = 0;

                if (cgx_pressed(K_UP))    tetris_hard_drop(&t);
                if (cgx_pressed(K_OK))    tetris_rotate(&t, 1);
                if (cgx_pressed(K_ALT))   tetris_rotate(&t, -1);
                if (cgx_pressed(K_ALT2))  tetris_hold(&t);

                tetris_tick(&t, ms);
            }
        }

        draw(&t, paused);
        cgx_present();
    }

    return 0;
}
