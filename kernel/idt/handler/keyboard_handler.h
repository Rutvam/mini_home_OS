#include <stdint.h>
#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H

extern volatile uint8_t last_scancode;
extern const unsigned char qwertz_german[3][128];
void keyboard_init(void);
void keyboard_handler_c(void);
extern void keyboard_handler_asm(void);

#endif
