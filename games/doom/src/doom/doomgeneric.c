// doomgeneric.c -- startup glue (replaces the upstream file).
// Upstream allocates a 32-bit screen buffer here; on the calculator the video
// backend (i_video.c) blits straight to VRAM, so none is needed.

#include <stdio.h>

#include "m_argv.h"

#include "doomgeneric.h"

void M_FindResponseFile(void);
void D_DoomMain(void);

void doomgeneric_Create(int argc, char **argv)
{
    myargc = argc;
    myargv = argv;

    M_FindResponseFile();

    D_DoomMain();
}
