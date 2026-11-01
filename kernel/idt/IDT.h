#include <stdint.h>
#ifndef IDT_H
#define IDT_H


void pic_remap();
void set_idt_gate(int n, uint32_t handler_address);
void init_idt();
struct IDT_entry {
    uint16_t offset_lowerbits; // Les 16 bits de poids faible de l'adresse de la fonction
    uint16_t selector;         // Le sélecteur de segment de code (généralement 0x08 pour le noyau)
    uint8_t  zero;             // Toujours à 0
    uint8_t  type_attr;        // Attributs de droits et de type (0x8E pour une interruption système)
    uint16_t offset_higherbits;// Les 16 bits de poids fort de l'adresse de la fonction
} __attribute__((packed));     // Empêche GCC d'ajouter du "padding" (alignement auto)
// Le registre IDTR qui dit au CPU où se trouve la table
struct IDTR {
    uint16_t limit;            // Taille de la table - 1
    uint32_t base;             // Adresse mémoire du début de notre tableau idt
} __attribute__((packed));
// Notre tableau de 256 interruptions
extern struct IDTR idtr;
extern struct IDT_entry idt[256];

#endif
