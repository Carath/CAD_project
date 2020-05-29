#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "learning.h"
#include "matrix.h"
#include "activation.h"
#include "random.h"
#include "benchmarking.h"
#include "high_perf.h"


static int Warning_softmax = 1; // Used to only print the warning once.


///////////////////////////////////////////////////////////////////////////////////////
// Prototypes of static functions:
///////////////////////////////////////////////////////////////////////////////////////


// Predict the answers of the given inputs, and do the following depending on the value of 'type':
// VALIDATION -> compare the network answers to the correct ones, and print the validation level.
// PREDICTION -> write the network answers in the given inputs.
static void recognitionFramework(NeuralNetwork *network, Inputs *inputs, RecogType type, RecognitionMode recog);


// Creating a buffer for the networks's nets:
static Number** createNetworkBuffer(NeuralNetwork *network);


// Setting free the network buffer:
static void freeNetworkBuffer(NeuralNetwork *network, Number **buffer);


// Propagating the questions from the batch forward, and returning the network's answers:
static Number* propagation(NeuralNetwork *network, Number **batch_questions, int batch_size);


// Backpropagation: recursively update each 'GradSum'.
// A propagation pass is necessary before doing the backpropagation:
static void backpropagation(NeuralNetwork *network, Number **batch_good_answers, LearningParameters *params, int batch_size);


// Update 'grad_buffer' for the whole batch:
static void updateGradBufferBatch(NeuralNetwork *network, Number **grad_buffer, int batch_size);


static void gradientDescent(NeuralNetwork *network, Inputs *inputs, LearningParameters *params);


static void updateNetwork(NeuralNetwork *network, Number **grad_buffer, Number **M_buffer,	Number **V_buffer,
	LearningParameters *params, int step_number);


///////////////////////////////////////////////////////////////////////////////////////
// Learning framework:
///////////////////////////////////////////////////////////////////////////////////////


// Creates some learning settings, initialized with default values:
LearningParameters* initLearningParameters(void)
{
	LearningParameters *params = (LearningParameters*) calloc(1, sizeof(LearningParameters));

	params -> Method = MINI_BATCHES;
	params -> LossFun = CROSS_ENTROPY;
	params -> Random = GAUSSIAN;
	params -> Init = AUTOMATIC_STD;
	params -> Shuffle = SHUFFLE;
	params -> Optim = NO_OPT;
	params -> Reg = NO_REG;
	params -> RecogEstimates = ALL_CORRECT;

	params -> PrintEstimates = 1;
	params -> EpochNumber = 1;
	params -> BatchSize = 32;
	params -> BatchSizeMultiplier = 1.;
	params -> InitRange = 0.01;
	params -> LearningRate = 0.01;
	params -> LearningRateMultiplier = 1.;

	params -> MomentumRate = 0.5;
	params -> RMScoeff = 0.9;
	params -> AdamBetaM = 0.9;
	params -> AdamBetaV = 0.999;

	params -> L2regCoeff = 0.0001;

	return params;
}


// Frees the given LearningParameters passed by address, and sets it to NULL.
void freeParameters(LearningParameters **params)
{
	if (params == NULL || *params == NULL)
		return;

	free(*params);
	*params = NULL;
}


