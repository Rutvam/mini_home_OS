#include <stdint.h>
#include "./graphics.h"
#include "../function.h"
#include "../../data/global_value.h"

void screen_analyze(void)
{
	var.cursor.y = (var.cursor.position / var.screen.VBWidth) * var.screen.caracter_height;
	var.cursor.x = (var.cursor.position % var.screen.VBWidth) * var.screen.caracter_width;
	if (!var.screen.VB) {return;}

	int j = 0;
	for (var.cursor.position = 0; var.cursor.position < var.screen.VBSize; var.cursor.position += 2)
	{
		for (int y = 0; y < var.screen.caracter_height; y++)
		{
			for (int x = 0; x < var.screen.caracter_width; x++)
			{
				if (var.screen.police[ var.screen.VB[ var.cursor.position ]-1 ][y][x])
				{
					int temp = j * 8;
					var.screen.pixel_position = temp + (y * var.screen.FBPitch + x);
					var.screen.FB[ var.screen.pixel_position ] = var.screen.VB[ var.cursor.position+1 ];
				}
			}
		}
		j++;
	}
}
