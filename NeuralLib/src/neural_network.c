#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "neural_network.h"
#include "matrix.h"
#include "saving.h"


static const char* LearningStateAnswer[] = {"no", "yes"};


// For initializing a layer or a network:
static void initLayer(NeuronLayer *layer, int InputSize, int NeuronsNumber, Activation fun, int MaxBatchSize)
{
	*(int*) &(layer -> InputSize) = InputSize;
	*(int*) &(layer -> NeuronsNumber) = NeuronsNumber;
	layer -> Fun = fun;

	layer -> Input = NULL; // Pointer to the previous output if not the first layer.
	layer -> Net = createVector((InputSize + 1) * NeuronsNumber);
	layer -> Sum = createVector(MaxBatchSize * NeuronsNumber);
	layer -> GradSum = createVector(MaxBatchSize * NeuronsNumber);
	layer -> Output = createVector(MaxBatchSize * (NeuronsNumber + 1));

	// Filling with 1 every last column of the layer Output:

	for (int b = 0; b < MaxBatchSize; ++b) // MaxBatchSize needed, for validation/prediction optimizations!
		layer -> Output[b * (layer -> NeuronsNumber + 1) + layer -> NeuronsNumber] = 1;
}


// 'NeuronsNumberArray' and 'funArray' are of size 'LayersNumber'.
NeuralNetwork* createNetwork(int InputSize, int LayersNumber, int *NeuronsNumberArray, Activation *funArray, int MaxBatchSize)
{
	NeuralNetwork *network = (NeuralNetwork*) calloc(1, sizeof(NeuralNetwork));

	network -> HasLearned = 0;
	*(int*) &(network -> LayersNumber) = LayersNumber;
	*(int*) &(network -> MaxBatchSize) = MaxBatchSize;

	network -> Layers = (NeuronLayer*) calloc(LayersNumber, sizeof(NeuronLayer));

	NeuronLayer *layer = network -> Layers;

	// Initializing the first layer:

	initLayer(layer, InputSize, NeuronsNumberArray[0], funArray[0], MaxBatchSize);

	layer -> Input = createVector(MaxBatchSize * (InputSize + 1));

	// Filling with 1 the last column of the first Input:

	for (int b = 0; b < MaxBatchSize; ++b) // MaxBatchSize needed, for validation/prediction optimizations!
		layer -> Input[b * (layer -> InputSize + 1) + layer -> InputSize] = 1;

	// Initializing the other layers:

	for (int l = 1; l < LayersNumber; ++l)
	{
		++layer;

		initLayer(layer, NeuronsNumberArray[l-1], NeuronsNumberArray[l], funArray[l], MaxBatchSize);

		layer -> Input = (layer - 1) -> Output; // Previous output.
	}

	return network;
}


// Frees the given neural network passed by address, and sets it to NULL.
void freeNetwork(NeuralNetwork **network)
{
	if (network == NULL || *network == NULL)
		return;

	NeuronLayer *layer = (*network) -> Layers;

	free(layer -> Input); // freeing the first input.

	for (int l = 0; l < (*network) -> LayersNumber; ++l)
	{
		// Useless to free from memory 'layer -> Input' has it is only pointing to addresses.
		free(layer -> Net);
		free(layer -> Sum);
		free(layer -> GradSum);
		free(layer -> Output);

		++layer;
	}

	free((*network) -> Layers);
	free(*network);
	*network = NULL;
}


int network_inputSize(const NeuralNetwork *network)
{
	if (network == NULL)
	{
		printf("\nCannot get the input size of a NULL network.\n\n");
		exit(EXIT_FAILURE);
	}

	NeuronLayer *layer = network -> Layers;

	return layer -> InputSize;
}


int network_outputSize(const NeuralNetwork *network)
{
	if (network == NULL)
	{
		printf("\nCannot get the output size of a NULL network.\n\n");
		exit(EXIT_FAILURE);
	}

	return network_outputLayer(network) -> NeuronsNumber;
}


NeuronLayer* network_outputLayer(const NeuralNetwork *network)
{
	if (network == NULL)
	{
		printf("\nCannot get the output layer of a NULL network.\n\n");
		exit(EXIT_FAILURE);
	}

	return network -> Layers + network -> LayersNumber - 1;
}


// Printing a network's content:
void printNetwork(const NeuralNetwork *network, PrintOption opt)
{
	if (network == NULL)
	{
		printf("\nCannot print informations of a NULL network.\n\n");
		return;
	}

	printf("\nThis network has learned: %s, and has %d neuron layer(s):\n\n",
		LearningStateAnswer[network -> HasLearned], network -> LayersNumber);

	NeuronLayer *layer = network -> Layers;

	for (int l = 0; l < network -> LayersNumber; ++l)
	{
		printf("\nLayer °%d:\n\n -> Input size: %d, neurons number: %d, activation: %s\n\n",
			l + 1, layer -> InputSize, layer -> NeuronsNumber, getActivationString(layer -> Fun));

		if (opt == ALL)
		{
			printf(" -> Net (last row: biases, other rows: weights):\n\n");

			printFlatMatrix(layer -> Net, layer -> InputSize + 1, layer -> NeuronsNumber);
		}

		++layer;
	}
}


