#include <stddef.h>
#include <stdint.h>
#include "memory.h"

void *memset(void *ptr, int value, size_t num)
{
    unsigned char *p = ptr;
    unsigned char byte = (unsigned char)value;

    uint64_t val64 = (uint64_t)byte;
    val64 |= val64 << 8;
    val64 |= val64 << 16;
    val64 |= val64 << 32;

    while (num >= sizeof(uint64_t) && ((uintptr_t)p % sizeof(uint64_t) == 0))
    {
        *(uint64_t *)p = val64;
        p += sizeof(uint64_t);
        num -= sizeof(uint64_t);
    }

    while (num--)
        *p++ = byte;

    return ptr;
}

void *memcpy(void *dest, const void *src, size_t num)
{
    unsigned char *d = dest;
    const unsigned char *s = src;

    while (num >= sizeof(uint64_t) && ((uintptr_t)d % sizeof(uint64_t) == 0) && ((uintptr_t)s % sizeof(uint64_t) == 0))
    {
        *(uint64_t *)d = *(const uint64_t *)s;
        d += sizeof(uint64_t);
        s += sizeof(uint64_t);
        num -= sizeof(uint64_t);
    }

    while (num--)
        *d++ = *s++;

    return dest;
}

void *memmove(void *dest, const void *src, size_t num)
{
    unsigned char *d = dest;
    const unsigned char *s = src;

    if (d == s || num == 0)
        return dest;

    if (d < s)
    {
        while (num >= sizeof(uint64_t) && ((uintptr_t)d % sizeof(uint64_t) == 0) && ((uintptr_t)s % sizeof(uint64_t) == 0))
        {
            *(uint64_t *)d = *(const uint64_t *)s;
            d += sizeof(uint64_t);
            s += sizeof(uint64_t);
            num -= sizeof(uint64_t);
        }
        while (num--)
            *d++ = *s++;
    }
    else
    {
        d += num;
        s += num;
        while (num >= sizeof(uint64_t) && ((uintptr_t)d % sizeof(uint64_t) == 0) && ((uintptr_t)s % sizeof(uint64_t) == 0))
        {
            d -= sizeof(uint64_t);
            s -= sizeof(uint64_t);
            *(uint64_t *)d = *(const uint64_t *)s;
            num -= sizeof(uint64_t);
        }
        while (num--)
            *--d = *--s;
    }

    return dest;
}
