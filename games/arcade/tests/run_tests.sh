#!/bin/sh
# Host-side tests for the arcade games: compiles each game's logic with the
# stub gint headers and runs its test, then builds each game binary and runs
# it headlessly for a number of frames with scripted input, writing a
# screenshot of every game to $OUT (default: build-host/shots).
#   tests/run_tests.sh [--shots]
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/.." && pwd)
HOST=$(cd "$ROOT/../../common/host" && pwd)
OUT=${OUT:-$ROOT/build-host}
mkdir -p "$OUT/shots"
CFLAGS="-O1 -g -Wall -Wno-unused-function -I$ROOT/common -I$HERE -I$HOST/stubs"
FAIL=0

cc $CFLAGS -c "$HOST/host_gint.c" -o "$OUT/host_gint.o"
cc $CFLAGS -c "$ROOT/common/cgx.c" -o "$OUT/cgx.o"
cc $CFLAGS -c "$ROOT/common/font5x7.c" -o "$OUT/font.o"
LIBS="$OUT/host_gint.o $OUT/cgx.o $OUT/font.o -lm -lpthread"

for g in g2048 snake mines flappy tetris sokoban pacman doodle; do
  src=$(ls "$ROOT/$g"/*.c | grep -v '/main.c')
  cc $CFLAGS -I"$ROOT/$g" "$HERE/test_$g.c" $src $LIBS -o "$OUT/test_$g"
  "$OUT/test_$g" || FAIL=1
done

# Headless runs of the real game binaries (draws every frame via cgx).
KEYS_g2048="5:LEFT:1,7:LEFT:0,9:UP:1,11:UP:0,13:RIGHT:1,15:RIGHT:0,17:DOWN:1,19:DOWN:0,21:LEFT:1,23:LEFT:0,25:UP:1,27:UP:0"
KEYS_snake="5:DOWN:1,8:DOWN:0,30:RIGHT:1,33:RIGHT:0"
KEYS_mines="5:EXE:1,7:EXE:0,9:RIGHT:1,11:RIGHT:0,13:EXE:1,15:EXE:0,17:SHIFT:1,19:SHIFT:0"
KEYS_flappy="5:EXE:1,7:EXE:0,40:EXE:1,42:EXE:0"
KEYS_tetris="5:LEFT:1,9:LEFT:0,12:EXE:1,14:EXE:0,20:UP:1,22:UP:0,30:RIGHT:1,36:RIGHT:0"
KEYS_sokoban="5:RIGHT:1,7:RIGHT:0,9:UP:1,11:UP:0"
KEYS_pacman="5:LEFT:1,7:LEFT:0"
KEYS_doodle="5:LEFT:1,20:LEFT:0,25:RIGHT:1,40:RIGHT:0"
for g in g2048 snake mines flappy tetris sokoban pacman doodle; do
  src=$(ls "$ROOT/$g"/*.c)
  cc $CFLAGS -I"$ROOT/$g" $src $LIBS -o "$OUT/run_$g"
  eval keys=\$KEYS_$g
  HOST_FRAMES=60 HOST_DUMP=60 HOST_OUT="$OUT/shots" HOST_KEYS="$keys" "$OUT/run_$g" > /dev/null 2>"$OUT/$g.log" || true
  mv "$OUT/shots/frame0060.ppm" "$OUT/shots/$g.ppm" 2>/dev/null || { echo "$g: no screenshot"; FAIL=1; }
done
python3 -I "$HOST/ppm2png.py" "$OUT"/shots/*.ppm > /dev/null && rm -f "$OUT"/shots/*.ppm
echo "screenshots in $OUT/shots"
exit $FAIL
