#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H


#include "settings.h"


typedef enum {MAX_POOLING, AVERAGE_POOLING} PoolingMode;


// Print in the console a grayscale image contained in a 1-dimensional array:
void printGrayscaleImage(const Number *image, int width, int height);


void pooling(Number *output, const Number *input, int output_width, int output_height,
	int input_width, int input_height, PoolingMode poolmeth);


// Find the hull of the given image, assuming it has a white background:
void find_hull(const Number *image, int width, int height, int *colMin, int *colMax, int *rowMin, int *rowMax);


void recenter(Number *dest, const Number *src, int width, int height);


#endif