// Make the neural network learn the given inputs, while using the given parameters:
void learn(NeuralNetwork *network, Inputs *inputs, LearningParameters *params)
{
	double time_1 = get_time();

	if (network == NULL || inputs == NULL || params == NULL)
	{
		printf("\nInvalid arguments passed to the learn function.\n\n");
		return;
	}

	if (inputs -> InputNumber <= 0 || inputs -> Questions == NULL || inputs -> Answers == NULL)
	{
		printf("\nNothing to learn.\n\n");
		return;
	}

	const int net_input_size = network_inputSize(network), net_output_size = network_outputSize(network);

	if (net_input_size != inputs -> QuestionsSize || net_output_size != inputs -> AnswersSize)
	{
		printf("\nImcompatible sizes between network and inputs! Questions size: %d vs %d, answers size: %d vs %d.\n\n",
			net_input_size, inputs -> QuestionsSize, net_output_size, inputs -> AnswersSize);
		return;
	}

	// Batch size management:

	if (params -> Method == ON_LINE)
		params -> BatchSize = 1;

	else if (params -> Method == FULL_BATCH)
		params -> BatchSize = inputs -> InputNumber;

	int batch_size_bound = MIN(network -> MaxBatchSize, inputs -> InputNumber);

	if (params -> BatchSize > batch_size_bound)
	{
		printf("\nLearning batch size changed to 'batch_size_bound': %d.\n\n", batch_size_bound);
		params -> BatchSize = batch_size_bound;
	}

	// Making sure the settings are coherent enough:

	NeuronLayer *layer = network_outputLayer(network);

	if (net_output_size == 1 && layer -> Fun == Softmax)
	{
		layer -> Fun = Sigmoid;
		printf("\nOutput of dimension 1: activation changed to 'Sigmoid'.\n");
	}

	if (params -> Shuffle == SHUFFLE)
		printf("\nRemark: the inputs will be shuffled during the learning phase.\nThis can be turned off via 'params -> Shuffle'.\n");

	printf("\n-> Starting to learn the %d given inputs:\n", inputs -> InputNumber);

	if (network -> HasLearned == 0) // First learning.
	{
		layer = network -> Layers;

		for (int l = 0; l < network -> LayersNumber; ++l)
		{
			// Setting a range for the next step:

			Number range;

			if (params -> Init == AUTOMATIC_STD)
				range = number_sqrt(1. / layer -> InputSize);

			else if (params -> Init == AUTOMATIC_NORMALIZED)
				range = number_sqrt(6. / (layer -> InputSize + layer -> NeuronsNumber));

			else // BY_RANGE
				range = params -> InitRange;

			// Initializing each Net randomly (N.B: biases can be set to 0, it doesn't really matter):

			if (params -> Random == UNIFORM)
				randomFillVector_uniform(layer -> Net, (layer -> InputSize + 1) * layer -> NeuronsNumber, range);

			else // GAUSSIAN
				randomFillVector_gaussian(layer -> Net, (layer -> InputSize + 1) * layer -> NeuronsNumber, range);

			++layer;
		}
	}

	gradientDescent(network, inputs, params);

	network -> HasLearned = 1;

	double time_2 = get_time();

	printf("\n\n-> Learning done (%d epochs). Time elapsed: %.2f s\n\n", params -> EpochNumber, time_2 - time_1);
}


///////////////////////////////////////////////////////////////////////////////////////
// Recognition:
///////////////////////////////////////////////////////////////////////////////////////


// Compare the network answers to the correct ones, and print the validation level.
inline void validation(NeuralNetwork *network, Inputs *inputs, RecognitionMode recog)
{
	recognitionFramework(network, inputs, VALIDATION, recog);
}

// Write the network answers in the given inputs.
inline void prediction(NeuralNetwork *network, Inputs *inputs)
{
	recognitionFramework(network, inputs, PREDICTION, ALL_CORRECT); // 'ALL_CORRECT' is arbitrary, it has no effect here.
}


// Predict the answers of the given inputs, and do the following depending on the value of 'type':
// VALIDATION -> compare the network answers to the correct ones, and print the validation level.
// PREDICTION -> write the network answers in the given inputs.
static void recognitionFramework(NeuralNetwork *network, Inputs *inputs, RecogType type, RecognitionMode recog)
{
	if (network == NULL || inputs == NULL)
	{
		printf("\nInvalid arguments passed for validation/prediction.\n\n");
		return;
	}

	if (inputs -> InputNumber <= 0 || inputs -> Questions == NULL || (inputs -> Answers == NULL && type == VALIDATION))
	{
		printf("\nNothing to recognize.\n\n");
		return;
	}

	const int net_input_size = network_inputSize(network), net_output_size = network_outputSize(network);

	if (net_input_size != inputs -> QuestionsSize || net_output_size != inputs -> AnswersSize)
	{
		printf("\nImcompatible sizes between network and inputs! Questions size: %d vs %d, answers size: %d vs %d.\n\n",
			net_input_size, inputs -> QuestionsSize, net_output_size, inputs -> AnswersSize);
		return;
	}

	if (recog == MAX_VALUE && net_output_size == 1)
	{
		recog = MAX_CORRECT;
		printf("\nOutput of dimension 1, recognition mode changed to: 'MAX_CORRECT'.\n");
	}

	if (type == VALIDATION && recog == ALL_CORRECT && network_outputLayer(network) -> Fun == Softmax && Warning_softmax)
	{
		printf("\nWarning: 'Softmax' activation is not suited for non-mutually exclusive classification problems,\n");
		printf("but is used during validation along 'ALL_CORRECT' recognition mode...\n");

		Warning_softmax = 0; // Not printing the warning anymore.
	}

	if (type == PREDICTION && inputs -> Answers == NULL) // Do not exit if answers already exist, as one may want to rewrite them!
	{
		// Allocate enough space to predict answers.
		inputs -> Answers = createMatrix(inputs -> InputNumber, inputs -> AnswersSize);
	}

	if (network -> HasLearned == 0)
		printf("\nNo learning has been done...\n");

	// The recognition may start:

	int batch_size_bound = MIN(network -> MaxBatchSize, inputs -> InputNumber); // for maximum speed!
	int remainder = (inputs -> InputNumber) % batch_size_bound;
	int current_batch_size = remainder == 0 ? batch_size_bound : remainder;
	int batch_index = 0, sum = 0;

	while (batch_index < inputs -> InputNumber)
	{
		Number **batch_questions = inputs -> Questions + batch_index;
		Number **batch_goodOrToFill_answers = inputs -> Answers + batch_index;

		Number *batch_answers = propagation(network, batch_questions, current_batch_size);

		for (int b = 0; b < current_batch_size; ++b)
		{
			sum += recog_method(batch_goodOrToFill_answers[b], batch_answers + b * (inputs -> AnswersSize + 1),
				inputs -> AnswersSize, recog, type);
		}

		batch_index += current_batch_size;
		current_batch_size = batch_size_bound; // only 'batch_size_bound' after the first pass.
	}

	if (type == VALIDATION)
	{
		float validation_level = 100. * sum / inputs -> InputNumber;

		printf("\n-> Recognition level: %.2f %%\n\n", validation_level);
	}

	// else // PREDICTION
	// 	printf("\n-> Prediction done.\n\n");
}


