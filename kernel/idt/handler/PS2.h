#include <stdint.h>
#ifndef PS2_H
#define PS2_H

void ps2_send(uint8_t value);
uint8_t ps2_read(void);
void read_keyboard(void);

#endif
