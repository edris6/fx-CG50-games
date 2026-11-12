# Doom for the Casio fx-CG50

A port of Doom (via the portable [doomgeneric](https://github.com/ozkl/doomgeneric)
source, upstream commit `e257a09`) to the fx-CG50 using the fxSDK and gint.

> **Status: written without access to a calculator or the fxSDK.**
> The whole engine plus the platform layer builds, links and *runs* on a Mac
> against stub gint headers (title screen, menus, E1M1, walking, shooting all
> render correctly into a 396x224 RGB565 VRAM image). What has **not** been
> tested is anything that touches the real hardware or the real gint/fxlibc
> headers. See "Things most likely to need a fix" below.

## What you need

- fxSDK with gint >= 2.1 (`giteapc install Lephenixnoir/fxsdk`, then gint)
- A WAD: `DOOM1.WAD` (shareware, free) or a retail/registered IWAD renamed to
  `DOOM1.WAD`. Not included in this repo.

## Build

```sh
fxsdk build-cg          # -> build-cg/Doom.g3a
```

## Prepare the WAD

The port has no sound, so strip it to save ~20% (the trimmed shareware WAD is
about 3.4 MB):

```sh
python3 tools/wadtrim.py DOOM1.WAD DOOM1.TRIM.WAD
# smaller still (only keep some levels):
python3 tools/wadtrim.py DOOM1.WAD DOOM1.TRIM.WAD --maps E1M1,E1M2,E1M3
```

Copy `Doom.g3a` **and** the WAD (named exactly `DOOM1.WAD`) to the root of the
calculator's storage memory using USB mass-storage mode. Check you have enough
free space first (the OS shows it in the Memory app).

## Controls

| Calculator key | Doom action |
|---|---|
| Arrow keys | Move forward/back, turn |
| EXE | Fire (and "Enter" in menus) |
| ALPHA | Use / open doors |
| SHIFT | Run (hold) |
| OPTN | Strafe modifier (hold + left/right) |
| F1 / F2 | Strafe left / right |
| F3 | Automap |
| 1 - 7 | Select weapon |
| + / - | Screen size |
| 0 | Pause |
| EXIT | Menu / back |
| DEL | Backspace (menus) |
| F5 / F6 | "Y" / "N" for prompts such as quit |

The key table is `keymap[]` in `src/platform/cg50_platform.c`; change it freely.

## How it works

```
src/doom/       doomgeneric engine, lightly patched (see "Patches")
src/platform/   everything calculator-specific:
  cg50_main.c      entry point, builds argv, runs the tick loop
  cg50_platform.c  video blit, keypad, 4 ms timer, zone allocation, error screen
  cg50_file.c      BFile-backed WAD reader + small FILE replacement
  cg50_compat.h    force-included; remaps libc calls fxlibc may lack
tools/wadtrim.py   WAD shrinker
tests/host/        build + run the real engine on a desktop with stub gint
```

- **Video:** Doom renders 320x200 8-bit. Each frame is converted through a
  256-entry RGB565 palette straight into gint's VRAM (two pixels per 32-bit
  store), centered in the 396x224 VRAM, then `dupdate()`.
- **WAD:** read lump-by-lump from storage memory with `BFile_Read`; never
  loaded whole.
- **Memory:** the zone heap is allocated at start-up by trying 3 MiB and
  stepping down in 64 KiB steps to 900 KiB, always leaving 96 KiB free for small
  allocations. Tune `DEFAULT_ZONE_KB` / `MIN_ZONE_KB` in `src/doom/i_system.c`.
- **Sound:** none. `FEATURE_SOUND` is not defined, so the engine skips it.
- **Saving:** config and savegames go through `cg_fopen` and are written to the
  root of storage memory when the file is closed.

## Patches against upstream doomgeneric

| File | Change |
|---|---|
| `i_video.c` | rewritten: palette to RGB565 + `cg_video_present()` |
| `i_system.c` | zone allocation via `cg_alloc_zone`, errors shown on screen, `I_Quit` returns to the OS menu, desktop error-box code removed |
| `m_misc.c`, `m_config.c` | `sscanf` replaced by `strtol`; `sys/stat.h` include removed |
| `doomgeneric.c` | no 32-bit screen buffer |
| `w_file_stdc.c` | replaced by `src/platform/cg50_file.c` |

## Things most likely to need a fix on first build

I wrote against gint from memory. If the build fails, look here first:

1. **gint API names** in `cg50_platform.c` / `cg50_file.c`: `timer_configure` +
   `GINT_CALL`, `clearevents` / `keydown`, `gint_vram`, `gint_osmenu`,
   `BFile_*` (especially the last argument of `BFile_Read`, which I treat as an
   absolute read offset), and key names such as `KEY_ADD` / `KEY_SUB`.
2. **RAM**: if start-up shows "Not enough RAM for the zone heap", lower
   `MIN_ZONE_KB`, or build with a smaller static footprint.
3. **Stack**: Doom recurses (BSP traversal) and uses some large locals. If it
   crashes in-level, the add-in stack is the first suspect.
4. **libc gaps**: anything missing from fxlibc will show up as an undefined
   symbol at link time; add it to `cg50_file.c`.
5. **Speed**: the FPS counter (top-left) is on by default. If it is too slow,
   try overclocking with a tool such as Ptune, reduce the view size with `-`,
   or trim the WAD to fewer levels.

## Host test harness

`tests/host/run_checks.sh [dir-with-DOOM1.WAD]` verifies the key constants,
compiles and links the entire port against stub gint headers, and (with a WAD)
boots the engine headlessly. Dump frames with `HOST_DUMP=10,200` and script
key presses with `HOST_KEYS="40:EXIT:1,45:EXIT:0"`; see `common/host/host_gint.c`.

## License

Doom is GPL-2.0-or-later (id Software, Simon Howard / Chocolate Doom, ozkl /
doomgeneric). See `LICENSE` and `LICENSE.doomgeneric`. The shareware and
retail WADs are **not** covered by this license and are not included.
