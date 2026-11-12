#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "./print_function.h"
#include "../function.h"
#include "../../data/global_value.h"
#include "../../data/graphics_value/police.h"
#include "../memory/memory.h"

#define COLOR_WHITE 0x00FFFFFF
#define COLOR_BLACK 0x00000000

/* Cursor coordinates */
int cursor_x = 0;
int cursor_y = 0;

static inline void serial_putc(char c)
{
    if (my.outb)
        my.outb(0x3F8, (unsigned char)c);
}

static inline uint32_t *get_fb(void)
{
    if (var.screen.FB)
        return var.screen.FB;
    if (var.params && var.params->GPU_Configs.FrameBuffer)
    {
        var.screen.FB = (uint32_t *)(uintptr_t)var.params->GPU_Configs.FrameBuffer;
        return var.screen.FB;
    }
    return NULL;
}

static inline int get_screen_width(void)
{
    if (var.screen.FBWidth > 0)
        return var.screen.FBWidth;
    if (var.params && var.params->GPU_Configs.width > 0)
        return (int)var.params->GPU_Configs.width;
    return 800;
}

static inline int get_screen_height(void)
{
    if (var.screen.FBHeight > 0)
        return var.screen.FBHeight;
    if (var.params && var.params->GPU_Configs.height > 0)
        return (int)var.params->GPU_Configs.height;
    return 600;
}

static inline int get_screen_pitch(void)
{
    if (var.screen.FBPitch > 0)
        return var.screen.FBPitch;
    if (var.params && var.params->GPU_Configs.pitch > 0)
        return (int)var.params->GPU_Configs.pitch;
    return get_screen_width();
}

void print_outb(const char *text, ...)
{
    va_list args;
    va_start(args, text);

    for (int i = 0; text[i] != 0x00; i++)
    {
        if (text[i] == '%' && text[i + 1] == 'c')
        {
            char caracter = (char)va_arg(args, int);
            serial_putc(caracter);
            i++;
        }
        else if (text[i] == '%' && text[i + 1] == 'd')
        {
            int value = va_arg(args, int);
            char buffer[12];
            int bi = 0;

            if (value == 0)
            {
                serial_putc('0');
            }
            else
            {
                unsigned int uval;
                if (value < 0)
                {
                    serial_putc('-');
                    uval = (unsigned int)(-(long long)value);
                }
                else
                {
                    uval = (unsigned int)value;
                }

                while (uval > 0)
                {
                    buffer[bi++] = '0' + (uval % 10);
                    uval /= 10;
                }

                while (bi > 0)
                {
                    serial_putc(buffer[--bi]);
                }
            }
            i++;
        }
        else if (text[i] == '%' && text[i + 1] == 's')
        {
            char *string = va_arg(args, char *);
            if (string)
                my.print.outb(string);
            i++;
        }
        else if (text[i] == '%' && text[i + 1] == 'h')
        {
            int value = va_arg(args, int);
            char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
            char caracter = hex[value & 0xF];
            serial_putc(caracter);
            i++;
        }
        else
        {
            serial_putc(text[i]);
        }
    }

    va_end(args);
}

void NO_print(const char *text, ...)
{
    (void)text;
}

/* Fast memory movement for scrolling */
static void *fast_memmove(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    if (d == s || n == 0)
        return dest;

    if (d < s)
    {
        while (n >= sizeof(uint64_t) && ((uintptr_t)d % sizeof(uint64_t) == 0) && ((uintptr_t)s % sizeof(uint64_t) == 0))
        {
            *(uint64_t *)d = *(const uint64_t *)s;
            d += sizeof(uint64_t);
            s += sizeof(uint64_t);
            n -= sizeof(uint64_t);
        }
        while (n--)
            *d++ = *s++;
    }
    else
    {
        d += n;
        s += n;
        while (n >= sizeof(uint64_t) && ((uintptr_t)d % sizeof(uint64_t) == 0) && ((uintptr_t)s % sizeof(uint64_t) == 0))
        {
            d -= sizeof(uint64_t);
            s -= sizeof(uint64_t);
            *(uint64_t *)d = *(const uint64_t *)s;
            n -= sizeof(uint64_t);
        }
        while (n--)
            *--d = *--s;
    }

    return dest;
}

