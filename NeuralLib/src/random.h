#ifndef RANDOM_H
#define RANDOM_H


#include "settings.h"


// Returns a random number in [min, max[.
Number uniform_random(Number min, Number max);


// Box–Muller transform, generate two 'Number' following the distribution N(0,1):
void Box_Muller(Number *x1, Number *x2);


// Generate two 'Number' following the distribution N(mean, std_dev²):
void gaussian_random(Number *x1, Number *x2, Number mean, Number std_dev);


// Shuffling:


// Fisher–Yates shuffle:
void shuffle(void* *array, int len);


// Readable if every element of array is the address of a Number...
void demo_print(Number* *array, int len);


#endif
