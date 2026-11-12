// Verifies that the DK_* constants in src/platform/cg50_platform.c match
// src/doom/doomkeys.h. Build/run: see tests/host/run_checks.sh
#include <stdio.h>
#include "doomkeys.h"
#define EXPECT(name, val) do { if ((name) != (val)) { printf("MISMATCH %s: doomkeys.h=%d platform=%d\n", #name, (int)(name), (int)(val)); bad++; } } while (0)
int main(void) {
    int bad = 0;
    EXPECT(KEY_RIGHTARROW, 0xae); EXPECT(KEY_LEFTARROW, 0xac);
    EXPECT(KEY_UPARROW, 0xad);    EXPECT(KEY_DOWNARROW, 0xaf);
    EXPECT(KEY_STRAFE_L, 0xa0);   EXPECT(KEY_STRAFE_R, 0xa1);
    EXPECT(KEY_USE, 0xa2);        EXPECT(KEY_FIRE, 0xa3);
    EXPECT(KEY_ESCAPE, 27);       EXPECT(KEY_ENTER, 13);
    EXPECT(KEY_TAB, 9);           EXPECT(KEY_BACKSPACE, 0x7f);
    EXPECT(KEY_PAUSE, 0xff);      EXPECT(KEY_EQUALS, 0x3d);
    EXPECT(KEY_MINUS, 0x2d);      EXPECT(KEY_RSHIFT, 0x80 + 0x36);
    EXPECT(KEY_RALT, 0x80 + 0x38);
    printf(bad ? "FAIL\n" : "doomkeys OK\n");
    return bad != 0;
}
