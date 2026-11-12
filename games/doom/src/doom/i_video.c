//
// i_video.c -- fx-CG50 video backend for Doom.
//
// Replaces the stock doomgeneric/chocolate-doom video code. Doom renders into
// an 8-bit 320x200 buffer (I_VideoBuffer). On every frame we convert it to
// RGB565 through a 256-entry palette and hand it to the platform layer
// (src/platform/cg50_platform.c), which writes it into the gint VRAM.
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// GPL v2 or later.
//

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "config.h"
#include "d_event.h"
#include "d_main.h"
#include "doomkeys.h"
#include "doomtype.h"
#include "i_system.h"
#include "i_video.h"
#include "m_argv.h"
#include "tables.h"
#include "v_video.h"
#include "z_zone.h"

#include "cg50_platform.h"

void I_GetEvent(void);

// ---------------------------------------------------------------------------
// Variables the rest of the engine (mostly m_config.c) expects to exist.
// ---------------------------------------------------------------------------

byte *I_VideoBuffer = NULL;

boolean screensaver_mode = false;
boolean screenvisible = true;

float mouse_acceleration = 2.0f;
int mouse_threshold = 10;
int usemouse = 0;

int usegamma = 0;

char *video_driver = "";
int screen_width = SCREENWIDTH;
int screen_height = SCREENHEIGHT;
int screen_bpp = 16;
int fullscreen = 0;
int aspect_ratio_correct = 0;
int show_diskicon = 0;
int diskicon_readbytes = 0;
int fb_scaling = 1;

// Palette converted to RGB565, indexed by Doom palette index.
static uint16_t rgb565_palette[256];
// The same palette as 8-bit RGB, kept for I_GetPaletteIndex().
static byte rgb_palette[256][3];

#define RGB565(r, g, b) \
    ((uint16_t) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3)))

// ---------------------------------------------------------------------------

void I_InitGraphics(void)
{
    // The zone allocator owns the buffer so it follows Doom's memory rules.
    I_VideoBuffer = (byte *) Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
    memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT);

    screenvisible = true;

    cg_video_init();
}

void I_ShutdownGraphics(void)
{
    Z_Free(I_VideoBuffer);
    I_VideoBuffer = NULL;
}

void I_StartFrame(void)
{
}

void I_StartTic(void)
{
    I_GetEvent();
}

void I_UpdateNoBlit(void)
{
}

void I_FinishUpdate(void)
{
    cg_video_present(I_VideoBuffer, rgb565_palette);
}

void I_ReadScreen(byte *scr)
{
    memcpy(scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
}

void I_SetPalette(byte *palette)
{
    int i;

    for (i = 0; i < 256; ++i)
    {
        byte r = gammatable[usegamma][*palette++];
        byte g = gammatable[usegamma][*palette++];
        byte b = gammatable[usegamma][*palette++];

        rgb_palette[i][0] = r;
        rgb_palette[i][1] = g;
        rgb_palette[i][2] = b;
        rgb565_palette[i] = RGB565(r, g, b);
    }
}

// Given an RGB value, find the closest matching palette index.
int I_GetPaletteIndex(int r, int g, int b)
{
    int best = 0;
    int best_diff = INT_MAX;
    int i;

    for (i = 0; i < 256; ++i)
    {
        int dr = r - rgb_palette[i][0];
        int dg = g - rgb_palette[i][1];
        int db = b - rgb_palette[i][2];
        int diff = dr * dr + dg * dg + db * db;

        if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }

        if (diff == 0)
        {
            break;
        }
    }

    return best;
}

void I_BeginRead(void)
{
}

void I_EndRead(void)
{
}

void I_SetWindowTitle(char *title)
{
    (void) title;
}

void I_GraphicsCheckCommandLine(void)
{
}

void I_SetGrabMouseCallback(grabmouse_callback_t func)
{
    (void) func;
}

void I_EnableLoadingDisk(void)
{
}

void I_BindVideoVariables(void)
{
}

void I_DisplayFPSDots(boolean dots_on)
{
    (void) dots_on;
}

void I_CheckIsScreensaver(void)
{
}

void I_InitWindowTitle(void)
{
}

void I_InitWindowIcon(void)
{
}