///////////////////////////////////////////////////////////////////////////////////////
// Network buffer management:
///////////////////////////////////////////////////////////////////////////////////////


// Creating a buffer for the networks's nets:
static Number** createNetworkBuffer(NeuralNetwork *network)
{
	Number **buffer = (Number**) calloc(network -> LayersNumber, sizeof(Number*));

	NeuronLayer *layer = network -> Layers;

	for (int l = 0; l < network -> LayersNumber; ++l)
	{
		buffer[l] = createVector((layer -> InputSize + 1) * layer -> NeuronsNumber);

		++layer;
	}

	return buffer;
}


// Setting free the network buffer:
static inline void freeNetworkBuffer(NeuralNetwork *network, Number **buffer)
{
	freeMatrix(&buffer, network -> LayersNumber); // Not exactly a 2D matrix, but will work nonetheless.
}


///////////////////////////////////////////////////////////////////////////////////////
// Propagation and backpropagation functions:
///////////////////////////////////////////////////////////////////////////////////////


// Propagating the questions from the batch forward, and returning the network's answers:
static Number* propagation(NeuralNetwork *network, Number **batch_questions, int batch_size)
{
	NeuronLayer *layer = network -> Layers;

	// Copying the questions from the batch to the first network's Input:

	for (int b = 0; b < batch_size; ++b)
		copy(layer -> Input + b * (layer -> InputSize + 1), batch_questions[b], layer -> InputSize);

	// Propagating the questions through each layers:

	for (int l = 0; l < network -> LayersNumber; ++l)
	{
		// For each layer: Sum = Input * Net

		matrix_multiply(NoTrans, NoTrans, layer -> Input, layer -> Net, layer -> Sum,
			batch_size, layer -> NeuronsNumber, layer -> InputSize + 1);

		// Activation:

		for (int b = 0; b < batch_size; ++b)
		{
			int gradsum_pos = b * layer -> NeuronsNumber;
			int output_pos = gradsum_pos + b; // = b * (layer -> NeuronsNumber + 1)

			if (layer -> Fun == Softmax)
				softmax(layer -> Output + output_pos, layer -> Sum + gradsum_pos, layer -> NeuronsNumber);
			else
			{
				for (int j = 0; j < layer -> NeuronsNumber; ++j)
					layer -> Output[output_pos + j] = activation(layer -> Fun, layer -> Sum[gradsum_pos + j]);
			}
		}

		++layer;
	}

	// Returning the answers:

	return (layer - 1) -> Output;
}


