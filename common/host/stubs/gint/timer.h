#pragma once
#include <stdint.h>
#define TIMER_ANY (-1)
#define TIMER_CONTINUE 0
typedef struct { int (*fn)(void *); void *arg; } gint_call_t;
#define GINT_CALL(f, a) ((gint_call_t){ (int (*)(void *)) (f), (void *) (a) })
int timer_configure(int timer, uint64_t delay_us, gint_call_t cb);
int timer_start(int timer);
