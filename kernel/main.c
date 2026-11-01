// Debut main.c
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
	my.print.outb("[ OK ] Main\r\n");
	
	if (params == 0)
	{
		my.print.outb("[ X ] Params\r\n");
		while (1);
	}
	var.params = params; // save params
	if (var.params == 0)
	{
		my.print.outb("[ X ] Init Params\r\n");
		while (1);
	}
	my.print.outb("[ OK ] Params\r\n");

	my.init.function = init_function; // init the init function.
	my.init.function(1); // 1 = yes, it is with QEMU; 0 = No, it's not with QEMU
	my.print.outb("[ OK ] Init all function in struct my.");

	my.init.var();
	my.print.outb("[ OK ] Init all variable in struct var.");


	my.print.outb("FrameBuffer size: %d\r\n", var.params->GPU_Configs->FrameBuffer_Size);
	my.print.outb("UEFI version: %h\r\n", params->UEFI_Version);
	my.print.outb("Bootloader version: %d.%d\r\n", params->Bootloader_MajorVersion, params->Bootloader_MinorVersion);
	int base = my.random.randint(0, 20);
	int exp = my.random.randint(1, 3);
	int value = my.math.pow(base, exp);
	// my.print.basic("AB %d", my.random.randint(-100, 100));
	// my.print.basic(" ----- ");
	// my.print.basic("%d^(%d) = %d | ", base, exp, value);
	// my.print.basic("%d/%d/%d %d:%d.%d | ", var.time.day, var.time.month, var.time.year, var.time.hour, var.time.minute, var.time.second);
	// for (int i = 0; i < 100; i++)
	// {
	// 	my.print.basic("%c", my.random.randint(65, 91));
	// }
	my.screen.analyse();

	while (1);

    return 0;
}
// Fin main.c