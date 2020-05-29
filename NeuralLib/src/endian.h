#ifndef ENDIAN_H
#define ENDIAN_H


// The following functions are useful to translate float/double matrices
// from the little-endian format to the big-endian format, and vice versa.


// Swap the bytes order of every float in the given matrix:
void swap_matrix_float(float **matrix, int rows, int cols);


// Swap the bytes order of every double in the given matrix:
void swap_matrix_double(double **matrix, int rows, int cols);


#endif
