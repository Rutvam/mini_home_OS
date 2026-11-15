#include <stdint.h>
#include "./idt/handler/GP_handler.h"
#include "./idt/handler/keyboard_handler.h"
#include "./function/function.h"
#include "./data/global_value.h"
#include "./pong/main_pong.h"
#include "./shell/main_shell.h"
#include "./function/graphics/graphics.h"
#include "./shutdown/main_shutdown.h"

// Dans kernel.c
int main(struct LOADER_PARAMS *params)
{
	int is_qemu = 1;
	if (params == 0)
	{
		my.print.outb("[ X ] Params\r\n");
		while (1);
	}
	var.params = params; // save params

	my.init.function = init_function; // init the init function.
	my.init.function(is_qemu);
	// Warning don't move this next function befor `my.init.function`!!!
	my.print.outb("[ OK ] Main\r\n");
	my.print.outb("[ OK ] Params\r\n");
	my.print.outb("[ OK ] Function initialiation\r\n");

	my.init.GDT();
	my.print.outb("[ OK ] Create GDT\r\n");

	my.interrupt.pic_remap();
	my.interrupt.set_idt_gate(0x21, (uint64_t)&keyboard_handler_asm);
	my.interrupt.set_idt_gate(0x0D, (uint64_t)&isr_gp_handler_asm);
	my.init.IDT();
	__asm__ __volatile__("sti");

	my.init.var();
	var.isQEMU = is_qemu;
	my.print.outb("[ OK ] Variable initialiation\r\n");

	my.function.clear();
	my.print.outb("[ OK ] Graphic Test\n\r");

	var.cursor.position = 0;
	my.print.basic("[ ... ] Keyboard init\n");
	var.screen.VB[5] = 0xAA0000;
	var.screen.VB[7] = 0xFF0000;
	var.screen.VB[9] = 0xAA0000;
	char buffer[5] = {0};
	my.print.basic("de = german\nde-ch = german swiss\nfr-ch = french swiss\n>>");
	my.init.keyboard();
	int n = 0;
	while (var.scancode != var.key.ENTER.SCC_press) {
		if (var.scancode < 0x80 && var.last_scancode != var.ethendu.SCC_prefix && n < 5)
		{
			char c = var.key.pos.qwertz_german[0][var.scancode];
			buffer[n] = c;
		}
		my.screen.analyse();
	}
	if (my.cmp.string(buffer, "de-ch")) {
		my.function.memcpy(
			(void *)var.key.conf.actuel_position,
			var.key.pos.qwertz_german_swiss,
			sizeof(var.key.pos.qwertz_german_swiss)
		);
		my.print.basic("german swiss qwertz keyboard\n");
	} else if (my.cmp.string(buffer, "fr-ch")) {
		my.function.memcpy(
			(void *)var.key.conf.actuel_position,
			var.key.pos.qwertz_french_swiss,
			sizeof(var.key.pos.qwertz_french_swiss)
		);
		my.print.basic("french swiss qwertz keyboard\n");
	} else if (my.cmp.string(buffer, "de")) {
		my.function.memcpy(
			(void *)var.key.conf.actuel_position,
			var.key.pos.qwertz_german,
			sizeof(var.key.pos.qwertz_german)
		);
		my.print.basic("german qwertz keyboard\n");
	}
	var.cursor.position = 0;
	my.print.basic("[ OK ] Keyboard init \n");
	my.print.basic("\tset %d\n", var.key.conf.set);

	my.print.outb("[ OK ] Can Start\n\r");
	
	my.print.basic("Hello\n");
	my.print.basic("%d * %d = %d\n", 4, 3, 3*4);


	while (1);
	{
		my.screen.analyse();
	}

    return 0;
}
