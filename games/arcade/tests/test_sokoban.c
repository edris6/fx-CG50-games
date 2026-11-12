// Verifies every Sokoban level is solvable with a BFS over (player, boxes).
#include <stdlib.h>
#include <string.h>
#include "check.h"
#include "sokoban.h"

// Push-based BFS with player-region normalisation. Returns the number of
// pushes of an optimal solution, -1 if the level is provably unsolvable, or
// -2 if the state cap was hit.
#define MAXBOX 8
#define CAP (1 << 23)

typedef struct { uint8_t n; uint8_t box[MAXBOX]; uint8_t player; } State;

static int W, H;
static uint8_t wall[SK_MAXH * SK_MAXW], goal[SK_MAXH * SK_MAXW];

static int cmp(const void *a, const void *b) { return *(const uint8_t *) a - *(const uint8_t *) b; }

static uint64_t hash_of(const State *s)
{
    uint64_t h = 1469598103934665603ULL;
    int i;
    for (i = 0; i < s->n; ++i) { h ^= s->box[i] + 1; h *= 1099511628211ULL; }
    h ^= s->player + 7; h *= 1099511628211ULL;
    return h | 1;
}

static int has_box(const State *s, int c) { int i; for (i = 0; i < s->n; ++i) if (s->box[i] == c) return 1; return 0; }

// Flood the player's region; returns the canonical (lowest-index) cell.
static int region(const State *s, int start, uint8_t *mark)
{
    int stack[SK_MAXH * SK_MAXW], sp = 0, low = start;
    static const int dx[4] = { 0, 1, 0, -1 }, dy[4] = { -1, 0, 1, 0 };
    memset(mark, 0, W * H);
    stack[sp++] = start; mark[start] = 1;
    while (sp)
    {
        int c = stack[--sp], x = c % W, y = c / W, d;
        if (c < low) low = c;
        for (d = 0; d < 4; ++d)
        {
            int nx = x + dx[d], ny = y + dy[d], nc;
            if (nx < 0 || ny < 0 || nx >= W || ny >= H) continue;
            nc = ny * W + nx;
            if (wall[nc] || mark[nc] || has_box(s, nc)) continue;
            mark[nc] = 1; stack[sp++] = nc;
        }
    }
    return low;
}

static int solved_state(const State *s)
{
    int i;
    for (i = 0; i < s->n; ++i) if (!goal[s->box[i]]) return 0;
    return 1;
}

static long solve(int level)
{
    static const int dx[4] = { 0, 1, 0, -1 }, dy[4] = { -1, 0, 1, 0 };
    Sokoban sk;
    State *q = malloc(sizeof(State) * CAP / 4);
    int *dist = malloc(sizeof(int) * CAP / 4);
    uint64_t *table = calloc(CAP, sizeof(uint64_t));
    long head = 0, tail = 0, result = -1;
    uint8_t mark[SK_MAXH * SK_MAXW];
    State st;
    int x, y;

    sk_load(&sk, level);
    W = sk.w; H = sk.h;
    memset(&st, 0, sizeof(st));
    for (y = 0; y < H; ++y)
        for (x = 0; x < W; ++x)
        {
            wall[y * W + x] = !!(sk.cell[y][x] & SK_WALL);
            goal[y * W + x] = !!(sk.cell[y][x] & SK_GOAL);
            if (sk.cell[y][x] & SK_BOX) st.box[st.n++] = (uint8_t) (y * W + x);
        }
    qsort(st.box, st.n, 1, cmp);
    st.player = (uint8_t) region(&st, sk.py * W + sk.px, mark);

    q[tail] = st; dist[tail++] = 0;
    { uint64_t h = hash_of(&st); table[h & (CAP - 1)] = h; }

    while (head < tail)
    {
        State cur = q[head];
        int d0 = dist[head++], i, d;

        if (solved_state(&cur)) { result = d0; goto out; }
        region(&cur, cur.player, mark);

        for (i = 0; i < cur.n; ++i)
            for (d = 0; d < 4; ++d)
            {
                int bx = cur.box[i] % W, by = cur.box[i] / W;
                int px = bx - dx[d], py = by - dy[d], tx = bx + dx[d], ty = by + dy[d];
                State nx;
                uint64_t h;

                if (px < 0 || py < 0 || px >= W || py >= H || tx < 0 || ty < 0 || tx >= W || ty >= H) continue;
                if (!mark[py * W + px]) continue;
                if (wall[ty * W + tx] || has_box(&cur, ty * W + tx)) continue;

                nx = cur;
                nx.box[i] = (uint8_t) (ty * W + tx);
                qsort(nx.box, nx.n, 1, cmp);
                {
                    uint8_t m2[SK_MAXH * SK_MAXW];
                    nx.player = (uint8_t) region(&nx, by * W + bx, m2);
                }
                h = hash_of(&nx);
                {
                    uint64_t slot = h & (CAP - 1);
                    while (table[slot] && table[slot] != h) slot = (slot + 1) & (CAP - 1);
                    if (table[slot] == h) continue;
                    table[slot] = h;
                }
                if (tail >= CAP / 4) { result = -2; goto out; }
                q[tail] = nx; dist[tail++] = d0 + 1;
            }
    }
out:
    free(q); free(dist); free(table);
    return result;
}

int main(void)
{
    static Sokoban s;
    int l, n = sk_level_count();

    CHECK(n >= 5);

    for (l = 0; l < n; ++l)
    {
        long r;

        CHECK_EQ(sk_load(&s, l), 0);
        CHECK(s.w <= SK_MAXW && s.h <= SK_MAXH);
        CHECK(!sk_solved(&s));

        r = solve(l);
        if (r >= 0) printf("  level %d: solvable, %ld pushes\n", l + 1, r);
        else printf("  level %d: %s\n", l + 1, r == -1 ? "UNSOLVABLE" : "search cap hit");
        CHECK(r >= 0);
    }

    // undo restores state exactly
    sk_load(&s, 1);
    {
        Sokoban a = s;
        int i;
        for (i = 0; i < 12; ++i) sk_move(&s, i % 4);
        while (sk_undo(&s)) {}
        CHECK(memcmp(a.cell, s.cell, sizeof(a.cell)) == 0);
        CHECK(a.px == s.px && a.py == s.py && s.moves == 0 && s.pushes == 0);
    }

    CHECK_DONE("sokoban");
}
