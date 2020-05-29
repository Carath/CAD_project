#ifndef LEARNING_H
#define LEARNING_H


#include "settings.h"
#include "neural_network.h"
#include "inputs.h"
#include "recognition.h"


typedef enum {ON_LINE, MINI_BATCHES, FULL_BATCH} BatchMethod;
typedef enum {QUADRATIC, CROSS_ENTROPY} LossFunction;
typedef enum {UNIFORM, GAUSSIAN} InitRandDistribution;
typedef enum {AUTOMATIC_STD, AUTOMATIC_NORMALIZED, BY_RANGE} InitMethod;
typedef enum {NO_OPT, MOMENTUM, RMSprop, ADAM} Optimizer;
typedef enum {NO_REG, L2} Regularization;
typedef enum {NO_SHUFFLE, SHUFFLE} ShuffleMode;

// Tips:
// ON_LINE is slower than MINI_BATCHES, which performs best when BatchSize >= 16.
// AUTOMATIC_NORMALIZED works better when Init = UNIFORM.


typedef struct
{
	BatchMethod Method;
	LossFunction LossFun;
	InitRandDistribution Random;
	InitMethod Init;
	ShuffleMode Shuffle; // Shuffling by default.
	Optimizer Optim;
	Regularization Reg;
	RecognitionMode RecogEstimates; // ALL_CORRECT by default. This can be changed for better estimates.

	int PrintEstimates;
	int EpochNumber;
	int BatchSize;
	Number BatchSizeMultiplier; // Multiply the batch size by this value after each epoch.
	Number InitRange; // If Init = BY_RANGE, weights are randomly chosen between -InitRange and InitRange.
	Number LearningRate;
	Number LearningRateMultiplier; // Multiply the learning rates by this value after each epoch.

	// Optimizers settings:
	Number MomentumRate;
	Number RMScoeff;
	Number AdamBetaM;
	Number AdamBetaV;

	// Regularization settings:
	Number L2regCoeff;
} LearningParameters;


///////////////////////////////////////////////////////////////////////////////////////
// Learning framework:
///////////////////////////////////////////////////////////////////////////////////////

// Creates some learning settings, initialized with default values:
LearningParameters* initLearningParameters(void);


// Frees the given LearningParameters passed by address, and sets it to NULL.
void freeParameters(LearningParameters **params);


// Make the neural network learn the given inputs, while using the given parameters:
void learn(NeuralNetwork *network, Inputs *inputs, LearningParameters *params);


///////////////////////////////////////////////////////////////////////////////////////
// Recognition:
///////////////////////////////////////////////////////////////////////////////////////


// Compare the network answers to the correct ones, and print the validation level.
void validation(NeuralNetwork *network, Inputs *inputs, RecognitionMode recog);


// Write the network answers in the given inputs.
void prediction(NeuralNetwork *network, Inputs *inputs);


#endif
