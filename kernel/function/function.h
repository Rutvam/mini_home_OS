#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#ifndef FUNCTION_KERNEL_H
#define FUNCTION_KERNEL_H

extern void outb (unsigned short port, unsigned char value);
extern uint8_t inb (uint16_t port);

struct PRINT_FUNCTION
{
	void (*basic)(const char *text, ...);
	void (*outb)(const char *text, ...);
};

struct FUNCTION
{
	void *(*memcpy)(void *dest, const void *src, size_t num);
	void (*scroll)(void);
	void (*clear)(void);
	int (*time)(void);
};

struct CMP_FUNCTION
{
	int (*string)(const char *first_string, const char *second_string);
	int (*key)(uint8_t value, uint8_t SCC_press, uint8_t SCC_release, uint8_t need_ethendu, uint8_t last_scancode);
};

struct INIT
{
	void (*var)(void);
	void (*GDT)(void);
	void (*IDT)(void);
	void (*time)(void);
	void (*keyboard)(void);
	void (*function)(int is_qemu);
};

struct RANDOM_FUNCTION
{
	int (*randint)(int min, int max);
};

struct MATH_FUNCTION
{
	int (*pow)(int base, int exposant);
};

struct SHUTDOWN_FUNCTION
{
	
};

struct SCREEN_FUNCTION
{
	void (*analyse)(void);
};

struct INTERRUPT_FUNCTION {
	void (*pic_remap)(void);
	void (*set_idt_gate)(int n, uint64_t handler_address);
};

struct PS2_FUNCTION {
	void (*send)(uint8_t value);
	uint8_t (*read)(void);
	void (*read_keyboard)(void);
};

struct MY_FUNCTION
{
	struct INIT init;
	struct FUNCTION function;
	struct CMP_FUNCTION cmp;
	struct PS2_FUNCTION ps2;
	struct MATH_FUNCTION math;
	struct PRINT_FUNCTION print;
	struct SCREEN_FUNCTION screen;
	struct RANDOM_FUNCTION random;
	struct SHUTDOWN_FUNCTION shutdown;
	struct INTERRUPT_FUNCTION interrupt;
	void (*outb)(unsigned short port, unsigned char value);
	uint8_t (*inb)(uint16_t port);
};

extern struct MY_FUNCTION my;

void init_function(int is_qemu);

#endif
