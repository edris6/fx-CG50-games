// Flappy Bird for the fx-CG50.  EXE / UP / SHIFT flap, EXIT quits.
#include "cgx.h"
#include "flappy.h"

static void draw_pipe(int x, int top, int bottom)
{
    // top is the bottom edge of the upper pipe; bottom the top edge of the lower.
    uint16_t body = CGX_RGB(90, 190, 50), edge = CGX_RGB(40, 120, 30);

    cgx_rect(x, 0, FB_PIPE_W, top, body);
    cgx_frame(x, 0, FB_PIPE_W, top, 2, edge);
    cgx_rect(x - 3, top - 12, FB_PIPE_W + 6, 12, body);
    cgx_frame(x - 3, top - 12, FB_PIPE_W + 6, 12, 2, edge);

    cgx_rect(x, bottom, FB_PIPE_W, FB_GROUND - bottom, body);
    cgx_frame(x, bottom, FB_PIPE_W, FB_GROUND - bottom, 2, edge);
    cgx_rect(x - 3, bottom, FB_PIPE_W + 6, 12, body);
    cgx_frame(x - 3, bottom, FB_PIPE_W + 6, 12, 2, edge);
}

static void draw(const Flappy *f)
{
    int i, by = flappy_bird_y(f);

    cgx_clear(CGX_RGB(110, 200, 235));
    // simple clouds
    cgx_circle(60, 40, 12, C_WHT); cgx_circle(76, 44, 10, C_WHT); cgx_circle(46, 46, 9, C_WHT);
    cgx_circle(260, 30, 12, C_WHT); cgx_circle(278, 34, 10, C_WHT); cgx_circle(244, 36, 9, C_WHT);

    for (i = 0; i < FB_NPIPES; ++i)
        if (f->pipe[i].x < FB_W && f->pipe[i].x + FB_PIPE_W > 0)
            draw_pipe(f->pipe[i].x, f->pipe[i].gap_y, f->pipe[i].gap_y + FB_GAP);

    cgx_rect(0, FB_GROUND, FB_W, FB_H - FB_GROUND, CGX_RGB(222, 200, 130));
    cgx_rect(0, FB_GROUND, FB_W, 4, CGX_RGB(100, 190, 50));
    for (i = -1; i < FB_W / 24 + 1; ++i)
        cgx_rect(i * 24 - f->scroll + 24, FB_GROUND + 8, 12, 4, CGX_RGB(200, 175, 100));

    // the bird: body, wing, eye, beak
    cgx_rect(FB_BIRD_X, by, FB_BIRD_W, FB_BIRD_H, C_YEL);
    cgx_rect(FB_BIRD_X + 2, by + 6, 8, 5, CGX_RGB(240, 180, 20));
    cgx_rect(FB_BIRD_X + 11, by + 2, 4, 4, C_WHT);
    cgx_rect(FB_BIRD_X + 13, by + 3, 2, 2, C_BLK);
    cgx_rect(FB_BIRD_X + 14, by + 7, 5, 3, C_ORG);

    cgx_textf(CGX_W / 2 - 10, 8, 4, C_WHT, "%d", f->score);

    if (f->state == FB_READY)
        cgx_text_center(CGX_W / 2, 150, 2, C_WHT, "PRESS EXE TO FLAP");
    if (f->state == FB_DEAD)
    {
        cgx_rect(CGX_W / 2 - 80, 70, 160, 62, CGX_RGB(222, 200, 130));
        cgx_frame(CGX_W / 2 - 80, 70, 160, 62, 2, CGX_RGB(120, 80, 30));
        cgx_text_center(CGX_W / 2, 76, 3, CGX_RGB(220, 70, 30), "GAME OVER");
        cgx_textf(CGX_W / 2 - 66, 102, 2, CGX_RGB(90, 60, 20), "SCORE %d", f->score);
        cgx_textf(CGX_W / 2 - 66, 118, 1, CGX_RGB(90, 60, 20), "BEST %d   EXE: RETRY", f->best);
    }
}

int main(void)
{
    static Flappy f;
    uint32_t last = 0;
    int acc = 0, started = 0;

    cgx_init();
    flappy_init(&f);

    for (;;)
    {
        int n = cgx_steps(&last, &acc, 16, 4);     // ~60 steps per second
        int i;

        cgx_poll();
        if (cgx_pressed(K_EXIT))
            break;

        if (!started && cgx_any_pressed())
        {
            cgx_seed(cgx_ms() * 2654435761u + 3u);
            flappy_init(&f);
            started = 1;
        }

        if (cgx_pressed(K_OK) || cgx_pressed(K_UP) || cgx_pressed(K_ALT))
        {
            if (f.state == FB_DEAD && f.ticks > 0)
            {
                flappy_init(&f);
            }
            else
                flappy_flap(&f);
        }

        for (i = 0; i < n; ++i)
            flappy_step(&f);

        draw(&f);
        cgx_present();
    }

    return 0;
}
