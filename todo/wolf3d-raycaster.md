# Wolfenstein 3D-style raycaster

- Grid raycaster (DDA), no BSP or visplanes, so RAM use is tiny.
- Render at 192x108 or 384x216 and scale; textured walls, flat floor and ceiling.
- Reuses the same framebuffer, input and timing code as the Doom port.
- Stretch goals: sprites for enemies, doors, a shooting mechanic.
- Open question: render resolution vs framerate on the SH4AL-DSP (no FPU, so fixed-point math only).
