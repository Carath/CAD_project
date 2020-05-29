#ifndef TESTING_H
#define TESTING_H


#include "settings.h"


// Normalization of some inputs:
void test_normalize(void);


// Testing the Box-Muller transform, a normal distribution generator:
void test_Box_Muller(void);


// Shuffle demo:
void test_shuffle(void);


// 1 layer neural network for the logical gate 'AND':
void test_AND(void);


// 2 layers neural network for the logical gate 'XOR':
void test_XOR(void);


#endif
