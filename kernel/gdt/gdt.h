#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define GDT_KERNEL_CODE_SEGMENT 0x08
#define GDT_KERNEL_DATA_SEGMENT 0x10
#define GDT_USER_DATA_SEGMENT   0x18
#define GDT_USER_CODE_SEGMENT   0x20

// 64-bit GDTR pointer structure
struct GDTR {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

// Initialize and load 64-bit GDT into CPU
void gdt_init(void);

#endif // GDT_H
