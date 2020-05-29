#include <stdint.h> // for standard, fixed size int types.
#include <byteswap.h>

#include "endian.h"


typedef union
{
	float F;
	uint32_t I;
} Data32;


typedef union
{
	double F;
	uint64_t I;
} Data64;


// Swap the bytes order of every float in the given matrix:
void swap_matrix_float(float **matrix, int rows, int cols)
{
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
			Data32 data;

			data.F = matrix[i][j];
			data.I = __bswap_32(data.I);
			matrix[i][j] = data.F;
		}
	}
}


// Swap the bytes order of every double in the given matrix:
void swap_matrix_double(double **matrix, int rows, int cols)
{
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
			Data64 data;

			data.F = matrix[i][j];
			data.I = __bswap_64(data.I);
			matrix[i][j] = data.F;
		}
	}
}
