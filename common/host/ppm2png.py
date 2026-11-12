#!/usr/bin/env python3
"""ppm2png.py -- convert the host harness's PPM screenshots to PNG.
Usage: ppm2png.py file.ppm [more.ppm ...]   (writes file.png next to each)"""
import struct
import sys
import zlib


def convert(src):
    d = open(src, "rb").read()
    _, dims, _, px = d.split(b"\n", 3)
    w, h = map(int, dims.split())
    raw = b"".join(b"\x00" + px[y * w * 3:(y + 1) * w * 3] for y in range(h))

    def chunk(tag, data):
        body = tag + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)

    out = src.rsplit(".", 1)[0] + ".png"
    with open(out, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
                + chunk(b"IDAT", zlib.compress(raw)) + chunk(b"IEND", b""))
    return out


if __name__ == "__main__":
    for p in sys.argv[1:]:
        print(convert(p))
