//#include <efi.h>
#include "./function.h"
#include "./random/random.h"
#include "./math/math.h"
#include "./print_function/print_function.h"
#include "./graphics/graphics.h"
#include "../data/global_value.h"
#include "../data/graphics_value/police.h"

void scroll_video(void)
{
    for (int k = 160; k < 4000; k++)
        var.screen.VB[k - 160] = var.screen.VB[k];

    for (int k = 3840; k < 4000; k += 2)
    {
        var.screen.VB[k] = ' ';
        var.screen.VB[k + 1] = 0x0F;
    }

    var.screen.cursor_position = 3840;
}

void clear(void)
{
	my.print.outb("\033[2J\033[H");
	for (int i = 0; i < var.screen.FBSize; i++)
	{
		var.screen.FB[i] = 0x000000;
	}
	return;
}

int string(const char *first_string, const char *second_string)
{
	while (*first_string && *second_string && *first_string == *second_string)
	{
		first_string++;
		second_string++;
	}
	return (unsigned char)*first_string - (unsigned char)*second_string;
}

int key(uint8_t value, uint8_t SCC_press, uint8_t SCC_release, uint8_t need_ethendu, uint8_t last_scancode)
{
	if (var.ethendu.value == need_ethendu)
	{
		if (SCC_press == last_scancode) {
			return 1;
		} else if (SCC_release == last_scancode) {
			return 0;
		}
	}
	return value;
}



int time(void)
{
	var.params->RTServices->GetTime(&var.params->BootTime, NULL);
	my.init.time();
	return var.time.second * my.math.pow(60, 0) + var.time.minute * my.math.pow(60, 1) + var.time.hour * my.math.pow(60, 2);
}

void init_time()
{
	var.time.day = var.params->BootTime.Day;
	var.time.hour = var.params->BootTime.Hour;
	var.time.year = var.params->BootTime.Year;
	var.time.month = var.params->BootTime.Month;
	var.time.minute = var.params->BootTime.Minute;
	var.time.second = var.params->BootTime.Second;
}

void init_var()
{
	var.key.TAB.value = 0;
	var.key.TAB.SCC_press = 0x0F;
	var.key.TAB.SCC_release = 0x8F;
	var.key.ALT_L.value = 0;
	var.key.ALT_L.SCC_press = 0x38;
	var.key.ALT_L.SCC_release = 0xB8;
	var.key.ALT_L.need_ethendu = 0;
	var.key.ALT_R.value = 0;
	var.key.ALT_R.SCC_press = 0x38;
	var.key.ALT_R.SCC_release = 0xB8;
	var.key.ALT_R.need_ethendu = 1;
	var.key.DELET.value = 0;
	var.key.DELET.SCC_press = 0x53;
	var.key.DELET.SCC_release = 0xD3;
	var.key.DELET.need_ethendu = 1;
	var.key.ENTER.value = 0;
	var.key.ENTER.SCC_press = 0x1C;
	var.key.ENTER.SCC_release = 0x9C;
	var.key.ENTER.need_ethendu = 0;
	var.key.MAJ_L.value = 0;
	var.key.MAJ_L.SCC_press = 0x2A;
	var.key.MAJ_L.SCC_release = 0xAA;
	var.key.MAJ_L.need_ethendu = 0;
	var.key.MAJ_R.value = 0;
	var.key.MAJ_R.SCC_press = 0x36;
	var.key.MAJ_R.SCC_release = 0xB6;
	var.key.MAJ_R.need_ethendu = 0;
	var.key.CTRL_L.value = 0;
	var.key.CTRL_L.SCC_press = 0x1D;
	var.key.CTRL_L.SCC_release = 0x9D;
	var.key.CTRL_L.need_ethendu = 0;
	var.key.CTRL_R.value = 0;
	var.key.CTRL_R.SCC_press = 0x1D;
	var.key.CTRL_R.SCC_release = 0x9D;
	var.key.CTRL_R.need_ethendu = 1;
	var.key.FLECHE_L.value = 0;
	var.key.FLECHE_L.SCC_press = 0x4B;
	var.key.FLECHE_L.SCC_release = 0xCB;
	var.key.FLECHE_L.need_ethendu = 1;
	var.key.FLECHE_R.value = 0;
	var.key.FLECHE_R.SCC_press = 0x4D;
	var.key.FLECHE_R.SCC_release = 0xCD;
	var.key.FLECHE_R.need_ethendu = 1;
	var.key.FLECHE_T.value = 0;
	var.key.FLECHE_T.SCC_press = 0x48;
	var.key.FLECHE_T.SCC_release = 0xC8;
	var.key.FLECHE_T.need_ethendu = 1;
	var.key.FLECHE_B.value = 0;
	var.key.FLECHE_B.SCC_press = 0x50;
	var.key.FLECHE_B.SCC_release = 0xD0;
	var.key.FLECHE_B.need_ethendu = 1;
	var.key.CAPS_LOCK.value = 0;
	var.key.CAPS_LOCK.SCC_press = 0x3A;
	var.key.CAPS_LOCK.SCC_release = 0xBA;
	var.key.CAPS_LOCK.need_ethendu = 0;
	var.knopf.pong.name = "Pong";
	var.knopf.pong.value = 0;
	var.knopf.shell.name = "Shell";
	var.knopf.shell.value = 1;
	var.knopf.shutdown.name = "Shutdown";
	var.knopf.shutdown.value = 0;
	var.screen.FB = (uint32_t *)(uintptr_t)var.params->GPU_Configs.FrameBuffer;
	var.screen.police = police;
	var.screen.caracter_width = 8;
	var.screen.caracter_height = 12;
	var.screen.FBWidth = (int)var.params->GPU_Configs.width;
	var.screen.FBHeight = (int)var.params->GPU_Configs.height;
	var.screen.FBSize = (int)var.params->GPU_Configs.FrameBuffer_Size;
	var.screen.FBPitch = (int)var.params->GPU_Configs.pitch;
	var.screen.VB = default_video_buffer;
	for (int i = 0; i < 8192; i++)
	{
		default_video_buffer[i] = 0;
	}
	var.screen.VBWidth = var.screen.FBWidth / var.screen.caracter_width * 2;
	var.screen.VBHeight = var.screen.FBHeight / var.screen.caracter_height;
	var.screen.VBSize = 8192;
	var.screen.pixel_position = 0;
	var.screen.cursor_position = 0;
	var.command.shutdown = "shutdown";
	var.command.exit = "exit";
	var.command.clear = "clear";
	var.command.clist = "clist";
	var.ethendu.value = 0;
	var.ethendu.SCC_prefix = 0xE0;
	my.init.time();
}

struct MY_FUNCTION my;

void init_function(int is_qemu)
{
	my.init.time = init_time;
	my.outb = outb;
	my.cmp.key = key;
	my.cmp.string = string;
	my.math.pow = pow;
	my.init.var = init_var;
	my.print.basic = print;
	my.random.randint = randint;
	my.screen.analyse = screen_analyze;
	my.function.time = time;
	my.function.clear = clear;
	my.function.scroll = scroll_video;
	if (is_qemu) {
		my.print.outb = print_outb;
	} else {
		my.print.outb = NO_print;
	}
	seed_random(my.function.time());
	return;
}
