#include <stdint.h>
#include "IDT.h"

struct IDTR idtr;
struct IDT_entry idt[256];

void pic_remap() {
    // Initialisation du PIC Maître (port 0x20) et Esclave (port 0xA0)
    __asm__ __volatile__("outb %%al, %%dx" : : "a"(0x11), "d"(0x20));
    __asm__ __volatile__("outb %%al, %%dx" : : "a"(0x11), "d"(0xA0));

    // On dit au PIC Maître de mapper ses IRQ (0 à 7) à partir de 0x20 (32 en décimal)
    __asm__ __volatile__("outb %%al, %%dx" : : "a"(0x20), "d"(0x21));
    // On dit au PIC Esclave de mapper ses IRQ (8 à 15) à partir de 0x28 (40 en décimal)
    __asm__ __volatile__("outb %%al, %%dx" : : "a"(0x28), "d"(0xA1));

    // Configuration des liaisons physiques entre Maître et Esclave
    __asm__ __volatile__("outb %%al, %%dx" : : "a"(0x04), "d"(0x21));
    __asm__ __volatile__("outb %%al, %%dx" : : "a"(0x02), "d"(0xA1));

    // Mode 8086 basique
    __asm__ __volatile__("outb %%al, %%dx" : : "a"(0x01), "d"(0x21));
    __asm__ __volatile__("outb %%al, %%dx" : : "a"(0x01), "d"(0xA1));

    // Masquage : 0x00 pour tout activer (ou 0xFD pour n'activer que le clavier IRQ 1)
    __asm__ __volatile__("outb %%al, %%dx" : : "a"(0xFD), "d"(0x21));
    __asm__ __volatile__("outb %%al, %%dx" : : "a"(0xFF), "d"(0xA1));
}

// 1. On instancie officiellement nos variables globales ici (sans struct complète)
// struct IDT_entry idt[256];
// struct IDTR idtr;

// 2. Ta fonction pour configurer une case de l'IDT
void set_idt_gate(int n, uint32_t handler_address) {
    idt[n].offset_lowerbits = handler_address & 0xFFFF;
    idt[n].selector = 0x08; // Segment de code de ta GDT
    idt[n].zero = 0;
    idt[n].type_attr = 0x8E; // Présent, Anneau 0, 32-bit Interrupt Gate
    idt[n].offset_higherbits = (handler_address >> 16) & 0xFFFF;
}

// 3. Ta fonction pour charger l'IDT dans le CPU
void init_idt() {
    idtr.limit = (sizeof(struct IDT_entry) * 256) - 1;
    idtr.base = (uint32_t)(uintptr_t)&idt;

    // L'instruction assembleur magique
    __asm__ __volatile__("lidt (%0)" : : "r" (&idtr));
}

