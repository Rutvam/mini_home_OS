#include <stdint.h>
#include "../idt/IDT.h"
#include "../data/global_value.h"
#include "../../keyboard_driver/main.h"
#include "../function/function.h"

uint8_t map[25][80] = {0};

struct SPRITE {
	char skin;
	uint8_t value;
	uint8_t xc; // X Coordination
	uint8_t yc; // Y Coordination
	uint8_t xd; // X Direction 
	uint8_t yd; // Y Direction 
};

struct SPRITE ball = {
	.skin = 'O',
	.value = 9,
	.xc = 39, // X Coordination
	.yc = 12, // Y Coordination
	.xd = 0, // X Direction 
	.yd = 0, // Y Direction 
};

struct SPRITE player1 = {
	.skin = '#',
	.value = 1,
	.xc = 1, // X Coordination
	.yc = 12, // Y Coordination
	.xd = 0, // X Direction 
	.yd = 0, // Y Direction 
};

struct SPRITE player2 = {
	.skin = '#',
	.value = -1,
	.xc = 78, // X Coordination
	.yc = 12, // Y Coordination
	.xd = 0, // X Direction 
	.yd = 0, // Y Direction 
};

char analyze_field (int field_value)
{
	switch (field_value)
	{
		case  9: return 'O';
		case  1: return '#';
		case  0: return ' ';
		case -1: return '#';
		default: return '?';
	}
}

void init_sprites()
{
	map[ ball.yc ][ ball.xc ] = ball.value;
	map[ player1.yc ][ player1.xc ] = player1.value;
	map[ player2.yc ][ player2.xc ] = player2.value;
}

void print_map ()
{
	char* video = (char*)0xB8000;

	for (int x = 0; x < 80; x++)
	{
		for (int y = 0; y < 25; y++)
		{
			video[160 * y + x * 2] = analyze_field(map[y][x]);
		}
	}
}

int main_pong ()
{
	char* video_memory = (char*)0xB8000;

	// 1. On prépare l'affichage
	my.function.clear();
	
	// 2. Configuration matérielle (Une seule fois !)
	pic_remap();
	set_idt_gate(33, (uint32_t)(uintptr_t)keyboard_handler_asm);
	init_idt();

	// 3. On ouvre les vannes du clavier
	__asm__ __volatile__("sti");

	// 4. Boucle de repos (Le CPU attend sagement ici)
	while (1)
	{
		init_sprites();
		if (last_scancode) {
			if (last_scancode == 0x1C)
			{
				last_scancode = 0;
				continue;
			}

            if (var.screen.cursor_position > 4000){
                for (int k = 160; k < 4000; k++)
                {
                    video_memory[k-160] = video_memory[k];
                }

                for (int k = 3840; k < 3999; k += 2)
                {
                    video_memory[k] = ' ';
                    video_memory[k+1] = 0x0F;
                }

                var.screen.cursor_position = 3840;
            } else {
                uint8_t sc = last_scancode;
                last_scancode = 0;
                if (sc > 127)
                {
                    continue;
                }
                if (sc & 0x80)
                {
                    continue;
                }

                char c;
                // Essaie de mapper vers un caractère
                c = qwertz_german[0][sc];
                if (c == 'w' || c == 'W') {}
                if (c == 's' || c == 'S') {}
            }
        }
    }
}
