// cg50_platform.h -- the small interface between the Doom engine and the
// fx-CG50 hardware (gint). Everything calculator-specific lives behind these
// functions so the engine files stay close to upstream.
#ifndef CG50_PLATFORM_H
#define CG50_PLATFORM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Video -------------------------------------------------------------------
void cg_video_init(void);
// Convert the 320x200 8-bit frame to RGB565 using `pal` and show it.
void cg_video_present(const uint8_t *frame, const uint16_t *pal);

// Time --------------------------------------------------------------------
void cg_time_init(void);
uint32_t cg_millis(void);

// Memory ------------------------------------------------------------------
// Allocate the biggest zone we can, between min_bytes and max_bytes.
void *cg_alloc_zone(size_t max_bytes, size_t min_bytes, size_t *got);

// Errors / exit -----------------------------------------------------------
// Show a message, wait for a key, return to the OS menu. Never returns.
void cg_fatal(const char *msg);
void cg_quit(void);

// On-screen debug line (shown in the corner when DOOM_CG50_DEBUG is set).
void cg_log(const char *msg);

#ifdef __cplusplus
}
#endif
#endif
