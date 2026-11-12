// Doodle Jump style game for the fx-CG50.
//   LEFT/RIGHT steer (the character wraps around the screen edges)
//   EXE restarts after you fall   EXIT quits
#include "cgx.h"
#include "doodle.h"

#define PX ((CGX_W - DJ_W) / 2)

static int sy(const Doodle *d, int world_y)
{
    return DJ_H - (world_y - d->cam);
}

static void draw_plat(const Doodle *d, const DjPlat *p)
{
    int y = sy(d, p->y), x = PX + p->x;
    uint16_t c = p->type == DJ_MOVING ? CGX_RGB(70, 140, 240)
               : p->type == DJ_BREAKABLE ? CGX_RGB(170, 110, 60) : CGX_RGB(90, 200, 60);

    if (y < -10 || y > DJ_H + 10)
        return;

    if (p->broken)
        y += 14;                    // falling pieces
    cgx_rect(x, y, DJ_PW, DJ_PH, c);
    cgx_rect(x, y, DJ_PW, 2, CGX_RGB(200, 240, 170));
    cgx_frame(x, y, DJ_PW, DJ_PH, 1, CGX_RGB(40, 90, 30));
    if (p->type == DJ_BREAKABLE)
        cgx_rect(x + DJ_PW / 2, y, 2, DJ_PH, CGX_RGB(90, 50, 20));

    if (p->spring)
    {
        cgx_rect(x + DJ_PW / 2 - 4, y - 7, 8, 7, CGX_RGB(220, 40, 40));
        cgx_rect(x + DJ_PW / 2 - 4, y - 4, 8, 1, C_WHT);
    }
}

static void draw_player(const Doodle *d)
{
    int x = PX + (d->x >> 8), y = sy(d, d->y >> 8);     // y = feet
    int squash = d->bounce_anim > 6 ? 3 : 0;
    int top = y - DJ_PLH + squash;

    // wrap-around: draw a second copy when near an edge
    int copies = (x < 0 + PX) ? 2 : ((x + DJ_PLW > PX + DJ_W) ? 2 : 1), c;

    for (c = 0; c < copies; ++c)
    {
        int bx = x + (c == 1 ? (x < PX + DJ_W / 2 ? DJ_W : -DJ_W) : 0);

        cgx_rect(bx + 2, top + 2, DJ_PLW - 4, DJ_PLH - 8 - squash, CGX_RGB(150, 210, 60));
        cgx_rect(bx, top + 6, DJ_PLW, DJ_PLH - 12 - squash, CGX_RGB(150, 210, 60));
        cgx_rect(bx + 3, top + DJ_PLH - 7 - squash, 4, 5 + squash, CGX_RGB(90, 150, 30));
        cgx_rect(bx + DJ_PLW - 7, top + DJ_PLH - 7 - squash, 4, 5 + squash, CGX_RGB(90, 150, 30));
        cgx_rect(bx + (d->face > 0 ? 9 : 3), top + 5, 4, 4, C_WHT);
        cgx_rect(bx + (d->face > 0 ? 11 : 3), top + 6, 2, 2, C_BLK);
        cgx_rect(bx + (d->face > 0 ? DJ_PLW : -4), top + 9, 4, 3, CGX_RGB(90, 150, 30));
    }
}

static void draw(const Doodle *d)
{
    int i, gy;

    cgx_clear(CGX_RGB(40, 40, 52));
    cgx_rect(PX, 0, DJ_W, DJ_H, CGX_RGB(250, 246, 220));
    for (gy = -(d->cam % 20); gy < DJ_H; gy += 20)
        cgx_rect(PX, gy, DJ_W, 1, CGX_RGB(220, 230, 215));
    for (i = 0; i <= DJ_W; i += 20)
        cgx_rect(PX + i, 0, 1, DJ_H, CGX_RGB(220, 230, 215));

    for (i = 0; i < DJ_NPLAT; ++i)
        draw_plat(d, &d->plat[i]);
    draw_player(d);

    cgx_text(8, 16, 2, C_WHT, "SCORE");
    cgx_textf(8, 34, 3, C_YEL, "%d", d->score);
    cgx_text(8, 70, 2, C_WHT, "BEST");
    cgx_textf(8, 88, 2, C_GRY, "%d", d->best);
    cgx_text(PX + DJ_W + 10, 16, 1, C_GRY, "LEFT/RIGHT");
    cgx_text(PX + DJ_W + 10, 28, 1, C_GRY, "STEER");
    cgx_text(PX + DJ_W + 10, 196, 1, C_GRY, "EXIT: QUIT");

    if (d->state == DJ_DEAD)
    {
        cgx_rect(PX + 20, 80, DJ_W - 40, 50, C_BLK);
        cgx_frame(PX + 20, 80, DJ_W - 40, 50, 2, C_RED);
        cgx_text_center(PX + DJ_W / 2, 88, 3, C_RED, "YOU FELL");
        cgx_textf(PX + DJ_W / 2 - 40, 112, 1, C_WHT, "SCORE %d  EXE: RETRY", d->score);
    }
}

int main(void)
{
    static Doodle d;
    uint32_t last = 0;
    int acc = 0, started = 0;

    cgx_init();
    dj_init(&d);

    for (;;)
    {
        int n = cgx_steps(&last, &acc, 16, 4), i;
        int steer;

        cgx_poll();
        if (cgx_pressed(K_EXIT))
            break;

        if (!started && cgx_any_pressed())
        {
            cgx_seed(cgx_ms() * 2654435761u + 17u);
            dj_init(&d);
            started = 1;
        }

        steer = cgx_down(K_LEFT) ? -1 : (cgx_down(K_RIGHT) ? 1 : 0);
        if (d.state == DJ_DEAD && cgx_pressed(K_OK))
            dj_init(&d);

        for (i = 0; i < n; ++i)
            dj_step(&d, steer);

        draw(&d);
        cgx_present();
    }

    return 0;
}
