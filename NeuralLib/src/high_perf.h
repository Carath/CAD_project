// Framework for using a CBLAS high performance library:

// General functions defined:

// copy(dest, src, len)

// scal(X, len, alpha): X <- alpha * X

// addScal(X, Y, len, alpha): X <- X + alpha * Y

// matrix_multiply(optA, optB, A, B, C, rows_op_A, cols_op_B, cols_op_A): C <- op(A) * op(B)


#ifndef HIGH_PERF_H
#define HIGH_PERF_H


// To use a CBLAS library, e.g OpenBLAS:
#if defined _OPENBLAS
	#define CBLAS
#endif


#include "settings.h" // For 'Number' definition.


#if defined CBLAS

	#pragma message "OpenBLAS is being used."

	#include <cblas.h>

	#define ORDER CblasRowMajor // Do not modify.
	#define CONVERT_CST 111 // Do not modify, used for converting 'Trans' to 'CblasTrans'.


	#define leadingDimension(rows, cols) \
		(ORDER == CblasRowMajor ? cols : rows)

	// float/double implementation:

	#if defined _FLOAT

		#define _copy(dest, src, len) \
			cblas_scopy(len, src, 1, dest, 1)

		#define _scal(X, len, alpha) \
			cblas_sscal(len, alpha, X, 1)

		#define _axpy(X, Y, len, alpha) \
			cblas_saxpy(len, alpha, Y, 1, X, 1)

		#define _gemm(ORDER, optA, optB, rows_op_A, cols_op_B, cols_op_A, alpha, A, lda, B, ldb, beta, C, ldc) \
			cblas_sgemm(ORDER, optA, optB, rows_op_A, cols_op_B, cols_op_A, alpha, A, lda, B, ldb, beta, C, ldc)


	#elif defined _DOUBLE

		#define _copy(dest, src, len) \
			cblas_dcopy(len, src, 1, dest, 1)

		#define _scal(X, len, alpha) \
			cblas_dscal(len, alpha, X, 1)

		#define _axpy(X, Y, len, alpha) \
			cblas_daxpy(len, alpha, Y, 1, X, 1)

		#define _gemm(ORDER, optA, optB, rows_op_A, cols_op_B, cols_op_A, alpha, A, lda, B, ldb, beta, C, ldc) \
			cblas_dgemm(ORDER, optA, optB, rows_op_A, cols_op_B, cols_op_A, alpha, A, lda, B, ldb, beta, C, ldc)
	#endif


	// CBLAS framework:

	#define copy(dest, src, len) \
		copyVector(dest, src, len)
	// Not using CBLAS _copy here, since memcpy seems to be slightly faster...


	#define scal(X, len, alpha) \
		_scal(X, len, alpha)


	#define addScal(X, Y, len, alpha) \
		_axpy(X, Y, len, alpha)


	#define matrix_multiply(optA, optB, A, B, C, rows_op_A, cols_op_B, cols_op_A) \
	{ \
		CBLAS_TRANSPOSE cblas_optA = optA + CONVERT_CST; \
		CBLAS_TRANSPOSE cblas_optB = optB + CONVERT_CST; \
		int lda, ldb, ldc; \
		if (ORDER == CblasRowMajor) \
		{ \
			lda = cblas_optA == CblasNoTrans ? cols_op_A : rows_op_A; \
			ldb = cblas_optB == CblasNoTrans ? cols_op_B : cols_op_A; \
			ldc = cols_op_B; \
		} \
		else \
		{ \
			lda = cblas_optA == CblasNoTrans ? rows_op_A : cols_op_A; \
			ldb = cblas_optB == CblasNoTrans ? cols_op_A : cols_op_B; \
			ldc = rows_op_A; \
		} \
		_gemm(ORDER, cblas_optA, cblas_optB, rows_op_A, cols_op_B, cols_op_A, 1, A, lda, B, ldb, 0, C, ldc); \
	}


#else // Naive implementations:

	#pragma message "No high performance library is being used, expect slow learning."

	#define copy(dest, src, len) \
		copyVector(dest, src, len)

	#define scal(X, len, alpha) \
		naive_scal(X, len, alpha)

	#define addScal(X, Y, len, alpha) \
		naive_addScal(X, Y, len, alpha)

	#define matrix_multiply(optA, optB, A, B, C, rows_op_A, cols_op_B, cols_op_A) \
		naive_matrix_multiply(optA, optB, A, B, C, rows_op_A, cols_op_B, cols_op_A)
#endif


#endif
