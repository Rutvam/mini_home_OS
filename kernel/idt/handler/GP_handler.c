#include "../../function/function.h"

extern void isr_gp_handler_asm();

void isr_gp_handler_c(void) {
	my.print.outb("[ ERREUR FATALE ] General Protection Fault (#GP) détecté !\r\n");
	while (1) {
		__asm__ __volatile__("hlt");
	}
}