/* Clear the screen and reset cursor position */
void clear_screen(void)
{
    uint32_t *fb = get_fb();
    int screen_height = get_screen_height();
    int pitch = get_screen_pitch();

    if (fb && screen_height > 0 && pitch > 0)
    {
        size_t total_bytes = (size_t)screen_height * (size_t)pitch * sizeof(uint32_t);
        memset(fb, 0, total_bytes);
    }
    else if (fb && var.screen.FBSize > 0)
    {
        memset(fb, 0, (size_t)var.screen.FBSize);
    }

    cursor_x = 0;
    cursor_y = 0;
    var.screen.cursor_position = 0;
}

void video_clear(void)
{
    clear_screen();
}

/* Vertical Scrolling */
void scroll(void)
{
    uint32_t *fb = get_fb();
    int screen_height = get_screen_height();
    int pitch = get_screen_pitch();

    if (!fb || screen_height <= FONT_HEIGHT || pitch <= 0)
    {
        cursor_y = (screen_height > FONT_HEIGHT) ? (screen_height - FONT_HEIGHT) : 0;
        return;
    }

    size_t rows_to_copy = (size_t)(screen_height - FONT_HEIGHT);
    size_t copy_bytes = rows_to_copy * (size_t)pitch * sizeof(uint32_t);

    /* a) Shift framebuffer rows up by copying (screen_height - FONT_HEIGHT) rows up */
    fast_memmove(fb, fb + ((size_t)FONT_HEIGHT * pitch), copy_bytes);

    /* b) Clear the newly freed bottom text row with the background color (black) */
    size_t bottom_bytes = (size_t)FONT_HEIGHT * (size_t)pitch * sizeof(uint32_t);
    uint32_t *bottom_row = fb + (rows_to_copy * pitch);
    memset(bottom_row, 0, bottom_bytes);

    /* c) Set cursor_y = screen_height - FONT_HEIGHT */
    cursor_y = screen_height - FONT_HEIGHT;
}

/* Render character glyph to framebuffer */
static void draw_char(char c, int x, int y, uint32_t fg_color, uint32_t bg_color)
{
    uint32_t *fb = get_fb();
    if (!fb)
        return;

    int screen_width = get_screen_width();
    int screen_height = get_screen_height();
    int pitch = get_screen_pitch();

    unsigned char uc = (unsigned char)c;
    int has_glyph = (uc >= 1 && uc <= 126);
    int font_idx = has_glyph ? (uc - 1) : 0;

    for (int dy = 0; dy < FONT_HEIGHT; dy++)
    {
        int py = y + dy;
        if (py < 0 || py >= screen_height)
            continue;

        size_t row_offset = (size_t)py * (size_t)pitch;

        for (int dx = 0; dx < FONT_WIDTH; dx++)
        {
            int px = x + dx;
            if (px < 0 || px >= screen_width)
                continue;

            uint32_t color = bg_color;
            if (has_glyph && police[font_idx][dy][dx])
                color = fg_color;

            fb[row_offset + px] = color;
        }
    }
}

