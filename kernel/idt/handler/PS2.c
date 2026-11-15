#include "../../data/global_value.h"
#include "../../function/function.h"
#include "./PS2.h"

void ps2_send(uint8_t value)
{
	while (my.inb(0x64) & 2)
	{
		// Attendre
	}

	my.outb(0x60, value);
}

uint8_t ps2_read(void)
{
	while (!(my.inb(0x64) & 1))
	{
		// Wait
	}

	return my.inb(0x60);
}

void read_keyboard_set1(void)
{
	int ac = 0;
	int SHIFT = 0;
	uint8_t scancode = var.scancode;
	uint8_t last_scancode = var.last_scancode;
	// First step: 0xE0 SCC detect
	if (scancode == var.ethendu.SCC_prefix) {
		var.ethendu.value = 1;
	} else {
		if (last_scancode != var.ethendu.SCC_prefix || var.ethendu.value == 0)
		{
			// Enter
			if (scancode == var.key.ENTER.SCC_press) {
				var.key.ENTER.value = 1;
			} else if (scancode == var.key.ENTER.SCC_release) {
				var.key.ENTER.value = 0;
			}
			// Shift Left
			if (scancode == var.key.MAJ_L.SCC_press) {
				var.key.MAJ_L.value = 1;
			} else if (scancode == var.key.MAJ_L.SCC_release) {
				var.key.MAJ_L.value = 0;
			}
			// Shift Right
			if (scancode == var.key.MAJ_R.SCC_press) {
				var.key.MAJ_R.value = 1;
			} else if (scancode == var.key.MAJ_R.SCC_release) {
				var.key.MAJ_R.value = 0;
			}
			var.ethendu.value = 0;
		}

		if (last_scancode == var.ethendu.SCC_prefix || var.ethendu.value == 1)
		{
			if (scancode == var.key.FLECHE_L.SCC_press) {
				var.key.FLECHE_L.value = 1;
			} else if (scancode == var.key.FLECHE_L.SCC_release) {
				var.key.FLECHE_L.value = 0;
			}
			if (scancode == var.key.FLECHE_R.SCC_press) {
				var.key.FLECHE_R.value = 1;
			} else if (scancode == var.key.FLECHE_R.SCC_release) {
				var.key.FLECHE_R.value = 0;
			}
			if (scancode == var.key.FLECHE_T.SCC_press) {
				var.key.FLECHE_T.value = 1;
			} else if (scancode == var.key.FLECHE_T.SCC_release) {
				var.key.FLECHE_T.value = 0;
			}
			if (scancode == var.key.FLECHE_B.SCC_press) {
				var.key.FLECHE_B.value = 1;
			} else if (scancode == var.key.FLECHE_B.SCC_release) {
				var.key.FLECHE_B.value = 0;
			}
			var.ethendu.value = 0;
		}

		// Thirt step: action var special keys
		if (var.key.ENTER.value == 1)
		{
			my.print.basic("\n");
			var.key.ENTER.value = 0;
			ac = 1;
		}

		if (var.key.FLECHE_L.value == 1)
		{
			var.cursor.position -= 2;
			var.key.FLECHE_L.value = 0;
			ac = 1;
		}

		if (var.key.FLECHE_R.value == 1)
		{
			var.cursor.position += 2;
			var.key.FLECHE_R.value = 0;
			ac = 1;
		}

		if (var.key.FLECHE_T.value == 1)
		{
			// var.screen.cursor_position -= 2;
			var.cursor.position = 0;
			var.key.FLECHE_T.value = 0;
			ac = 1;
		}

		if (var.key.FLECHE_B.value == 1)
		{
			var.cursor.position += 2;
			var.key.FLECHE_B.value = 0;
			ac = 1;
		}

		if (var.key.MAJ_L.value || var.key.MAJ_R.value) {
			SHIFT = 1;
		} else if (!var.key.MAJ_L.value && !var.key.MAJ_R.value) {
			SHIFT = 0;
		}

		// Last step: Print a char
		if (scancode < 0x80 && !ac)
		{
			unsigned char ascii = var.key.conf.actuel_position[SHIFT][scancode];
	
			if (ascii != 0)
			{
				my.print.basic("%c", ascii);
			}
		}
	}
	var.scancode = scancode;
	var.last_scancode = last_scancode;
}

