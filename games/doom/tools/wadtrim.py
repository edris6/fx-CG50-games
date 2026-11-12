#!/usr/bin/env python3
"""wadtrim.py -- shrink a Doom IWAD so it fits comfortably on the fx-CG50.

The calculator port has no sound, so sound effects (DS*, DP*) and music
(D_*) lumps are dead weight. This tool rewrites the WAD with those lumps
emptied (the directory entries are kept so lump numbering never changes).

Optionally, --maps removes every level except the ones you list.

    python3 tools/wadtrim.py DOOM1.WAD DOOM1.TRIM.WAD
    python3 tools/wadtrim.py DOOM1.WAD out.wad --maps E1M1,E1M2,E1M3

Name the output DOOM1.WAD when copying it to the calculator.
"""
import argparse
import re
import struct
import sys

MAP_LUMPS = 10  # lumps that follow a map marker (THINGS ... BLOCKMAP)
MAP_RE = re.compile(r"^(E\dM\d|MAP\d\d)$")


def read_wad(path):
    data = open(path, "rb").read()
    ident, numlumps, diroff = struct.unpack_from("<4sii", data, 0)
    if ident not in (b"IWAD", b"PWAD"):
        sys.exit(f"{path}: not a WAD file")
    lumps = []
    for i in range(numlumps):
        pos, size, name = struct.unpack_from("<ii8s", data, diroff + 16 * i)
        name = name.split(b"\0")[0].decode("ascii")
        lumps.append((name, data[pos:pos + size]))
    return ident, lumps


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("input")
    ap.add_argument("output")
    ap.add_argument("--maps", help="comma-separated maps to keep, e.g. E1M1,E1M2")
    ap.add_argument("--keep-sound", action="store_true", help="do not strip sound/music")
    args = ap.parse_args()

    keep_maps = set(args.maps.upper().split(",")) if args.maps else None
    ident, lumps = read_wad(args.input)

    out = []
    drop_after_marker = 0
    for name, data in lumps:
        up = name.upper()
        if drop_after_marker > 0:
            drop_after_marker -= 1
            out.append((name, b""))
            continue
        if MAP_RE.match(up) and keep_maps is not None and up not in keep_maps:
            drop_after_marker = MAP_LUMPS
            out.append((name, b""))
            continue
        if not args.keep_sound and (up.startswith(("DS", "DP", "D_"))):
            out.append((name, b""))
            continue
        out.append((name, data))

    body = bytearray()
    entries = []
    for name, data in out:
        entries.append((12 + len(body) if data else 0, len(data), name))
        body += data
    diroff = 12 + len(body)

    with open(args.output, "wb") as f:
        f.write(struct.pack("<4sii", ident, len(out), diroff))
        f.write(body)
        for pos, size, name in entries:
            f.write(struct.pack("<ii8s", pos, size, name.encode("ascii")))

    before = sum(len(d) for _, d in lumps)
    after = len(body)
    print(f"{args.input}: {before} bytes of lump data -> {after} ({100 * after // max(before, 1)}%)")


if __name__ == "__main__":
    main()
