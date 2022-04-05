#include "preamble.h"
#include <stdio.h>

int main()
{
	double **_A = allocate_matrix(4, 4);
	double **_x = allocate_matrix(4, 1);
	double **_x_t = allocate_matrix(1, 4);
	double **_T = allocate_matrix(1, 1);
	double **_mul_1 = allocate_matrix(4, 1);
	double **_xy2 = allocate_matrix(4, 1);
	double _s= 0;
	assign_to_mat(_A, 4, 4, (double)0, (double)1, (double)2, (double)3, (double)4, (double)5, (double)6, (double)7, (double)8, (double)9, (double)1, (double)1, (double)1, (double)2, (double)3, (double)4);
	assign_to_mat(_x, 4, 1, (double)1, (double)1, (double)1, (double)1);
	assign_to_mat(_xy2, 4, 1, (double)2, (double)1, (double)3, (double)1);
	assign_mat_to_mat(_x_t, tr(_x, 4, 1), 1, 4);
	assign_mat_to_mat(_mul_1, (mat_mul(_A, 4, 4, _xy2, 4, 1)), 4, 1);
	assign_mat_to_mat(_T, (mat_mul(_x_t, 1, 4, _mul_1, 4, 1)), 1, 1);
	_s = _T[get_int(1) - 1][get_int(1) - 1];
	print(_s);
}