void read_keyboard_set2(void)
{
	int ac = 0;
	int SHIFT = 0;
	uint8_t scancode = var.scancode;
	uint8_t last_scancode = var.last_scancode;
    static uint8_t released = 0;

    // Gestion du préfixe 0xE0 (Touches étendues)
    if (scancode == 0xE0) {
        var.ethendu.value = 1;
        return;
    }

    // Gestion du préfixe de relâchement 0xF0 en Set 2
    if (scancode == 0xF0) {
        released = 1;
        return;
    }

    // À ce stade, 'scancode' est le code de la touche
    // Exemple de correspondance pour les touches spéciales en Set 2
    // (Assurez-vous que vos structures .SCC_press stockent les valeurs du Set 2)
    if (scancode == var.key.ENTER.SCC_press) {
        if (!released) {
            var.key.ENTER.value = 1;
            my.print.basic("\n");
            ac = 1;
        } else {
            var.key.ENTER.value = 0;
        }
    }

    // Gestion des flèches (nécessitent souvent le préfixe 0xE0 en Set 2)
    if (var.ethendu.value) {
        if (scancode == 0x6B) { // Flèche gauche en Set 2 (avec 0xE0)
            if (!released) { var.cursor.position -= 2; ac = 1; }
        }
        if (scancode == 0x74) { // Flèche droite en Set 2 (avec 0xE0)
            if (!released) { var.cursor.position += 2; ac = 1; }
        }
        // Réinitialisation du préfixe étendu après traitement
        if (released) {
            var.ethendu.value = 0;
        }
    }

    // Gestion de l'état Shift (Maj) en Set 2
    if (scancode == var.key.MAJ_L.SCC_press || scancode == var.key.MAJ_R.SCC_press) {
        SHIFT = !released;
    }

    // Impression des caractères normaux (Set 2 possède sa propre table de conversion)
    if (!released && !ac && scancode < 0x80) {
        unsigned char ascii = var.key.conf.actuel_position[SHIFT][scancode]; // Attention: nécessite une table adaptée au Set 2
        if (ascii != 0) {
            my.print.basic("%c", ascii);
        }
    }

    // Nettoyage de l'état de relâchement pour le prochain octet
    if (released) {
        released = 0;
    }
	var.scancode = scancode;
	var.last_scancode = last_scancode;
}

void read_keyboard_set3(void)
{
	int SHIFT = 0;
	uint8_t scancode = var.scancode;
	uint8_t last_scancode = var.last_scancode;
	static uint8_t released = 0;
	
	if (scancode == 0xF0) {
	    released = 1;
	    return;
	}

    int ac = 0;

    // Traitement de la touche Entrée en Set 3
    if (scancode == var.key.ENTER.SCC_press) {
        if (!released) {
            my.print.basic("\n");
            ac = 1;
        }
    }

    // Gestion des touches Shift
    if (scancode == var.key.MAJ_L.SCC_press || scancode == var.key.MAJ_R.SCC_press) {
        SHIFT = !released;
    }

    // Impression du caractère (nécessite une table de correspondance pour le Set 3)
    if (!released && !ac && scancode < 0x80) {
        unsigned char ascii = var.key.conf.actuel_position[SHIFT][scancode]; // Adapter la table au Set 3
        if (ascii != 0) {
            my.print.basic("%c", ascii);
        }
    }

    if (released) {
        released = 0;
	}
	var.scancode = scancode;
	var.last_scancode = last_scancode;
}

void read_keyboard(void)
{
	switch(var.key.conf.set)
	{
		case 1: read_keyboard_set1(); break;
		case 2: read_keyboard_set2(); break;
		case 3: read_keyboard_set3(); break;
	}
}
