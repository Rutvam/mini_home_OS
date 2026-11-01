#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

extern volatile uint8_t last_scancode;
extern const unsigned char qwertz_german[3][128];

void keyboard_handler_c();
extern void keyboard_handler_asm();

#endif
