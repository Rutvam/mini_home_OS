#include <stdint.h>
#ifndef PRINT_FUNCTION_H
#define PRINT_FUNCTION_H

// extern int cursor_x;
// extern int cursor_y;

void print_outb(const char *text, ...);
void NO_print(const char *text, ...);
void video_putchar(char c);
void video_print(const char *text);
void print_int(int value);
void print_hex(unsigned int value);
void scroll(void);
void clear_screen(void);
void video_clear(void);
void print(const char* text, ...);

#endif
