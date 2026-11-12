#include <stdarg.h>
#include "./print_function.h"
#include "../function.h"
#include "../../data/global_value.h"
#include "../../serial/serial.h"

void print_outb(const char *text, ...)
{
	va_list args;
	va_start(args, text);
	serial_vprintf(text, args);
	va_end(args);
}

void NO_print(const char *text, ...)
{
	(void)text;
	return;
}

void video_putchar(char c)
{
    if (var.screen.cursor_position >= 4000)
        return;

    var.screen.VB[var.screen.cursor_position] = c;
    var.screen.VB[var.screen.cursor_position + 1] = 0xFFFFFF;

    var.screen.cursor_position += 2;
}

void video_print(const char *text)
{
    for (int i = 0; text[i] != '\0'; i++)
        video_putchar(text[i]);
}

void print_int(int value)
{
    char buffer[12];
    int i = 0;

    if (value == 0)
    {
        video_putchar('0');
        return;
    }

    if (value < 0)
    {
        video_putchar('-');
        value = -value;
    }

    while (value > 0)
    {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0)
        video_putchar(buffer[--i]);
}

void print_hex(unsigned int value)
{
    const char hex[] = "0123456789ABCDEF";
    char buffer[8];
    int i = 0;

    if (value == 0)
    {
        video_putchar('0');
        return;
    }

    while (value > 0)
    {
        buffer[i++] = hex[value & 0xF];
        value >>= 4;
    }

    while (i > 0)
        video_putchar(buffer[--i]);
}

void print(const char *text, ...)
{
	va_list args;
	va_start(args, text);

	for (int j = 0; text[j] != '\0'; j++)
	{
		if (var.screen.cursor_position >= var.screen.VBSize)
		{
			var.screen.cursor_position = 0;
			break;
		}

		if (text[j] == '\n')
		{
			var.screen.cursor_position = ((var.screen.cursor_position / var.screen.VBWidth) + 1) * var.screen.VBWidth;
        }

        else if (text[j] == '%' && text[j + 1] != '\0')
        {
            j++;

            switch (text[j])
            {
                case 'c':
                {
                    int value = va_arg(args, int);
                    char c = (char)value;

                    my.outb(0x3F8, c);
                    video_putchar(c);
                    break;
                }

                case 'd':
                {
                    int value = va_arg(args, int);

                    print_int(value);
                    break;
                }

                case 's':
                {
                    char *string = va_arg(args, char *);

                    my.print.outb(string);
                    video_print(string);
                    break;
                }

                case 'h':
                {
                    unsigned int value = va_arg(args, unsigned int);

                    my.outb(0x3F8, '0');
                    my.outb(0x3F8, 'x');

                    video_putchar('0');
                    video_putchar('x');

                    print_hex(value);
                    break;
                }

                case '%':
                {
                    my.outb(0x3F8, '%');
                    video_putchar('%');
                    break;
                }

                default:
                {
                    my.outb(0x3F8, '%');
                    my.outb(0x3F8, text[j]);

                    video_putchar('%');
                    video_putchar(text[j]);
                    break;
                }
            }
        }
        else
        {
            my.outb(0x3F8, text[j]);
            video_putchar(text[j]);
        }

        if (var.screen.cursor_position >= var.screen.VBSize)
        {
			my.function.scroll();
        }
    }

    va_end(args);
}
