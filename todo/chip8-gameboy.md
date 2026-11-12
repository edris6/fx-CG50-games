# Chip-8 / Game Boy emulators

- Chip-8: trivial interpreter, ~35 opcodes, a large library of small public-domain ROMs. Good weekend project.
- Game Boy: much heavier (LR35902 CPU, PPU, timers, MBC cartridges). 160x144 fits the screen at 1x.
  - The SH4AL-DSP at ~118 MHz is probably enough for DMG, with care.
  - ROMs must be user-supplied and stored in flash; do not commit ROMs to the repo.
