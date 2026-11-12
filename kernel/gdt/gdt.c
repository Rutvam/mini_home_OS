#include "gdt.h"

// 5 GDT entries:
// 0: Null descriptor
// 1: 64-bit Kernel Code (0x08)
// 2: 64-bit Kernel Data (0x10)
// 3: 64-bit User Data   (0x18)
// 4: 64-bit User Code   (0x20)
static uint64_t gdt_entries[5];
static struct GDTR gdtr;

extern void gdt_load(struct GDTR *gdtr);

void gdt_init(void)
{
    // 0x00: Null Descriptor
    gdt_entries[0] = 0x0000000000000000ULL;

    // 0x08: 64-bit Kernel Code Segment
    // Base = 0, Limit = 0xFFFFF, Access = 0x9A, Flags = 0xA (L=1, D=0, G=1)
    gdt_entries[1] = 0x00AF9A000000FFFFULL;

    // 0x10: 64-bit Kernel Data Segment
    // Base = 0, Limit = 0xFFFFF, Access = 0x92, Flags = 0xC (D=1, G=1)
    gdt_entries[2] = 0x00CF92000000FFFFULL;

    // 0x18: 64-bit User Data Segment (Ring 3)
    // Base = 0, Limit = 0xFFFFF, Access = 0xF2, Flags = 0xC (D=1, G=1)
    gdt_entries[3] = 0x00CFF2000000FFFFULL;

    // 0x20: 64-bit User Code Segment (Ring 3)
    // Base = 0, Limit = 0xFFFFF, Access = 0xFA, Flags = 0xA (L=1, D=0, G=1)
    gdt_entries[4] = 0x00AFFA000000FFFFULL;

    gdtr.limit = (uint16_t)(sizeof(gdt_entries) - 1);
    gdtr.base = (uint64_t)(uintptr_t)&gdt_entries[0];

    // Load GDTR and reload CS (0x08) and DS/ES/FS/GS/SS (0x10)
    gdt_load(&gdtr);
}
