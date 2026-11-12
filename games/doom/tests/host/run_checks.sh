#!/bin/sh
# Host-side checks you can run without a calculator.
#   1. DOOM_* key constants match doomkeys.h
#   2. the whole port compiles + links with stub gint headers
#   3. (optional) boots the real engine headlessly if a WAD is given:
#        tests/host/run_checks.sh /path/to/dir/containing/DOOM1.WAD
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

cc -w -I"$ROOT/src/doom" "$HERE/check_keys.c" -o "$TMP/ck" && "$TMP/ck"
(cd "$TMP" && "$HERE/build.sh" "$TMP/out")

if [ -n "$1" ]; then
  # Menu: ESC, New Game, episode, skill -> then walk forward and shoot.
  HOST_KEYS="40:EXIT:1,45:EXIT:0,60:EXE:1,65:EXE:0,100:EXE:1,105:EXE:0,140:EXE:1,145:EXE:0,400:UP:1,800:UP:0,820:EXE:1,825:EXE:0" \
  HOST_DIR="$1" HOST_FRAMES=900 HOST_OUT="$TMP" HOST_DUMP=899 \
    "$TMP/out/doom-host" > "$TMP/log.txt" 2>&1 || true
  grep -q "reached 900 frames" "$TMP/log.txt" && echo "engine boot + 900 frames OK" || { tail -20 "$TMP/log.txt"; exit 1; }
fi
