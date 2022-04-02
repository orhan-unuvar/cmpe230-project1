#pragma once
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#define ___EPSILON 0.000000001

double **allocate_matrix(int size_1, int size_2)
{
  double **result = (double **)calloc(size_1, sizeof(double *));
  for (int o = 0; o < size_1; o++)
  {
    result[o] = (double *)calloc(size_2, sizeof(double));
  }

  return result;
}

double **mat_add(double **mat1, double **mat2, int h, int w)
{
  double **result = allocate_matrix(h, w);
  for (int i = 0; i < h; i++)
  {
    for (int j = 0; j < w; j++)
    {
      result[i][j] = mat1[i][j] + mat2[i][j];
    }
  }
  return result;
}

double **mat_sub(double **mat1, double **mat2, int h, int w)
{
  double **result = allocate_matrix(h, w);
  for (int i = 0; i < h; i++)
  {
    for (int j = 0; j < w; j++)
    {
      result[i][j] = mat1[i][j] + -1 * mat2[i][j];
    }
  }
  return result;
}

int get_int(double scalar)
{
  if (scalar - trunc(scalar) < 0.5)
  {
    return (int)scalar;
  }
  return (int)scalar + 1;
}

double **mat_mul(double **mat1, int h1, int w1, double **mat2, int h2, int w2)
{

  double **result = allocate_matrix(h1, w2);

  for (int i = 0; i < h1; i++)
  {
    for (int j = 0; j < w2; j++)
    {
      double sum = 0;
      for (int k = 0; k < w1; k++)
      {
        sum += mat1[i][k] * mat2[k][j];
      }
      result[i][j] = sum;
    }
  }
  return result;
}

double **mat_sca_mul(double **mat, int h, int w, double sca)
{
  for (int i = 0; i < h; i++)
  {
    for (int j = 0; j < w; j++)
    {
      mat[i][j] = mat[i][j] * sca;
    }
  }
  return mat;
}

double **tr(double **mat, int h, int w)
{
  double **result = allocate_matrix(w, h);

  for (int i = 0; i < h; i++)
  {
    for (int j = 0; j < w; j++)
    {
      result[j][i] = mat[i][j];
    }
  }

  return result;
}

double choose(double expr1, double expr2, double expr3, double expr4)
{
  if (fabs(expr1) < ___EPSILON)
  {
    return expr2;
  }
  if (expr1 > 0)
  {
    return expr3;
  }
  if (expr1 < 0)
  {
    return expr4;
  }
  return expr2;
}
void print(double number)
{
  if (fabs(number - round(number)) < ___EPSILON)
  {
    printf("%d\n", (int)round(number));
  }
  else
  {
    printf("%f\n", number);
  }
}
void print_mat(double **matrix, int height, int width)
{
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      print(matrix[i][j]);
    }
  }
}

void printsep()
{
  printf("------------\n");
}

void assign_mat_to_mat(double **matrix, double **assign, int height, int width)
{
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      matrix[i][j] = assign[i][j];
    }
  }
}

void assign_to_mat(double **matrix, int height, int width, ...)
{
  va_list args;
  va_start(args, width);
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      matrix[i][j] = va_arg(args, double);
    }
  }
  va_end(args);
}