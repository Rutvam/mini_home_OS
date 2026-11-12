#include "serial.h"

extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);

void serial_init(void)
{
    // 1. Disable all UART interrupts
    outb(COM1_PORT + 1, 0x00);

    // 2. Enable DLAB (Divisor Latch Access Bit) to configure baud rate
    outb(COM1_PORT + 3, 0x80);

    // 3. Set divisor to 1 (115200 baud: 115200 / 115200 = 1)
    outb(COM1_PORT + 0, 0x01); // Divisor low byte
    outb(COM1_PORT + 1, 0x00); // Divisor high byte

    // 4. Configure line control: 8 bits, no parity, 1 stop bit (8N1), clear DLAB
    outb(COM1_PORT + 3, 0x03);

    // 5. Enable FIFO, clear TX/RX queues, set 14-byte threshold
    outb(COM1_PORT + 2, 0xC7);

    // 6. Set modem control: Auxiliary output 2 (OUT2), RTS, DTR
    outb(COM1_PORT + 4, 0x0B);
}

static int serial_is_transmit_empty(void)
{
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_putchar(char c)
{
    // Wait until Transmit Holding Register is empty
    while (!serial_is_transmit_empty()) {
        __asm__ volatile ("pause");
    }
    outb(COM1_PORT, (uint8_t)c);
}

void serial_print(const char *str)
{
    if (!str) {
        return;
    }
    while (*str) {
        if (*str == '\n') {
            serial_putchar('\r');
        }
        serial_putchar(*str++);
    }
}

static void serial_print_unsigned(uint64_t val, int base, int uppercase, int pad_width, char pad_char)
{
    char buf[65];
    int pos = 0;
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

    if (val == 0) {
        buf[pos++] = '0';
    } else {
        while (val > 0 && pos < 64) {
            buf[pos++] = digits[val % base];
            val /= base;
        }
    }

    while (pos < pad_width && pos < 64) {
        buf[pos++] = pad_char;
    }

    while (pos > 0) {
        serial_putchar(buf[--pos]);
    }
}

static void serial_print_signed(int64_t val)
{
    if (val < 0) {
        serial_putchar('-');
        val = -val;
    }
    serial_print_unsigned((uint64_t)val, 10, 0, 0, ' ');
}

void serial_vprintf(const char *fmt, va_list args)
{
    if (!fmt) {
        return;
    }

    for (int i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%') {
            if (fmt[i] == '\n') {
                serial_putchar('\r');
            }
            serial_putchar(fmt[i]);
            continue;
        }

        i++; // skip '%'
        if (fmt[i] == '\0') {
            break;
        }

        // Parse optional padding (e.g. %04x, %016lx)
        char pad_char = ' ';
        int pad_width = 0;
        if (fmt[i] == '0') {
            pad_char = '0';
            i++;
        }
        while (fmt[i] >= '0' && fmt[i] <= '9') {
            pad_width = pad_width * 10 + (fmt[i] - '0');
            i++;
        }

        // Check for 'l' length modifier
        int is_long = 0;
        if (fmt[i] == 'l') {
            is_long = 1;
            i++;
        }

        switch (fmt[i]) {
            case 's': {
                const char *s = va_arg(args, const char *);
                serial_print(s ? s : "(null)");
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                serial_putchar(c);
                break;
            }
            case 'd':
            case 'i': {
                if (is_long) {
                    int64_t v = va_arg(args, int64_t);
                    serial_print_signed(v);
                } else {
                    int32_t v = va_arg(args, int32_t);
                    serial_print_signed(v);
                }
                break;
            }
            case 'u': {
                if (is_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    serial_print_unsigned(v, 10, 0, pad_width, pad_char);
                } else {
                    uint32_t v = va_arg(args, uint32_t);
                    serial_print_unsigned(v, 10, 0, pad_width, pad_char);
                }
                break;
            }
            case 'x': {
                if (is_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    serial_print_unsigned(v, 16, 0, pad_width, pad_char);
                } else {
                    uint32_t v = va_arg(args, uint32_t);
                    serial_print_unsigned(v, 16, 0, pad_width, pad_char);
                }
                break;
            }
            case 'X': {
                if (is_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    serial_print_unsigned(v, 16, 1, pad_width, pad_char);
                } else {
                    uint32_t v = va_arg(args, uint32_t);
                    serial_print_unsigned(v, 16, 1, pad_width, pad_char);
                }
                break;
            }
            case 'p': {
                uint64_t v = (uint64_t)(uintptr_t)va_arg(args, void *);
                serial_print("0x");
                serial_print_unsigned(v, 16, 0, 16, '0');
                break;
            }
            case '%': {
                serial_putchar('%');
                break;
            }
            default: {
                serial_putchar('%');
                serial_putchar(fmt[i]);
                break;
            }
        }
    }
}

void serial_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    serial_vprintf(fmt, args);
    va_end(args);
}
