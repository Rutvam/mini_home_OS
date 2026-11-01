#include "../data/global_value.h"
#include "../function/function.h"

void main_shutdown (void)
{
	my.function.clear();
	var.screen.cursor_position = 0;
	my.print.basic("[ ... ] exec shutdown\n");
	var.status.msg = "ERROR: Funktion shutdown not finich\n";
	var.status.value = 1;
	if (var.status.value == 1)
	{
		var.screen.cursor_position = 0;
		my.print.basic("[ X ] exec shutdown  \n");
		my.print.basic(var.status.msg);
		return;
	}
}
