#include "math.h"
int pow(int base, int exposant)
{
	int result = 1;
	for (int i = 0; i < exposant; i++)
	{
		result *= base;
	}
	return result;
}
