#include <stdint.h>
#include "../data/global_value.h"
#include "../function/function.h"

char memory[300] = {0x00};

void erase_memory(void)
{
	for (int i = 0; i < 300; i++)
	{
		memory[i] = 0;
	}
}

int main_shell ()
{
	var.cursor.position = 0;
	int nb_char = 0;

	char PROMPT[4] = {'>', '>', ' ', 0x00};

    // 1. On prépare l'affichage
	my.function.clear();
    my.print.basic(PROMPT);

    // 4. Boucle de repos (Le CPU attend sagement ici)
	while (1)
	{
		if (var.scancode) {
			// =================================================
			//  Step 1: Check if the extended prefix is predent
			// =================================================

			if (var.scancode == var.ethendu.SCC_prefix) {
				var.ethendu.value = 1;
			} else {
				var.ethendu.value = 0;
			}

			// ===========================================
			//  Step 2: Check if the Shift key is pressed
			// ===========================================
			// 1. We start with the left Shift key.
			// 2. Next, we continue with the right Shift key.
			var.key.MAJ_L.value = my.cmp.key(var.key.MAJ_L.value, var.key.MAJ_L.SCC_press, var.key.MAJ_L.SCC_release, var.key.MAJ_L.need_ethendu, var.scancode);
			var.key.MAJ_R.value = my.cmp.key(var.key.MAJ_R.value, var.key.MAJ_R.SCC_press, var.key.MAJ_R.SCC_release, var.key.MAJ_R.need_ethendu, var.scancode);
			var.key.ALT_L.value = my.cmp.key(var.key.ALT_L.value, var.key.ALT_L.SCC_press, var.key.ALT_L.SCC_release, var.key.ALT_L.need_ethendu, var.scancode);
			var.key.ALT_R.value = my.cmp.key(var.key.ALT_R.value, var.key.ALT_R.SCC_press, var.key.ALT_R.SCC_release, var.key.ALT_R.need_ethendu, var.scancode);
			var.key.ENTER.value = my.cmp.key(var.key.ENTER.value, var.key.ENTER.SCC_press, var.key.ENTER.SCC_release, var.key.ENTER.need_ethendu, var.scancode);
			var.key.DELET.value = my.cmp.key(var.key.DELET.value, var.key.DELET.SCC_press, var.key.DELET.SCC_release, var.key.DELET.need_ethendu, var.scancode);
            // pas fini ...
            if (var.key.ENTER.value)
			{
                var.scancode = 0;
                var.cursor.position = ((var.cursor.position) / 160 + 1) * 160;

				if (0 == my.cmp.string(memory, var.command.shutdown)) {
					// shutdown_function();
					break; // temporair exit
				} else if (0 == my.cmp.string(memory, var.command.exit)) {
					break;
				} else if (0 == my.cmp.string(memory, var.command.clear)) {
					my.function.clear();
					var.cursor.position = 0;
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
                var.scancode = 0;
                var.cursor.position -= 2;
                var.screen.VB[var.cursor.position] = ' ';
                var.screen.VB[var.cursor.position+1] = 0x0F;

                nb_char--;
                memory[nb_char] = 0;
                continue;
            }

            if (var.cursor.position > 4000){
                for (int k = 160; k < 4000; k++)
                {
                    var.screen.VB[k-160] = var.screen.VB[k];
                }

                for (int k = 3840; k < 3999; k += 2)
                {
                    var.screen.VB[k] = ' ';
                    var.screen.VB[k+1] = 0x0F;
                }

                var.cursor.position = 3840;
            } else {
                uint8_t sc = var.scancode;
                var.scancode = 0;
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
                    c = var.key.conf.actuel_position[1][sc];
                    memory[var.cursor.position] = c;
                } else {
                    c = var.key.conf.actuel_position[0][sc];
                    memory[var.cursor.position] = c;
                }
				var.screen.VB[var.cursor.position] = c;
				var.screen.VB[var.cursor.position+1] = 0x0F;
                var.cursor.position += 2;
                memory[nb_char] = c;
                nb_char++;
            }
        }
    }
}
