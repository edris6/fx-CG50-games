// Host-side stub of <gint/keyboard.h>. Key names follow gint's.
#pragma once
enum {
  KEY_NONE = 0,
  KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
  KEY_SHIFT, KEY_OPTN, KEY_VARS, KEY_MENU,
  KEY_LEFT, KEY_UP, KEY_DOWN, KEY_RIGHT,
  KEY_ALPHA, KEY_SQUARE, KEY_POWER, KEY_EXIT,
  KEY_XOT, KEY_LOG, KEY_LN, KEY_SIN, KEY_COS, KEY_TAN,
  KEY_FRAC, KEY_FD, KEY_LEFTP, KEY_RIGHTP, KEY_COMMA, KEY_ARROW,
  KEY_7, KEY_8, KEY_9, KEY_DEL,
  KEY_4, KEY_5, KEY_6, KEY_MUL, KEY_DIV,
  KEY_1, KEY_2, KEY_3, KEY_ADD, KEY_SUB,
  KEY_0, KEY_DOT, KEY_EXP, KEY_NEG, KEY_EXE,
  KEY_ACON,
  KEY_COUNT_
};
void clearevents(void);
int keydown(int key);
int getkey(void);
