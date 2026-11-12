#include <stddef.h>

#ifndef MEMORY_H
#define MEMORY_H

void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dest, const void *src, size_t num);
void *memmove(void *dest, const void *src, size_t num);

#endif
