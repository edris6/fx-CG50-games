// Pac-Man style game for the fx-CG50.  Arrows steer, EXE restarts after game
// over, EXIT quits.
#include "cgx.h"
#include "pacman.h"

#define T 11                          // pixels per tile
#define MX 6
#define MY 4

static void draw_pac(const Pacman *p, int cx, int cy, int r, int dir, int mouth_open)
{
    int x, y, d = dir < 0 ? PM_LEFT : dir;

    for (y = -r; y <= r; ++y)
        for (x = -r; x <= r; ++x)
        {
            int along, across;

            if (x * x + y * y > r * r)
                continue;

            switch (d)
            {
                case PM_RIGHT: along = x;  across = y; break;
                case PM_LEFT:  along = -x; across = y; break;
                case PM_UP:    along = -y; across = x; break;
                default:       along = y;  across = x; break;
            }
            if (across < 0)
                across = -across;

            if (mouth_open && along > 0 && across * 2 < along * mouth_open)
                continue;

            cgx_pixel(cx + x, cy + y, C_YEL);
        }
    (void) p;
}

static void draw_ghost(const Pacman *p, int i)
{
    static const uint16_t cols[4] = { CGX_RGB(240, 40, 40), CGX_RGB(255, 150, 200), CGX_RGB(60, 220, 240), CGX_RGB(255, 170, 40) };
    const PmActor *g = &p->gh[i];
    int cx = MX + g->x * T / PM_U, cy = MY + g->y * T / PM_U, r = 5, x, y;
    uint16_t body = cols[i];

    if (g->mode == GH_FRIGHT)
        body = (p->fright < 120 && (p->ticks / 10) % 2) ? C_WHT : CGX_RGB(40, 60, 220);

    if (g->mode != GH_EYES)
    {
        for (y = -r; y <= r; ++y)
            for (x = -r; x <= r; ++x)
                if ((y <= 0 && x * x + y * y <= r * r) || (y > 0 && y <= r && x >= -r && x <= r))
                    if (!(y == r && ((x + p->ticks / 8) & 2)))   // wavy skirt
                        cgx_pixel(cx + x, cy + y, body);
    }

    if (g->mode == GH_FRIGHT)
    {
        cgx_rect(cx - 3, cy - 2, 2, 2, C_WHT);
        cgx_rect(cx + 1, cy - 2, 2, 2, C_WHT);
    }
    else
    {
        int ex = g->dir >= 0 ? (g->dir == PM_RIGHT) - (g->dir == PM_LEFT) : 0;
        int ey = g->dir >= 0 ? (g->dir == PM_DOWN) - (g->dir == PM_UP) : 0;

        cgx_rect(cx - 4, cy - 3, 3, 3, C_WHT);
        cgx_rect(cx + 1, cy - 3, 3, 3, C_WHT);
        cgx_rect(cx - 3 + ex, cy - 2 + ey, 2, 2, C_BLU);
        cgx_rect(cx + 2 + ex, cy - 2 + ey, 2, 2, C_BLU);
    }
}

static void draw(const Pacman *p)
{
    int x, y, i;

    cgx_clear(C_BLK);

    for (y = 0; y < PM_H; ++y)
        for (x = 0; x < PM_W; ++x)
        {
            int px = MX + x * T, py = MY + y * T;

            switch (p->maze[y][x])
            {
                case PM_WALL:
                    cgx_rect(px, py, T, T, CGX_RGB(20, 20, 120));
                    cgx_frame(px, py, T, T, 1, CGX_RGB(60, 80, 255));
                    break;
                case PM_PELLET:
                    cgx_rect(px + T / 2 - 1, py + T / 2 - 1, 2, 2, CGX_RGB(255, 210, 180));
                    break;
                case PM_POWER:
                    if ((p->ticks / 15) % 2 == 0)
                        cgx_circle(px + T / 2, py + T / 2, 3, CGX_RGB(255, 210, 180));
                    break;
                default:
                    break;
            }
        }

    if (p->state != PM_DYING && p->state != PM_GAMEOVER)
        draw_pac(p, MX + p->pac.x * T / PM_U, MY + p->pac.y * T / PM_U, 5, p->pac.dir,
                 p->pac.dir < 0 ? 0 : ((p->ticks / 5) % 3) * 2);
    else if (p->state == PM_DYING)
    {
        int r = 5 - (100 - p->state_ticks) / 20;
        if (r > 0)
            cgx_circle(MX + p->pac.x * T / PM_U, MY + p->pac.y * T / PM_U, r, C_YEL);
    }

    if (p->state != PM_DYING || p->state_ticks > 70)
        for (i = 0; i < 4; ++i)
            draw_ghost(p, i);

    if (p->last_eaten_ticks > 0)
        cgx_textf(MX + p->last_eaten_x * T / PM_U - 8, MY + p->last_eaten_y * T / PM_U - 3, 1, C_CYN, "%d", p->last_eaten_score);

    // side panel
    cgx_text(250, 16, 3, C_YEL, "PAC");
    cgx_text(250, 42, 2, C_WHT, "SCORE");
    cgx_textf(250, 58, 2, C_WHT, "%d", p->score);
    cgx_text(250, 84, 2, C_WHT, "BEST");
    cgx_textf(250, 100, 2, C_YEL, "%d", p->best);
    cgx_textf(250, 126, 2, C_WHT, "LEVEL %d", p->level);
    for (i = 0; i < p->lives - 1; ++i)
        draw_pac(p, 258 + i * 16, 160, 6, PM_RIGHT, 2);
    cgx_text(250, 188, 1, C_GRY, "ARROWS: STEER");
    cgx_text(250, 200, 1, C_GRY, "EXIT: QUIT");

    if (p->state == PM_READY)
        cgx_text_center(MX + PM_W * T / 2, MY + 10 * T - 2, 1, C_YEL, "READY!");
    if (p->state == PM_GAMEOVER)
    {
        cgx_rect(MX + 30, MY + 80, 170, 40, C_BLK);
        cgx_frame(MX + 30, MY + 80, 170, 40, 2, C_RED);
        cgx_text_center(MX + PM_W * T / 2, MY + 88, 3, C_RED, "GAME OVER");
        cgx_text_center(MX + PM_W * T / 2, MY + 108, 1, C_WHT, "PRESS EXE");
    }
}

int main(void)
{
    static Pacman p;
    uint32_t last = 0;
    int acc = 0, started = 0;

    cgx_init();
    pm_init(&p);

    for (;;)
    {
        int n = cgx_steps(&last, &acc, 16, 4), i;

        cgx_poll();
        if (cgx_pressed(K_EXIT))
            break;

        if (!started && cgx_any_pressed())
        {
            cgx_seed(cgx_ms() * 2654435761u + 11u);
            started = 1;
        }

        if (cgx_pressed(K_UP) || cgx_down(K_UP))       pm_turn(&p, PM_UP);
        if (cgx_pressed(K_DOWN) || cgx_down(K_DOWN))   pm_turn(&p, PM_DOWN);
        if (cgx_pressed(K_LEFT) || cgx_down(K_LEFT))   pm_turn(&p, PM_LEFT);
        if (cgx_pressed(K_RIGHT) || cgx_down(K_RIGHT)) pm_turn(&p, PM_RIGHT);

        if (p.state == PM_GAMEOVER && cgx_pressed(K_OK))
            pm_init(&p);

        for (i = 0; i < n; ++i)
            pm_step(&p);

        draw(&p);
        cgx_present();
    }

    return 0;
}
