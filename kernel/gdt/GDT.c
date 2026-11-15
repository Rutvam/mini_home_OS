#include "./GDT.h"

// 3 entrées GDT pour un fonctionnement exclusif en Ring 0 :
// 0: Null descriptor (0x00)
// 1: 64-bit Kernel Code Segment (0x08)
// 2: 64-bit Kernel Data Segment (0x10)
static uint64_t gdt_entries[3];
static struct GDTR gdtr;

extern void gdt_load(struct GDTR *gdtr);

void gdt_init(void)
{
	// 0x00: Null Descriptor
	gdt_entries[0] = 0x0000000000000000ULL;

	// 0x08: 64-bit Kernel Code Segment (Ring 0)
	gdt_entries[1] = 0x00AF9A000000FFFFULL;

	// 0x10: 64-bit Kernel Data Segment (Ring 0)
	gdt_entries[2] = 0x00CF92000000FFFFULL;

	// Configuration du registre GDTR (taille - 1)
	gdtr.limit = (uint16_t)(sizeof(gdt_entries) - 1);
	gdtr.base = (uint64_t)(uintptr_t)&gdt_entries[0];

	// Chargement de la GDT par la fonction assembleur externe
	gdt_load(&gdtr);
}