// Saving a neural network. N.B: MaxBatchSize doesn't need to be saved, for its value is arbitrary:
int saveNetwork(const NeuralNetwork *network, const char *foldername)
{
	if (network == NULL)
	{
		printf("\nCannot save a NULL network.\n\n");
		return 0;
	}

	if (foldername == NULL)
	{
		printf("\nNULL foldername.\n\n");
		return 0;
	}

	if (network -> HasLearned != 1)
	{
		printf("\nNo learning has been done, there is nothing so save.\n\n");
		return 0;
	}

	// Creating a folder in which the network will be saved:

	createFolder(foldername);

	// Creating a text file with the network's readable data:

	char infos_filename[MAX_PATH_LENGTH];
	sprintf(infos_filename, "%s/infos.txt", foldername);

	FILE *infos_file = fopen(infos_filename, "w");

	if (infos_file == NULL)
		exitFileError(infos_file, "Cannot create the file", infos_filename);

	fprintf(infos_file, "Neural network.\nSize of Number: %ld bytes\nInput size: %d\nNumber of layers: %d\n",
		sizeof(Number), network_inputSize(network), network -> LayersNumber);

	// Saving each layer:

	NeuronLayer *layer = network -> Layers;

	char net_filename[MAX_PATH_LENGTH];

	for (int l = 0; l < network -> LayersNumber; ++l)
	{
		// Saving the layer's readable data:

		fprintf(infos_file, "Layer °%d -> number of neurons: %d, activation: %s\n",
			l + 1, layer -> NeuronsNumber, getActivationString(layer -> Fun));

		// Saving the net of this layer:

		sprintf(net_filename, "%s/net_%d.bin", foldername, l);

		save_flat_matrix(layer -> Net, layer -> InputSize + 1, layer -> NeuronsNumber, net_filename);

		++layer;
	}

	fclose(infos_file);

	printf("\nThe given neural network has been successfully saved in '%s'.\n\n", foldername);

	return 1;
}


NeuralNetwork* loadNetwork(const char *foldername, int MaxBatchSize)
{
	if (foldername == NULL)
	{
		printf("\nNULL foldername.\n\n");
		exit(EXIT_FAILURE);
	}

	char infos_filename[MAX_PATH_LENGTH];
	char error_message[MAX_PATH_LENGTH];

	sprintf(infos_filename, "%s/infos.txt", foldername);

	// Reading the text file with the network's readable data:

	FILE *infos_file = fopen(infos_filename, "r");

	if (infos_file == NULL)
		exitFileError(infos_file, "Cannot read the file", infos_filename);

	skip(infos_file, "Neural network.\nSize of Number: ");

	int size_of_number, input_size, layers_number;

	if (fscanf(infos_file, "%d", &size_of_number) != 1 || size_of_number != sizeof(Number))
	{
		sprintf(error_message, "Not found or incompatible 'Number' size (%d vs %ld) in", size_of_number, sizeof(Number));
		exitFileError(infos_file, error_message, infos_filename);
	}

	skip(infos_file, " bytes\nInput size: ");

	if (fscanf(infos_file, "%d", &input_size) != 1)
		exitFileError(infos_file, "Input size couldn't be retrieved from", infos_filename);

	skip(infos_file, "\nNumber of layers: ");

	if (fscanf(infos_file, "%d\n", &layers_number) != 1)
		exitFileError(infos_file, "Layers number couldn't be retrieved from", infos_filename);

	int *NeuronsNumberArray = (int*) calloc(layers_number, sizeof(int));

	Activation *funArray = (Activation*) calloc(layers_number, sizeof(Activation));

	int layer_index;
	char layer_activation[MAX_PATH_LENGTH];

	for (int l = 0; l < layers_number; ++l)
	{
		skip(infos_file, "Layer °");

		if (fscanf(infos_file, "%d", &layer_index) != 1)
			exitFileError(infos_file, "Layer index couldn't be retrieved from", infos_filename);

		skip(infos_file, " -> number of neurons: ");

		if (fscanf(infos_file, "%d", NeuronsNumberArray + l) != 1)
		{
			sprintf(error_message, "Neurons number from layer °%d couldn't be retrieved from", layer_index);
			exitFileError(infos_file, error_message, infos_filename);
		}

		skip(infos_file, ", activation: ");

		if (fscanf(infos_file, "%s\n", layer_activation) != 1)
		{
			sprintf(error_message, "layer activation from layer °%d couldn't be retrieved from", layer_index);
			exitFileError(infos_file, error_message, infos_filename);
		}

		funArray[l] = getActivation(layer_activation);
		// No need to reset 'layer_activation', as anything after '\0' is ignored.
	}

	fclose(infos_file);

	// Creation of the network:

	NeuralNetwork *network = createNetwork(input_size, layers_number, NeuronsNumberArray, funArray, MaxBatchSize);

	free(NeuronsNumberArray);
	free(funArray);

	network -> HasLearned = 1; // The network would not have been saved if it had not learned a thing.

	// Reading each layer weigths and bias:

	NeuronLayer *layer = network -> Layers;

	// Loading the net of each layer:

	char net_filename[MAX_PATH_LENGTH];

	for (int l = 0; l < layers_number; ++l)
	{
		sprintf(net_filename, "%s/net_%d.bin", foldername, l);

		load_toFlatMatrix(layer -> Net, layer -> InputSize + 1, layer -> NeuronsNumber, net_filename);

		++layer;
	}

	printf("\nThe given neural network has been successfully loaded from '%s'.\n\n", foldername);

	return network;
}