void video_putchar(char c)
{
    int screen_width = get_screen_width();
    int screen_height = get_screen_height();

    /* Handle '\r': reset cursor_x = 0 */
    if (c == '\r')
    {
        cursor_x = 0;
        return;
    }

    /* Handle '\n': reset cursor_x = 0 and increment cursor_y += FONT_HEIGHT */
    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y += FONT_HEIGHT;
        if ((cursor_y + FONT_HEIGHT) > screen_height)
        {
            scroll();
        }
        return;
    }

    /* Handle '\t': advance to next 4-space tab stop */
    if (c == '\t')
    {
        int tab_stop = FONT_WIDTH * 4;
        cursor_x = ((cursor_x / tab_stop) + 1) * tab_stop;
        if ((cursor_x + FONT_WIDTH) > screen_width)
        {
            cursor_x = 0;
            cursor_y += FONT_HEIGHT;
            if ((cursor_y + FONT_HEIGHT) > screen_height)
            {
                scroll();
            }
        }
        return;
    }

    /* Handle '\b': backspace */
    if (c == '\b')
    {
        if (cursor_x >= FONT_WIDTH)
        {
            cursor_x -= FONT_WIDTH;
            draw_char(' ', cursor_x, cursor_y, COLOR_WHITE, COLOR_BLACK);
        }
        return;
    }

    /* Automatic wrap: if (cursor_x + FONT_WIDTH) > screen_width */
    if ((cursor_x + FONT_WIDTH) > screen_width)
    {
        cursor_x = 0;
        cursor_y += FONT_HEIGHT;
        if ((cursor_y + FONT_HEIGHT) > screen_height)
        {
            scroll();
        }
    }

    /* Vertical scrolling if needed */
    if ((cursor_y + FONT_HEIGHT) > screen_height)
    {
        scroll();
    }

    /* Render character */
    draw_char(c, cursor_x, cursor_y, COLOR_WHITE, COLOR_BLACK);

    /* Advance cursor_x += FONT_WIDTH */
    cursor_x += FONT_WIDTH;
}

void video_print(const char *text)
{
    if (!text)
        return;
    for (int i = 0; text[i] != '\0'; i++)
        video_putchar(text[i]);
}

void print_int(int value)
{
    char buffer[12];
    int i = 0;

    if (value == 0)
    {
        serial_putc('0');
        video_putchar('0');
        return;
    }

    unsigned int uval;
    if (value < 0)
    {
        serial_putc('-');
        video_putchar('-');
        uval = (unsigned int)(-(long long)value);
    }
    else
    {
        uval = (unsigned int)value;
    }

    while (uval > 0)
    {
        buffer[i++] = '0' + (uval % 10);
        uval /= 10;
    }

    while (i > 0)
    {
        char c = buffer[--i];
        serial_putc(c);
        video_putchar(c);
    }
}

void print_hex(unsigned int value)
{
    const char hex[] = "0123456789ABCDEF";
    char buffer[16];
    int i = 0;

    if (value == 0)
    {
        serial_putc('0');
        video_putchar('0');
        return;
    }

    while (value > 0)
    {
        buffer[i++] = hex[value & 0xF];
        value >>= 4;
    }

    while (i > 0)
    {
        char c = buffer[--i];
        serial_putc(c);
        video_putchar(c);
    }
}

void print(const char *text, ...)
{
    va_list args;
    va_start(args, text);

    for (int j = 0; text[j] != '\0'; j++)
    {
        if (text[j] == '\n')
        {
            serial_putc('\n');
            video_putchar('\n');
        }
        else if (text[j] == '\r')
        {
            serial_putc('\r');
            video_putchar('\r');
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
                    serial_putc(c);
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
                    if (string)
                    {
                        my.print.outb(string);
                        video_print(string);
                    }
                    break;
                }

                case 'h':
                case 'x':
                {
                    unsigned int value = va_arg(args, unsigned int);
                    serial_putc('0');
                    serial_putc('x');
                    video_putchar('0');
                    video_putchar('x');
                    print_hex(value);
                    break;
                }

                case '%':
                {
                    serial_putc('%');
                    video_putchar('%');
                    break;
                }

                default:
                {
                    serial_putc('%');
                    serial_putc(text[j]);
                    video_putchar('%');
                    video_putchar(text[j]);
                    break;
                }
            }
        }
        else
        {
            serial_putc(text[j]);
            video_putchar(text[j]);
        }
    }

    va_end(args);
}