// Backpropagation: recursively update each 'GradSum'.
// A propagation pass is necessary before doing the backpropagation:
static void backpropagation(NeuralNetwork *network, Number **batch_good_answers, LearningParameters *params, int batch_size)
{
	NeuronLayer *layer = network_outputLayer(network);

	// Output layer:

	for (int b = 0; b < batch_size; ++b)
	{
		int gradsum_pos = b * layer -> NeuronsNumber;
		int output_pos = gradsum_pos + b; // = b * (layer -> NeuronsNumber + 1)

		if (params -> LossFun == QUADRATIC)
		{
			if (layer -> Fun == Softmax)
				updateGradSumSoftmaxQuadLoss(layer -> GradSum + gradsum_pos, layer -> Output + output_pos,
					batch_good_answers[b], layer -> NeuronsNumber);
			else
			{
				for (int j = 0; j < layer -> NeuronsNumber; ++j)
					layer -> GradSum[gradsum_pos + j] = der_activation(layer -> Fun, layer -> Sum[output_pos + j]) *
						(layer -> Output[output_pos + j] - batch_good_answers[b][j]);
			}
		}

		else // CROSS_ENTROPY
		{
			for (int j = 0; j < layer -> NeuronsNumber; ++j)
				layer -> GradSum[gradsum_pos + j] = layer -> Output[output_pos + j] - batch_good_answers[b][j];
		}
	}

	// Hidden layers:

	for (int l = 0; l < network -> LayersNumber - 1; ++l)
	{
		NeuronLayer *next_layer = layer;
		--layer;

		// For each hidden layer: GradSum = next GradSum * tr(next Net)

		matrix_multiply(NoTrans, Trans, next_layer -> GradSum, next_layer -> Net, layer -> GradSum,
			batch_size, layer -> NeuronsNumber, next_layer -> NeuronsNumber);

		// Multiplying by the activation derivative (softmax not supported here):

		for (int b = 0; b < batch_size; ++b)
		{
			int gradsum_pos = b * layer -> NeuronsNumber;

			for (int j = 0; j < layer -> NeuronsNumber; ++j)
				layer -> GradSum[gradsum_pos + j] *= der_activation(layer -> Fun, layer -> Sum[gradsum_pos + j]);
		}
	}
}


// Update 'grad_buffer' for the whole batch:
static void updateGradBufferBatch(NeuralNetwork *network, Number **grad_buffer, int batch_size)
{
	NeuronLayer *layer = network -> Layers;

	for (int l = 0; l < network -> LayersNumber; ++l)
	{
		// For each layer: grad_buffer[l] = tr(Input) * GradSum

		matrix_multiply(Trans, NoTrans, layer -> Input, layer -> GradSum, grad_buffer[l],
			layer -> InputSize + 1, layer -> NeuronsNumber, batch_size);

		++layer;
	}
}


///////////////////////////////////////////////////////////////////////////////////////
// Learning methods:
///////////////////////////////////////////////////////////////////////////////////////


static void gradientDescent(NeuralNetwork *network, Inputs *inputs, LearningParameters *params)
{
	// Buffers initialization:

	Number **grad_buffer = createNetworkBuffer(network);
	Number **M_buffer = NULL;
	Number **V_buffer = NULL;

	switch (params -> Optim)
	{
		case NO_OPT:

			break;

		case MOMENTUM:

			M_buffer = createNetworkBuffer(network);
			break;

		case RMSprop:

			V_buffer = createNetworkBuffer(network);
			break;

		case ADAM:

			M_buffer = createNetworkBuffer(network);
			V_buffer = createNetworkBuffer(network);
			break;

		default:
			break;
	}

	int batch_size_bound = MIN(network -> MaxBatchSize, inputs -> InputNumber);
	int step_number = 0; // Number of batches done since the beginning.

	// Learning begins:

	for (int epoch = 0; epoch < params -> EpochNumber; ++epoch)
	{
		if (params -> Shuffle == SHUFFLE)
			shuffleInputs(inputs);

		int current_remainder = (inputs -> InputNumber) % (params -> BatchSize); // here since BatchSize may be changed with epochs.
		int current_batch_size = current_remainder == 0 ? params -> BatchSize : current_remainder;
		int batch_index = 0, sum = 0;

		while (batch_index < inputs -> InputNumber)
		{
			Number **batch_questions = inputs -> Questions + batch_index;
			Number **batch_good_answers = inputs -> Answers + batch_index;

			Number *batch_answers = propagation(network, batch_questions, current_batch_size);

			if (params -> PrintEstimates)
			{
				for (int b = 0; b < current_batch_size; ++b)
					sum += recog_method(batch_good_answers[b], batch_answers + b * (inputs -> AnswersSize + 1),
						inputs -> AnswersSize, params -> RecogEstimates, VALIDATION);
			}

			backpropagation(network, batch_good_answers, params, current_batch_size);

			updateGradBufferBatch(network, grad_buffer, current_batch_size);

			++step_number;

			updateNetwork(network, grad_buffer, M_buffer, V_buffer, params, step_number);

			batch_index += current_batch_size;
			current_batch_size = params -> BatchSize; // only 'params -> BatchSize' after the first pass.
		}

		if (params -> PrintEstimates)
		{
			float learning_level = 100. * sum / inputs -> InputNumber;

			printf("\nEpoch °%d, learning level estimate: %.2f %%\n", epoch + 1, learning_level);
		}

		// Multiply the learning rate by an user given value:
		params -> LearningRate *= params -> LearningRateMultiplier;

		// Multiply the batch size by the given value:
		params -> BatchSize = MIN(params -> BatchSize * params -> BatchSizeMultiplier, batch_size_bound);
		params -> BatchSize = MAX(params -> BatchSize, 1); // so that batch size != 0.
	}

	freeNetworkBuffer(network, grad_buffer);
	freeNetworkBuffer(network, M_buffer);
	freeNetworkBuffer(network, V_buffer);
}


