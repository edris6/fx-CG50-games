// Minimal assertion helpers for the host-side logic tests.
#ifndef CHECK_H
#define CHECK_H
#include <stdio.h>
static int check_failures, check_count;
#define CHECK(cond) do { check_count++; if (!(cond)) { check_failures++; \
    printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long) (a), _b = (long) (b); check_count++; \
    if (_a != _b) { check_failures++; printf("  FAIL %s:%d: %s == %s (%ld vs %ld)\n", \
    __FILE__, __LINE__, #a, #b, _a, _b); } } while (0)
#define CHECK_DONE(name) do { printf("%-10s %d checks, %d failed\n", name, check_count, check_failures); \
    return check_failures != 0; } while (0)
#endif
