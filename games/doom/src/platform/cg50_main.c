// cg50_main.c -- add-in entry point for Doom on the fx-CG50.
//
// Needs DOOM1.WAD (shareware or retail) in the root of storage memory.
// Run tools/wadtrim.py first to shrink it so it fits and loads faster.

#define CG50_NO_REMAP
#include "cg50_compat.h"

#include "cg50_platform.h"
#include "doomgeneric.h"

int main(void)
{
    static char *args[] = {
        "doom",
        "-iwad", "DOOM1.WAD",
        NULL,
    };

    cg_time_init();

    doomgeneric_Create(3, args);

    for (;;)
        doomgeneric_Tick();

    return 0;
}