static void updateNetwork(NeuralNetwork *network, Number **grad_buffer, Number **M_buffer, Number **V_buffer,
	LearningParameters *params, int step_number)
{
	NeuronLayer *layer = network -> Layers;

	for (int l = 0; l < network -> LayersNumber; ++l)
	{
		// Regularization:

		if (params -> Reg == L2)
		{
			int weigthsLength = layer -> InputSize * layer -> NeuronsNumber;

			scal(layer -> Net, weigthsLength, 1 - params -> L2regCoeff);
		}


		// Optimizer:

		int netLength = (layer -> InputSize + 1) * layer -> NeuronsNumber;

		switch (params -> Optim)
		{
			case NO_OPT:

				// For all layers: Net -= eta * grad_buffer[l]

				addScal(layer -> Net, grad_buffer[l], netLength, - params -> LearningRate);

				break;


			case MOMENTUM:

				// M_buffer[l] = params -> MomentumRate * M_buffer[l] + params -> LearningRate * grad_buffer[l]:

				scal(M_buffer[l], netLength, params -> MomentumRate);

				addScal(M_buffer[l], grad_buffer[l], netLength, params -> LearningRate);

				// layer -> Net -= M_buffer[l]:

				addScal(layer -> Net, M_buffer[l], netLength, -1.);

				break;


			case RMSprop:

				;Number RMScoeff_conj = 1 - params -> RMScoeff;

				// V_buffer[l] = params -> RMScoeff * V_buffer[l] + RMScoeff_conj * grad_buffer[l] * grad_buffer[l]
				// layer -> Net -= params -> LearningRate * grad_buffer[l] / (number_sqrt(V_buffer[l]) + EPSILON)

				// Naive:

				for (int i = 0; i < netLength; ++i)
				{
					V_buffer[l][i] = params -> RMScoeff * V_buffer[l][i] + RMScoeff_conj * grad_buffer[l][i] * grad_buffer[l][i];

					layer -> Net[i] -= params -> LearningRate * grad_buffer[l][i] / (number_sqrt(V_buffer[l][i]) + EPSILON);
				}

				break;


			case ADAM:

				;Number AdamBetaM_conj = 1 - params -> AdamBetaM, AdamBetaV_conj = 1 - params -> AdamBetaV;

				Number scalar = params -> LearningRate * number_sqrt(1 - number_pow(params -> AdamBetaV, step_number)) /
					(1 - number_pow(params -> AdamBetaM, step_number));

				// M_buffer[l] = params -> AdamBetaM * M_buffer[l] + AdamBetaM_conj * grad_buffer[l]
				// V_buffer[l] = params -> AdamBetaM * V_buffer[l] + AdamBetaM_conj * grad_buffer[l] * grad_buffer[l]
				// layer -> Net -= scalar * M_buffer[l] / (number_sqrt(V_buffer[l]) + EPSILON)

				// Naive:

				for (int i = 0; i < netLength; ++i)
				{
					M_buffer[l][i] = params -> AdamBetaM * M_buffer[l][i] + AdamBetaM_conj * grad_buffer[l][i];

					V_buffer[l][i] = params -> AdamBetaV * V_buffer[l][i] + AdamBetaV_conj * grad_buffer[l][i] * grad_buffer[l][i];

					layer -> Net[i] -= scalar * M_buffer[l][i] / (number_sqrt(V_buffer[l][i]) + EPSILON);
				}

				break;


			default:
				break;
		}

		++layer;
	}
}
