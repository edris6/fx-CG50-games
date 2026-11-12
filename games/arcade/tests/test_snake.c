#include <string.h>
#include "check.h"
#include "cgx.h"
#include "snake.h"

int main(void)
{
    static Snake s;
    int hx, i;

    cgx_seed(5);
    memset(&s, 0, sizeof(s));
    snake_init(&s);
    CHECK_EQ(s.len, 3);
    CHECK(!snake_occupies(&s, s.fx, s.fy));

    hx = s.x[s.head];
    snake_step(&s);
    CHECK_EQ(s.x[s.head], hx + 1);
    CHECK_EQ(s.len, 3);

    // reversing is ignored
    snake_turn(&s, SN_LEFT);
    snake_step(&s);
    CHECK_EQ(s.x[s.head], hx + 2);
    CHECK(!s.over);

    // eating grows and scores
    memset(&s, 0, sizeof(s));
    snake_init(&s);
    s.fx = s.x[s.head] + 1; s.fy = s.y[s.head];
    snake_step(&s);
    CHECK_EQ(s.len, 4);
    CHECK_EQ(s.score, 10);
    CHECK(!snake_occupies(&s, s.fx, s.fy));

    // hitting the wall ends the game
    memset(&s, 0, sizeof(s));
    snake_init(&s);
    s.fx = 0; s.fy = 0;
    for (i = 0; i < SN_W + 2 && !s.over; ++i)
        snake_step(&s);
    CHECK(s.over);

    // running into yourself ends the game (needs a snake of length >= 5)
    memset(&s, 0, sizeof(s));
    snake_init(&s);
    s.len = 5; s.head = 4;
    for (i = 0; i < 5; ++i) { s.x[i] = 6 + i; s.y[i] = 8; }    // tail (6,8) .. head (10,8)
    s.dir = SN_RIGHT;
    s.fx = 0; s.fy = 0;
    snake_turn(&s, SN_UP);      // (10,7)
    snake_step(&s);
    snake_turn(&s, SN_LEFT);    // (9,7)
    snake_step(&s);
    CHECK(!s.over);
    snake_turn(&s, SN_DOWN);    // (9,8) is still body -> dies
    snake_step(&s);
    CHECK(s.over);

    // moving into the cell the tail just left is allowed
    memset(&s, 0, sizeof(s));
    snake_init(&s);
    s.len = 4; s.head = 3;
    s.x[0] = 5; s.y[0] = 8; s.x[1] = 5; s.y[1] = 9; s.x[2] = 6; s.y[2] = 9; s.x[3] = 6; s.y[3] = 8;
    s.dir = SN_UP; s.fx = 0; s.fy = 0;
    snake_turn(&s, SN_LEFT);    // head (6,8) -> (5,8) is the tail cell, which is vacated
    snake_step(&s);
    CHECK(!s.over);

    // speed increases as the snake grows
    CHECK(snake_step_ms(&s) <= 150);

    CHECK_DONE("snake");
}
