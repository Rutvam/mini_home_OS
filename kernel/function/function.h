#include <stdint.h>
#include <stdarg.h>
#ifndef FUNCTION_H
#define FUNCTION_H


struct PRINT_FUNCTION
{
	void (*basic)(const char *text, ...);
	void (*outb)(const char *text, ...);
};

struct FUNCTION
{
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
	void (*function)(int is_qemu);
	void (*var)(void);
	void (*time)(void);
};

struct RANDOM_FUNCTION
{
	int (*randint)(int min, int max);
};

struct MATH_FUNCTION
{
	int (*pow)(int base, int exposant);
};

struct SCREEN_FUNCTION
{
	void (*analyse)(void);
};

struct MY_FUNCTION
{
	struct INIT init;
	struct FUNCTION function;
	struct CMP_FUNCTION cmp;
	struct MATH_FUNCTION math;
	struct PRINT_FUNCTION print;
	struct SCREEN_FUNCTION screen;
	struct RANDOM_FUNCTION random;
	void (*outb)(unsigned short port, unsigned char value);
};

extern struct MY_FUNCTION my;

void init_function(int is_qemu);

#endif
