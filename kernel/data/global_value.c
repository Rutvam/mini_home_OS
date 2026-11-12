// Debut global_value.c
#include <stdint.h>
#include "global_value.h"

struct VARIABLE var;
uint32_t default_video_buffer[8192];

void set_loader_params(struct LOADER_PARAMS *params)
{
	var.params = params;
}

// Debut global_value.c
