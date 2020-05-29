#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> // for memcpy

#include "matrix.h"
#include "random.h"


// Matrix utilities:


Number* createVector(int len) // Every field is initialized to 0.
{
	Number *vector = (Number*) calloc(len, sizeof(Number));

	if (vector == NULL)
		printf("\nImpossible to allocate enough memory for a vector.\n\n");

	return vector;
}


Number** createMatrix(int rows, int cols) // Every field is initialized to 0.
{
	Number **matrix = (Number**) calloc(rows, sizeof(Number*));

	if (matrix == NULL)
	{
		printf("\nImpossible to allocate enough memory for a matrix.\n\n");
		return matrix;
	}

	for (int i = 0; i < rows; ++i)
		matrix[i] = createVector(cols);

	return matrix;
}


// Frees a vector, passed by address, and sets it to NULL:
void freeVector(Number **vector)
{
	if (vector == NULL || *vector == NULL)
		return;

	free(*vector);
	*vector = NULL;
}


// Frees a matrix, passed by address, and sets it to NULL:
void freeMatrix(Number ***matrix, int rows)
{
	if (matrix == NULL || *matrix == NULL)
		return;

	for (int i = 0; i < rows; ++i)
		freeVector(*matrix + i);

	free(*matrix);
	*matrix = NULL;
}


void printVector(const Number *vector, int len)
{
	if (vector == NULL)
		return;

	for (int i = 0; i < len; ++i)
		printf("%8.2f", vector[i]);

	printf("\n");
}


void printFlatMatrix(const Number *matrix, int rows, int cols)
{
	if (matrix == NULL)
		return;

	for (int i = 0; i < rows; ++i)
		printVector(matrix + i * cols, cols);

	printf("\n");
}


void printMatrix(Number* const* matrix, int rows, int cols)
{
	if (matrix == NULL)
		return;

	for (int i = 0; i < rows; ++i)
		printVector(matrix[i], cols);

	printf("\n");
}


inline void resetVector(Number *vector, int len)
{
	for (int i = 0; i < len; ++i)
		vector[i] = 0;
}


inline void resetMatrix(Number **matrix, int rows, int cols)
{
	for (int i = 0; i < rows; ++i)
		resetVector(matrix[i], cols);
}


inline void copyVector(Number *dest, const Number *src, int len)
{
	memcpy(dest, src, len * sizeof(Number));
}


inline void copyMatrix(Number **dest, Number* const* src, int rows, int cols)
{
	for (int i = 0; i < rows; ++i)
		copyVector(dest[i], src[i], cols);
}


// Randomly fill vectors/matrices:


void randomFillVector_uniform(Number *vector, int len, Number bound)
{
	if (vector == NULL)
		return;

	for (int i = 0; i < len; ++i)
		vector[i] = uniform_random(-bound, bound);
}


void randomFillMatrix_uniform(Number **matrix, int rows, int cols, Number bound)
{
	if (matrix == NULL)
		return;

	for (int i = 0; i < rows; ++i)
		randomFillVector_uniform(matrix[i], cols, bound);
}


void randomFillVector_gaussian(Number *vector, int len, Number std_dev)
{
	if (vector == NULL)
		return;

	for (int i = 0; i < len - 1; i += 2)
		gaussian_random(vector + i, vector + i + 1, 0, std_dev);

	if (len % 2 == 0)
		gaussian_random(vector + len - 2, vector + len - 1, 0, std_dev);

	else
	{
		Number buffer;
		gaussian_random(vector + len - 1, &buffer, 0, std_dev); // 'buffer' needed.
	}
}


void randomFillMatrix_gaussian(Number **matrix, int rows, int cols, Number std_dev)
{
	if (matrix == NULL)
		return;

	for (int i = 0; i < rows; ++i)
		randomFillVector_gaussian(matrix[i], cols, std_dev);
}


// Computational part:
// Matrices here are stored as 1 dimensional arrays, for compliance with high performance libraries.


// Naive implementation of: X <- alpha * X.
inline void naive_scal(Number *X, int len, Number alpha)
{
	for (int i = 0; i < len; ++i)
		X[i] *= alpha;
}


// Naive implementation of: X <- X + alpha * Y.
inline void naive_addScal(Number *X, const Number *Y, int len, Number alpha)
{
	for (int i = 0; i < len; ++i)
		X[i] += alpha * Y[i];
}


// Naive implementation of: C <- op(A) * op(B). For each case, the best loop order has been chosen:
void naive_matrix_multiply(TransposeOptions optA, TransposeOptions optB, const Number *A, const Number *B, Number *C,
	int rows_op_A, int cols_op_B, int cols_op_A)
{
	// Resetting:
	for (int i = 0; i < rows_op_A * cols_op_B; ++i)
		C[i] = 0;

	if (optA == NoTrans && optB == NoTrans)
	{
		for (int i = 0; i < rows_op_A; ++i)
		{
			for (int k = 0; k < cols_op_A; ++k)
			{
				for (int j = 0; j < cols_op_B; ++j)
					C[i * cols_op_B + j] += A[i * cols_op_A + k] * B[k * cols_op_B + j];
			}
		}
	}

	else if (optA == NoTrans && optB == Trans)
	{
		for (int i = 0; i < rows_op_A; ++i)
		{
			for (int j = 0; j < cols_op_B; ++j)
			{
				for (int k = 0; k < cols_op_A; ++k)
					C[i * cols_op_B + j] += A[i * cols_op_A + k] * B[j * cols_op_A + k];
			}
		}
	}

	else if (optA == Trans && optB == NoTrans)
	{
		for (int k = 0; k < cols_op_A; ++k)
		{
			for (int i = 0; i < rows_op_A; ++i)
			{
				for (int j = 0; j < cols_op_B; ++j)
					C[i * cols_op_B + j] += A[k * rows_op_A + i] * B[k * cols_op_B + j];
			}
		}
	}

	else // optA == Trans && optB == Trans
	{
		for (int i = 0; i < rows_op_A; ++i)
		{
			for (int j = 0; j < cols_op_B; ++j)
			{
				for (int k = 0; k < cols_op_A; ++k)
					C[i * cols_op_B + j] += A[k * rows_op_A + i] * B[j * cols_op_A + k];
			}
		}
	}
}
