#!/bin/sh
# Regenerates every game's icons (no external dependencies).
set -e
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
MK="python3 -I $ROOT/common/tools/make_icon.py"
$MK DOOM   "$ROOT/games/doom/assets-cg"            --color c01010
A="$ROOT/games/arcade"
$MK 2048   "$A/g2048/assets-cg"   --color eda23a
$MK TETRIS "$A/tetris/assets-cg"  --color 3cdce6
$MK SNAKE  "$A/snake/assets-cg"   --color 3cc850
$MK PACMAN "$A/pacman/assets-cg"  --color facc14
$MK SOKOBAN "$A/sokoban/assets-cg" --color d2963c
$MK MINES  "$A/mines/assets-cg"   --color 7890b4
$MK FLAPPY "$A/flappy/assets-cg"  --color 6ec8eb
$MK DOODLE "$A/doodle/assets-cg"  --color 96d23c
