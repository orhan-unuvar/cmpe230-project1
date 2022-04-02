#include "preamble.h"
#include <stdio.h>

int main()
{
	double **A = allocate_matrix(4, 4);
	double **x = allocate_matrix(4, 1);
	double **x_t = allocate_matrix(1, 4);
	double **T = allocate_matrix(1, 1);
	double **mul_1 = allocate_matrix(4, 1);
	double **xy2 = allocate_matrix(4, 1);
	double s= 0;
	assign_to_mat(A, 4, 4, (double)0, (double)1, (double)2, (double)3, (double)4, (double)5, (double)6, (double)7, (double)8, (double)9, (double)1, (double)1, (double)1, (double)2, (double)3, (double)4);
	assign_to_mat(x, 4, 1, (double)1, (double)1, (double)1, (double)1);
	assign_to_mat(xy2, 4, 1, (double)2, (double)1, (double)3, (double)1);
	assign_mat_to_mat(x_t, tr(x, 4, 1), 1, 4);
	assign_mat_to_mat(mul_1, (mat_mul(A, 4, 4, xy2, 4, 1)), 4, 1);
	assign_mat_to_mat(T, (mat_mul(x_t, 1, 4, mul_1, 4, 1)), 1, 1);
	s = T[get_int(1) - 1][get_int(1) - 1];
	print(s);
}
