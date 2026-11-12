#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define COM1_PORT 0x3F8

// Initialize UART 16550 serial port COM1 (115200 baud, 8N1, FIFOs enabled)
void serial_init(void);

// Write a single character to COM1
void serial_putchar(char c);

// Write a null-terminated string to COM1
void serial_print(const char *str);

// Formatted print to COM1 (%s, %c, %d, %i, %u, %x, %X, %lx, %p, %0Nx)
void serial_printf(const char *fmt, ...);

// Formatted print using va_list to COM1
void serial_vprintf(const char *fmt, va_list args);

#endif // SERIAL_H
