#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_ENTRIES 256

// 64-bit Interrupt Gate Descriptor (16 bytes)
struct IDT_entry {
    uint16_t offset_lowerbits; // Bits 0..15 of handler address
    uint16_t selector;         // Code segment selector (0x08 for kernel code)
    uint8_t  ist;              // IST index (bits 0..2)
    uint8_t  type_attr;        // Type & attributes (0x8E for 64-bit Ring 0 interrupt gate)
    uint16_t offset_mid;       // Bits 16..31 of handler address
    uint32_t offset_high;      // Bits 32..63 of handler address
    uint32_t zero;             // Reserved (must be 0)
} __attribute__((packed));

// 64-bit IDTR pointer structure (10 bytes)
struct IDTR {
    uint16_t limit;            // Size of IDT - 1
    uint64_t base;             // 64-bit linear base address of IDT
} __attribute__((packed));

// Register frame passed to exception_handler
struct interrupt_frame {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t vector;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed));

extern struct IDTR idtr;
extern struct IDT_entry idt[IDT_ENTRIES];

void set_idt_gate(int n, uint64_t handler_address);
void init_idt(void);
void pic_remap(void);
void exception_handler(struct interrupt_frame *frame);

#endif // IDT_H
