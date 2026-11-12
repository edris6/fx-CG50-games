#!/usr/bin/env python3
"""make_icon.py -- generate the two 92x64 add-in icons (unselected/selected)
for a game, using a built-in 5x7 pixel font. No third-party libraries.

    python3 common/tools/make_icon.py "DOOM" games/doom/assets-cg --color c01010
"""
import argparse
import os
import struct
import zlib

FONT = {
    "A": ["01110", "10001", "10001", "11111", "10001", "10001", "10001"],
    "B": ["11110", "10001", "10001", "11110", "10001", "10001", "11110"],
    "C": ["01110", "10001", "10000", "10000", "10000", "10001", "01110"],
    "D": ["11110", "10001", "10001", "10001", "10001", "10001", "11110"],
    "E": ["11111", "10000", "10000", "11110", "10000", "10000", "11111"],
    "F": ["11111", "10000", "10000", "11110", "10000", "10000", "10000"],
    "I": ["01110", "00100", "00100", "00100", "00100", "00100", "01110"],
    "J": ["00111", "00010", "00010", "00010", "00010", "10010", "01100"],
    "K": ["10001", "10010", "10100", "11000", "10100", "10010", "10001"],
    "L": ["10000", "10000", "10000", "10000", "10000", "10000", "11111"],
    "M": ["10001", "11011", "10101", "10101", "10001", "10001", "10001"],
    "N": ["10001", "11001", "10101", "10011", "10001", "10001", "10001"],
    "O": ["01110", "10001", "10001", "10001", "10001", "10001", "01110"],
    "P": ["11110", "10001", "10001", "11110", "10000", "10000", "10000"],
    "R": ["11110", "10001", "10001", "11110", "10100", "10010", "10001"],
    "S": ["01111", "10000", "10000", "01110", "00001", "00001", "11110"],
    "T": ["11111", "00100", "00100", "00100", "00100", "00100", "00100"],
    "U": ["10001", "10001", "10001", "10001", "10001", "10001", "01110"],
    "Y": ["10001", "10001", "01010", "00100", "00100", "00100", "00100"],
    "0": ["01110", "10001", "10011", "10101", "11001", "10001", "01110"],
    "2": ["01110", "10001", "00001", "00010", "00100", "01000", "11111"],
    "4": ["00010", "00110", "01010", "10010", "11111", "00010", "00010"],
    "8": ["01110", "10001", "10001", "01110", "10001", "10001", "01110"],
    " ": ["00000"] * 7,
}


def png(path, w, h, pixels):
    raw = b"".join(b"\x00" + bytes(c for px in pixels[y * w:(y + 1) * w] for c in px)
                   for y in range(h))

    def chunk(tag, data):
        body = tag + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)))
        f.write(chunk(b"IDAT", zlib.compress(raw, 9)))
        f.write(chunk(b"IEND", b""))


def render(text, fg, bg):
    w, h = 92, 64
    px = [bg] * (w * h)
    scale = 1
    while (len(text) * 6 * (scale + 1) - (scale + 1)) <= w - 8 and 7 * (scale + 1) <= h - 8:
        scale += 1
    tw = len(text) * 6 * scale - scale
    x0, y0 = (w - tw) // 2, (h - 7 * scale) // 2
    for i, ch in enumerate(text):
        glyph = FONT[ch]
        for gy, row in enumerate(glyph):
            for gx, bit in enumerate(row):
                if bit == "1":
                    for sy in range(scale):
                        for sx in range(scale):
                            x = x0 + i * 6 * scale + gx * scale + sx
                            y = y0 + gy * scale + sy
                            px[y * w + x] = fg
    # 2px border
    for x in range(w):
        for t in (0, 1, h - 1, h - 2):
            px[t * w + x] = fg
    for y in range(h):
        for t in (0, 1, w - 1, w - 2):
            px[y * w + t] = fg
    return w, h, px


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("text")
    ap.add_argument("outdir")
    ap.add_argument("--color", default="c01010", help="accent color, hex RRGGBB")
    a = ap.parse_args()
    accent = tuple(int(a.color[i:i + 2], 16) for i in (0, 2, 4))
    dark, light = (24, 24, 24), (240, 240, 240)
    os.makedirs(a.outdir, exist_ok=True)
    w, h, px = render(a.text.upper(), accent, dark)
    png(os.path.join(a.outdir, "icon-uns.png"), w, h, px)
    w, h, px = render(a.text.upper(), dark, accent if False else light)
    png(os.path.join(a.outdir, "icon-sel.png"), w, h, px)


if __name__ == "__main__":
    main()
