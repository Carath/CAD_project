#include <stdio.h>
#include <stdlib.h>

#include "learning_phase.h"
#include "learning_dataset.h"
#include "parsing.h"


// Creates a neural network, makes it learn the expanded dataset, and then saves it.
void learningPhase(void)
{
	printf("\n\n-> Starting some deep learning!\n");

	int learning_sampleNumber = 100000;

	Inputs *learning_dataset = createDataset(learning_sampleNumber);

	printInputs(learning_dataset, INFOS);

	int input_size = getSymptomNumber();
	int answer_size = getIllnessNumber();

	// Creating a neural network:

	int max_batch_size = 64;
	int NeuronsNumberArray[] = {256, 150, answer_size};
	Activation funArray[] = {ReLu, ReLu, Softmax};

	int layer_number = ARRAYS_COMPARE_LENGTH(NeuronsNumberArray, funArray);

	NeuralNetwork *network = createNetwork(input_size, layer_number, NeuronsNumberArray, funArray, max_batch_size);

	printNetwork(network, INFOS);

	// Setting the learning parameters:

	LearningParameters *params = initLearningParameters();

	params -> Method = MINI_BATCHES;
	params -> BatchSize = max_batch_size;
	params -> EpochNumber = 5;
	params -> LearningRate = 0.005;
	params -> LearningRateMultiplier = 0.9;
	params -> RecogEstimates = MAX_CORRECT;

	learn(network, learning_dataset, params);

	// printNetwork(network, ALL);

	printf("\nValidation:\n");

	int validation_sampleNumber = 10000;

	Inputs *validation_dataset = createDataset(validation_sampleNumber);

	validation(network, validation_dataset, MAX_CORRECT);

	// Saving and loading:

	saveNetwork(network, NEURAL_NET_DIR_PATH);

	// Freeing everything:

	freeDatasetAssets();

	freeInputs(&validation_dataset);
	freeParameters(&params);
	freeNetwork(&network);
	freeInputs(&learning_dataset);
}
