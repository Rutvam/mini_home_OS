#include "../../function/function.h"
#include "../../data/global_value.h"
#include "./keyboard_handler.h"

extern void keyboard_handler_asm();

int SHIFT = 0;

void keyboard_init(void)
{
	if (!var.isQEMU) {
		// 1. Envoyer la commande
		my.ps2.send(0xF0);
	
		// 2. Attendre ACK
		uint8_t response = my.ps2.read();
	
		if (response == 0xFA)
		{
			// 3. Demander le Scancode Set actuel
			my.ps2.send(0x00);

			// 4. Lire la réponse
			response = my.ps2.read();

			if (response == 0xFA)
			{
				uint8_t set = my.ps2.read();
				if (1 <= set && set <= 3) {
					var.key.conf.set = set;
				} else {
					var.key.conf.set = 0;
				}
			}
		}
	} else if (var.isQEMU) {
		var.key.conf.set = 1;
	}
}



void keyboard_handler_c(void)
{
	__asm__ __volatile__("inb $0x60, %0" : "=a"(var.scancode));
	my.ps2.read_keyboard();
	var.last_scancode = var.scancode;
	__asm__ __volatile__("outb %0, %1" : : "a"((uint8_t)0x20), "d"((uint16_t)0x20));
}
