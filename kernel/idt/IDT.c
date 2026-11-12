#include "IDT.h"
#include "../serial/serial.h"
#include "../data/global_value.h"

struct IDTR idtr;
struct IDT_entry idt[IDT_ENTRIES];

extern void idt_load(struct IDTR *idtr_ptr);
extern void outb(uint16_t port, uint8_t value);

// ISR declarations from kernel_entry.asm
typedef void (*isr_func_t)(void);

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

static const isr_func_t isr_table[32] = {
    isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7,
    isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15,
    isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
    isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
};

static const char *exception_messages[32] = {
    "#DE: Divide-by-Zero Error",
    "#DB: Debug Exception",
    "NMI: Non-Maskable Interrupt",
    "#BP: Breakpoint",
    "#OF: Overflow",
    "#BR: Bound Range Exceeded",
    "#UD: Invalid Opcode",
    "#NM: Device Not Available",
    "#DF: Double Fault",
    "Coprocessor Segment Overrun",
    "#TS: Invalid TSS",
    "#NP: Segment Not Present",
    "#SS: Stack-Segment Fault",
    "#GP: General Protection Fault",
    "#PF: Page Fault",
    "Reserved",
    "#MF: x87 FPU Floating-Point Error",
    "#AC: Alignment Check",
    "#MC: Machine Check",
    "#XM: SIMD Floating-Point Exception",
    "#VE: Virtualization Exception",
    "#CP: Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void set_idt_gate(int n, uint64_t handler_address)
{
    if (n < 0 || n >= IDT_ENTRIES) {
        return;
    }
    idt[n].offset_lowerbits = (uint16_t)(handler_address & 0xFFFF);
    idt[n].selector = 0x08; // GDT Kernel Code Segment (0x08)
    idt[n].ist = 0;
    idt[n].type_attr = (handler_address != 0) ? 0x8E : 0x00; // 0x8E: Present, Ring 0, 64-bit Interrupt Gate
    idt[n].offset_mid = (uint16_t)((handler_address >> 16) & 0xFFFF);
    idt[n].offset_high = (uint32_t)((handler_address >> 32) & 0xFFFFFFFF);
    idt[n].zero = 0;
}

void pic_remap(void)
{
    // ICW1: Start PIC initialization in cascade mode
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // ICW2: Vector offset mapping: Master -> 0x20 (32..37), Slave -> 0x28 (40..47)
    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    // ICW3: Cascade configuration: Master has slave at IRQ2 (0x04), Slave cascade identity (0x02)
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW4: 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Mask all PIC interrupts initially so unhandled IRQs do not trigger
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

void init_idt(void)
{
    // 1. Clear all 256 IDT gates
    for (int i = 0; i < IDT_ENTRIES; i++) {
        set_idt_gate(i, 0);
    }

    // 2. Install exception ISR handlers for vectors 0 through 31
    for (int i = 0; i < 32; i++) {
        set_idt_gate(i, (uint64_t)(uintptr_t)isr_table[i]);
    }

    // 3. Remap PIC away from CPU exceptions (0..31 -> 32..47) and mask
    pic_remap();

    // 4. Configure IDTR and load it using lidt
    idtr.limit = (uint16_t)(sizeof(idt) - 1);
    idtr.base = (uint64_t)(uintptr_t)&idt[0];
    idt_load(&idtr);
}

void exception_handler(struct interrupt_frame *frame)
{
    const char *name = (frame->vector < 32) ? exception_messages[frame->vector] : "Unknown Interrupt";

    serial_print("\r\n=======================================================\r\n");
    serial_print(" [CPU EXCEPTION CAUGHT] SYSTEM HALTED\r\n");
    serial_print("=======================================================\r\n");
    serial_printf("Exception:  %s (Vector %lu)\r\n", name, frame->vector);
    serial_printf("Error Code: 0x%016lx (%lu)\r\n", frame->error_code, frame->error_code);
    serial_printf("RIP:        0x%016lx  CS:     0x%04lx\r\n", frame->rip, frame->cs);
    serial_printf("RFLAGS:     0x%016lx\r\n", frame->rflags);
    serial_printf("RSP:        0x%016lx  SS:     0x%04lx\r\n", frame->rsp, frame->ss);

    if (frame->vector == 14) { // #PF Page Fault
        uint64_t cr2;
        __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));
        serial_printf("Fault Address (CR2): 0x%016lx\r\n", cr2);
        serial_printf("PF Details: %s | %s | %s | %s\r\n",
                      (frame->error_code & 1) ? "Protection violation" : "Non-present page",
                      (frame->error_code & 2) ? "Write operation" : "Read operation",
                      (frame->error_code & 4) ? "User mode (Ring 3)" : "Supervisor mode (Ring 0)",
                      (frame->error_code & 8) ? "Reserved bit violation" : "No reserved bit set");
    } else if (frame->vector == 13) { // #GP General Protection Fault
        serial_printf("GPF Details: Selector Index = %lu, Table = %s, External = %lu\r\n",
                      (frame->error_code >> 3) & 0x1FFF,
                      ((frame->error_code >> 1) & 0x03) == 0 ? "GDT" :
                      (((frame->error_code >> 1) & 0x03) == 1 ? "IDT" : "LDT"),
                      frame->error_code & 1);
    }

    serial_printf("RAX: 0x%016lx  RBX: 0x%016lx  RCX: 0x%016lx\r\n", frame->rax, frame->rbx, frame->rcx);
    serial_printf("RDX: 0x%016lx  RSI: 0x%016lx  RDI: 0x%016lx\r\n", frame->rdx, frame->rsi, frame->rdi);
    serial_printf("RBP: 0x%016lx  R8:  0x%016lx  R9:  0x%016lx\r\n", frame->rbp, frame->r8, frame->r9);
    serial_printf("R10: 0x%016lx  R11: 0x%016lx  R12: 0x%016lx\r\n", frame->r10, frame->r11, frame->r12);
    serial_printf("R13: 0x%016lx  R14: 0x%016lx  R15: 0x%016lx\r\n", frame->r13, frame->r14, frame->r15);
    serial_print("=======================================================\r\n");

    // Paint the screen with emergency red / maroon color to visually signal CPU exception
    if (var.params && var.params->GPU_Configs.FrameBuffer) {
        volatile uint32_t *fb = (volatile uint32_t *)(uintptr_t)var.params->GPU_Configs.FrameBuffer;
        uint32_t pitch = var.params->GPU_Configs.pitch;
        uint32_t width = var.params->GPU_Configs.width;
        uint32_t height = var.params->GPU_Configs.height;
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                fb[y * pitch + x] = 0x00880000; // Red
            }
        }
    }

    while (1) {
        __asm__ volatile ("cli; hlt");
    }
}
