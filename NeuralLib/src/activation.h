#ifndef ACTIVATION_H
#define ACTIVATION_H


#include "settings.h"


// For performance: don't change the following order without doing it for the switches too.
// Also Softmax needs to be placed at the beginning or end of the list:
#define ACTIVATION_APPLY(FUN)	\
	FUN(Id)						\
	FUN(Heaviside)				\
	FUN(Sigmoid)				\
	FUN(Tanh)					\
	FUN(ReLu)					\
	FUN(LReLu)					\
	FUN(ELu)					\
	FUN(SELu)					\
	FUN(Softmax)				\


#define ADD_TO_LIST(OBJ) OBJ,


// Enum of activation functions:
typedef enum {ACTIVATION_APPLY(ADD_TO_LIST)} Activation;


// Returns the number of supported activation functions:
int getActivationNumber(void);


// Get the name of an Activation:
const char* getActivationString(Activation fun);


// Get an Activation from its string:
Activation getActivation(const char *string);


// Activation function, without softmax:
Number activation(Activation fun, Number x);


// Derivatives, without softmax:
Number der_activation(Activation fun, Number x);


void softmax(Number *dest, const Number *src, int len);


// Updating the last layer's GradSum for the softmax activation with quadratic loss:
void updateGradSumSoftmaxQuadLoss(Number *output_error, const Number *answer, const Number *good_answer, int len);


#endif
