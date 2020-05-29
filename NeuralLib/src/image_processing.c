#include <stdio.h>
#include <math.h>

#include "image_processing.h"


// Print in the console a grayscale image contained in a 1-dimensional array:
void printGrayscaleImage(const Number *image, int width, int height)
{
	if (image == NULL)
		return;

	for (int row = 0; row < height; ++row)
	{
		for (int col = 0; col < width; ++col)
		{
			const int pos = row * width + col;

			if (number_abs(image[pos]) > EPSILON)
				printf("%5.1f", image[pos]);
			else
				printf("     ");
		}

		printf("\n");
	}

	printf("\n");
}


inline static Number maxMatrix_case(const Number *image, int image_width,
	int kernel_width, int kernel_height, int row, int col)
{
	Number max = -INFINITY;

	int shift = row * image_width + col;

	for (int kernelRow = 0; kernelRow < kernel_height; ++kernelRow)
	{
		for (int kernelCol = 0; kernelCol < kernel_width; ++kernelCol)
			max = number_max(max, image[shift + kernelCol]);

		shift += image_width;
	}

	return max;
}


inline static Number averageMatrix_case(const Number *image, int image_width,
	int kernel_width, int kernel_height, int row, int col)
{
	Number sum = 0;

	const int kernel_pixels = kernel_width * kernel_height;
	int shift = row * image_width + col;

	for (int kernelRow = 0; kernelRow < kernel_height; ++kernelRow)
	{
		for (int kernelCol = 0; kernelCol < kernel_width; ++kernelCol)
			sum += image[shift + kernelCol];

		shift += image_width;
	}

	return sum / kernel_pixels;
}


void pooling(Number *output, const Number *input, int output_width, int output_height,
	int input_width, int input_height, PoolingMode poolmeth)
{
	if (output == NULL || input == NULL)
	{
		printf("\nNULL input or output passed for pooling.\n\n");
		return;
	}

	if (input_width < output_width || input_height < output_height)
	{
		printf("\nInput sizes lower than output sizes! Widths: %d vs %d, heights: %d vs %d.\n\n",
			input_width, output_width, input_height, output_height);
		return;
	}

	if (input_width % output_width != 0 || input_height % output_height != 0)
	{
		printf("\nOutput sizes do not divide input sizes! Widths: %d vs %d, heights: %d vs %d.\n\n",
			input_width, output_width, input_height, output_height);
		return;
	}

	const int kernel_height = input_height / output_height;
	const int kernel_width = input_width / output_width;

	// printf("\nkernel_height = %d\nkernel_width = %d\n", kernel_height, kernel_width);

	for (int row = 0; row < output_height; ++row)
	{
		for (int col = 0; col < output_width; ++col)
		{
			Number caseRowCol;

			if (poolmeth == MAX_POOLING)
				caseRowCol = maxMatrix_case(input, input_width, kernel_width, kernel_height, row * kernel_height, col * kernel_width);

			else // AVERAGE_POOLING
				caseRowCol = averageMatrix_case(input, input_width, kernel_width, kernel_height, row * kernel_height, col * kernel_width);

			output[row * output_width + col] = caseRowCol;
		}
	}
}


// Find the hull of the given image, assuming it has a white background:
void find_hull(const Number *image, int width, int height, int *rowMin, int *rowMax, int *colMin, int *colMax)
{
	if (image == NULL)
	{
		printf("\nCannot find the hull of a NULL image.\n\n");
		return;
	}

	int rowmin = height - 1, rowmax = 0, colmin = width - 1, colmax = 0; // starting from the opposite edges.

	for (int row = 0; row < height; ++row)
	{
		for (int col = 0; col < width; ++col)
		{
			if (image[row * width + col] > EPSILON)
			{
				if (col < colmin)
					colmin = col;

				if (col > colmax)
					colmax = col;

				if (row < rowmin)
					rowmin = row;

				if (row > rowmax)
					rowmax = row;
			}
		}
	}

	*colMin = colmin;
	*colMax = colmax;
	*rowMin = rowmin;
	*rowMax = rowmax;

	// printf("colMin = %d\ncolMax = %d\nrowMin = %d\nrowMax = %d\n", colmin, colmax, rowmin, rowmax);
}


void recenter(Number *dest, const Number *src, int width, int height)
{
	if (dest == NULL || src == NULL)
	{
		printf("\nNULL src or dest passed for recentering.\n\n");
		return;
	}

	const int pixel_number = width * height;

	// Resetting dest:
	for (int k = 0; k < pixel_number; ++k)
		dest[k] = 0;

	int colMin, colMax, rowMin, rowMax;

	find_hull(src, width, height, &rowMin, &rowMax, &colMin, &colMax);

	const int delta_row = (height - rowMax - rowMin - 1) / 2;
	const int delta_col = (width - colMax - colMin - 1) / 2;

	const int shift = delta_row * width + delta_col;

	for (int row = rowMin; row <= rowMax; ++row)
	{
		for (int col = colMin; col <= colMax; ++col)
		{
			const int old_pos = row * width + col;

			if (src[old_pos] > EPSILON)
				dest[old_pos + shift] = src[old_pos];
		}
	}
}
