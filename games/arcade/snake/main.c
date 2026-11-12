// Snake for the fx-CG50.  Arrows steer, EXE restarts, EXIT quits.
#include "cgx.h"
#include "snake.h"

#define CELL 12
#define HUD 12

static void draw(const Snake *s)
{
    int x, y, i;

    cgx_clear(CGX_RGB(20, 28, 20));
    for (y = 0; y < SN_H; ++y)
        for (x = 0; x < SN_W; ++x)
            if ((x + y) & 1)
                cgx_rect(x * CELL, HUD + y * CELL, CELL, CELL, CGX_RGB(26, 36, 26));

    cgx_circle(s->fx * CELL + CELL / 2, HUD + s->fy * CELL + CELL / 2, 5, C_RED);
    cgx_rect(s->fx * CELL + CELL / 2, HUD + s->fy * CELL + 1, 2, 3, C_GRN);

    for (i = s->len - 1; i >= 0; --i)
    {
        int k = (s->head - i + SN_MAX) % SN_MAX;
        uint16_t c = i == 0 ? CGX_RGB(120, 240, 90) : CGX_RGB(60, 190, 70);
        cgx_rect(s->x[k] * CELL + 1, HUD + s->y[k] * CELL + 1, CELL - 2, CELL - 2, c);
    }

    cgx_rect(0, 0, CGX_W, HUD, C_BLK);
    cgx_textf(4, 2, 1, C_WHT, "SCORE %d", s->score);
    cgx_textf(CGX_W - 90, 2, 1, C_YEL, "BEST %d", s->best);

    if (s->over)
    {
        cgx_rect(CGX_W / 2 - 80, CGX_H / 2 - 24, 160, 48, C_BLK);
        cgx_frame(CGX_W / 2 - 80, CGX_H / 2 - 24, 160, 48, 2, C_RED);
        cgx_text_center(CGX_W / 2, CGX_H / 2 - 14, 3, C_RED, "GAME OVER");
        cgx_text_center(CGX_W / 2, CGX_H / 2 + 12, 1, C_WHT, "PRESS EXE");
    }
}

int main(void)
{
    static Snake s;
    uint32_t last = 0;
    int acc = 0, step_acc = 0, started = 0;

    cgx_init();
    snake_init(&s);

    for (;;)
    {
        int n = cgx_steps(&last, &acc, 10, 8);   // 10 ms base tick

        cgx_poll();
        if (cgx_pressed(K_EXIT))
            break;

        if (!started && cgx_any_pressed())
        {
            cgx_seed(cgx_ms() * 2654435761u + 99u);
            snake_init(&s);
            started = 1;
        }

        if (cgx_pressed(K_UP))    snake_turn(&s, SN_UP);
        if (cgx_pressed(K_DOWN))  snake_turn(&s, SN_DOWN);
        if (cgx_pressed(K_LEFT))  snake_turn(&s, SN_LEFT);
        if (cgx_pressed(K_RIGHT)) snake_turn(&s, SN_RIGHT);
        if (cgx_pressed(K_OK) && s.over)
            snake_init(&s);

        step_acc += n * 10;
        while (step_acc >= snake_step_ms(&s))
        {
            step_acc -= snake_step_ms(&s);
            snake_step(&s);
        }

        draw(&s);
        cgx_present();
    }

    return 0;
}
