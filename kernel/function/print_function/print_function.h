#ifndef PRINT_FUNCTION_H
#define PRINT_FUNCTION_H

extern void outb (unsigned short port, unsigned char value);
void print_outb(const char *text, ...);
void NO_print(const char *text, ...);
void print(const char* text, ...);

#endif
