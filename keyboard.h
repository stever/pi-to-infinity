#ifndef MX_KEYBOARD_H
#define MX_KEYBOARD_H

#include <stdbool.h> // bool

// moveKeys bits.
#define MOVE_FORWARD 			0x01
#define MOVE_BACK 				0x02
#define MOVE_FORWARD_OVER_BACK	0x04
#define MOVE_LEFT 				0x08
#define MOVE_RIGHT 				0x10
#define MOVE_LEFT_OVER_RIGHT	0x20
#define MOVE_UP					0x40
#define MOVE_DOWN				0x80

// specialKeys bits.
#define KEY_EXIT				0x01
#define KEY_RESET_POS			0x02

bool mxKeyboardSetup();
void mxKeyboardUpdate(unsigned char *moveKeys, unsigned char *specialKeys);
void mxKeyboardCleanup();

#endif /* MX_KEYBOARD_H */