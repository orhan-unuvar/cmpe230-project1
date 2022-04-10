#include "preamble.h"
#include <stdio.h>

int main()
{
	double a= 0;
	double **B = allocate_matrix(12, 1);
	for (a = 1; get_int(a) <= get_int(2); a += 1)
	{
		for (B = 1; get_int(B) <= get_int(3); B += 2)
{
			print(a);
		}
	}
}
