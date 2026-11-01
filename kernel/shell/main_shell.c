#include <stdint.h>
#include "../idt/IDT.h"
#include "../data/global_value.h"
#include "../function/function.h"
#include "../shutdown/main_shutdown.h"
#include "../../keyboard_driver/main.h"

char memory[300] = {0x00};

void erase_memory(void)
{
	for (int i = 0; i < 300; i++)
	{
		memory[i] = 0;
	}
}

static inline unsigned char inb (unsigned short port)
{
	unsigned char value;
	__asm__ volatile (".intel_syntax noprefix\n" "in al, dx\n" ".att_syntax\n" : "=a"(value) : "d"(port));
	return value;
}



int main_shell ()
{
	var.screen.cursor_position = 0;
	int nb_char = 0;
	char* video_memory = (char*)0xB8000;

	char PROMPT[4] = {'>', '>', ' ', 0x00};

    // 1. On prépare l'affichage
	my.function.clear();
    my.print.basic(PROMPT);
	
    // 2. Configuration matérielle (Une seule fois !)
    pic_remap();
    set_idt_gate(33, (uint32_t)(uintptr_t)keyboard_handler_asm);
    init_idt();

    // 3. On ouvre les vannes du clavier
    __asm__ __volatile__("sti");

    // 4. Boucle de repos (Le CPU attend sagement ici)
	while (1)
	{
		if (last_scancode) {
			// =================================================
			//  Step 1: Check if the extended prefix is predent
			// =================================================

			if (last_scancode == var.ethendu.SCC_prefix) {
				var.ethendu.value = 1;
			} else {
				var.ethendu.value = 0;
			}

			// ===========================================
			//  Step 2: Check if the Shift key is pressed
			// ===========================================
			// 1. We start with the left Shift key.
			// 2. Next, we continue with the right Shift key.
			var.key.MAJ_L.value = my.cmp.key(var.key.MAJ_L.value, var.key.MAJ_L.SCC_press, var.key.MAJ_L.SCC_release, var.key.MAJ_L.need_ethendu, last_scancode);
			var.key.MAJ_R.value = my.cmp.key(var.key.MAJ_R.value, var.key.MAJ_R.SCC_press, var.key.MAJ_R.SCC_release, var.key.MAJ_R.need_ethendu, last_scancode);
			var.key.ALT_L.value = my.cmp.key(var.key.ALT_L.value, var.key.ALT_L.SCC_press, var.key.ALT_L.SCC_release, var.key.ALT_L.need_ethendu, last_scancode);
			var.key.ALT_R.value = my.cmp.key(var.key.ALT_R.value, var.key.ALT_R.SCC_press, var.key.ALT_R.SCC_release, var.key.ALT_R.need_ethendu, last_scancode);
			var.key.ENTER.value = my.cmp.key(var.key.ENTER.value, var.key.ENTER.SCC_press, var.key.ENTER.SCC_release, var.key.ENTER.need_ethendu, last_scancode);
			var.key.DELET.value = my.cmp.key(var.key.DELET.value, var.key.DELET.SCC_press, var.key.DELET.SCC_release, var.key.DELET.need_ethendu, last_scancode);
            // pas fini ...
            if (var.key.ENTER.value)
			{
                last_scancode = 0;
                var.screen.cursor_position = ((var.screen.cursor_position) / 160 + 1) * 160;

				if (0 == my.cmp.string(memory, var.command.shutdown)) {
					// shutdown_function();
					break; // temporair exit
				} else if (0 == my.cmp.string(memory, var.command.exit)) {
					break;
				} else if (0 == my.cmp.string(memory, var.command.clear)) {
					my.function.clear();
					var.screen.cursor_position = 0;
				} else if (0 == my.cmp.string(memory, var.command.clist)) {
					my.print.basic(var.command.clear);
					my.print.basic("\n");
					my.print.basic(var.command.clist);
					my.print.basic("\n");
					my.print.basic(var.command.exit);
					my.print.basic("\n");
					my.print.basic(var.command.shutdown);
					my.print.basic("\n");
				} else {
					my.print.basic("Error: Cette commande n'existe pas.\n");
				}

				erase_memory();
				nb_char = 0;
				my.print.basic(PROMPT);
                continue;
            }
            
            if (var.key.DELET.value) // touche Delet
            {
                last_scancode = 0;
                var.screen.cursor_position -= 2;
                video_memory[var.screen.cursor_position] = ' ';
                video_memory[var.screen.cursor_position+1] = 0x0F;

                nb_char--;
                memory[nb_char] = 0;
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
                if (var.key.MAJ_L.value || var.key.MAJ_R.value) {
                    c = qwertz_german[1][sc];
                    memory[var.screen.cursor_position] = c;
                } else {
                    c = qwertz_german[0][sc];
                    memory[var.screen.cursor_position] = c;
                }
				video_memory[var.screen.cursor_position] = c;
				video_memory[var.screen.cursor_position+1] = 0x0F;
                var.screen.cursor_position += 2;
                memory[nb_char] = c;
                nb_char++;
            }
        }
    }
}
