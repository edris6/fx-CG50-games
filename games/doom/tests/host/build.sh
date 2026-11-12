#!/bin/sh
# Build the whole port for the host with stub gint headers, to catch compile
# and link errors (and run the real engine headlessly) without a calculator.
# Usage: tests/host/build.sh [outdir] [extra cc flags, e.g. -fsanitize=address]
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
COMMON_HOST=$(cd "$ROOT/../../common/host" && pwd)
OUT=${1:-$ROOT/build-host}
shift 2>/dev/null || true
mkdir -p "$OUT"
SRCS=$(ls "$ROOT"/src/doom/*.c "$ROOT"/src/platform/*.c)
COMMON="-O1 -g -w -DNORMALUNIX -DDOOM_SHOW_FPS $*"
# Engine + platform files get the same force-included compat header the
# calculator build uses.
cc $COMMON -c \
   -include "$ROOT/src/platform/cg50_compat.h" \
   -I"$ROOT/src/doom" -I"$ROOT/src/platform" -I"$COMMON_HOST/stubs" \
   $SRCS
# The host harness itself must see the real libc.
cc $COMMON -c -I"$COMMON_HOST/stubs" "$COMMON_HOST/host_gint.c"
cc $COMMON *.o -lm -lpthread -o "$OUT/doom-host"
rm -f *.o
echo "built $OUT/doom-host"
