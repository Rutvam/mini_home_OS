#include <stdint.h>
#include "./graphics.h"
#include "../function.h"
#include "../../data/global_value.h"

void screen_analyze(void)
{
	if (!var.screen.VB)
		return;

	int j = 0;
	for (var.screen.cursor_position = 0; var.screen.cursor_position < var.screen.VBSize; var.screen.cursor_position += 2)
	{
		for (int y = 0; y < var.screen.caracter_height; y++)
		{
			for (int x = 0; x < var.screen.caracter_width; x++)
			{
				if (var.screen.police[ var.screen.VB[ var.screen.cursor_position ]-1 ][y][x])
				{
					int temp = j * 8;
					var.screen.pixel_position = temp + (y * var.screen.FBPitch + x);
					var.screen.FB[ var.screen.pixel_position ] = var.screen.VB[ var.screen.cursor_position+1 ];
				}
			}
		}
		j++;
	}
}
