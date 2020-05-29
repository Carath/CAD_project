#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H


#include "settings.h"
#include "activation.h"


typedef struct
{
	const int InputSize;
	const int NeuronsNumber; // i.e output size.
	Activation Fun;

	Number *Input;		// Pointer to the previous output if not the first layer.
	Number *Net;		// (InputSize + 1) * NeuronsNumber (last row: biases, other rows: weights)
	Number *Sum;		// MaxBatchSize * NeuronsNumber
	Number *GradSum;	// MaxBatchSize * NeuronsNumber
	Number *Output;		// MaxBatchSize * (NeuronsNumber + 1)
} NeuronLayer;


typedef struct
{
	int HasLearned;
	const int LayersNumber;
	const int MaxBatchSize; // The greater MaxBatchSize is, the faster the computations may be.
	NeuronLayer *Layers;	// size: LayersNumber
} NeuralNetwork;


// 'NeuronsNumberArray' and 'funArray' are of size 'LayersNumber'.
NeuralNetwork* createNetwork(int InputSize, int LayersNumber, int *NeuronsNumberArray, Activation *funArray, int MaxBatchSize);


// Frees the given neural network passed by address, and sets it to NULL.
void freeNetwork(NeuralNetwork **network);


int network_inputSize(const NeuralNetwork *network);


int network_outputSize(const NeuralNetwork *network);


NeuronLayer* network_outputLayer(const NeuralNetwork *network);


// Printing a network's content:
void printNetwork(const NeuralNetwork *network, PrintOption opt);


// Saving a neural network. N.B: MaxBatchSize doesn't need to be saved, for its value is arbitrary:
int saveNetwork(const NeuralNetwork *network, const char *foldername);


// Loading a neural network:
NeuralNetwork* loadNetwork(const char *foldername, int MaxBatchSize);


#endif
