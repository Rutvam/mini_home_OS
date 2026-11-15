#include <stdint.h>
#ifndef GDT_H
#define GDT_H

#define GDT_KERNEL_CODE_SEGMENT 0x08
#define GDT_KERNEL_DATA_SEGMENT 0x10

// Structure du pointeur GDTR
struct GDTR {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

// Initialisation et chargement de la GDT 64 bits
void gdt_init(void);

#endif
