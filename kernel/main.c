#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include "./idt/IDT.h"
#include "./function/function.h"
#include "./data/global_value.h"
#include "./pong/main_pong.h"
#include "./shell/main_shell.h"
#include "./function/graphics/graphics.h"
#include "./shutdown/main_shutdown.h"
#include "../keyboard_driver/main.h"

// Dans kernel.c
int main(struct LOADER_PARAMS *params)
{
	if (params == 0)
	{
		my.print.outb("[ X ] Params\r\n");
		while (1);
	}
	var.params = params; // save params
	// init_function(1);
	my.init.function = init_function; // init the init function.
	my.init.function(1); // 1 = yes, it is with QEMU; 0 = No, it's not with QEMU
	my.print.outb("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
	my.print.outb("[ OK ] Main\r\n");
	my.print.outb("[ OK ] Params\r\n");
	my.print.outb("[ OK ] Function initialiation\r\n");
	my.init.var();
	my.print.outb("[ OK ] Variable initialiation\r\n");
	my.print.basic("Hello\nMy name is ...");
	my.print.outb("[ OK ] Graphic Test\r\n");
	my.screen.analyse();

	while (1);

    return 0;
}
