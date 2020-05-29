//////////////////////////////////////////////////////////
// NeuralLib v1.3
//////////////////////////////////////////////////////////

// This file contains the public definitions and functions from the 'NeuralLib' library.

#ifndef NEURAL_LIB_H
#define NEURAL_LIB_H

//////////////////////////////////////////////////////////
// settings.h
//////////////////////////////////////////////////////////


typedef enum {INFOS, ALL} PrintOption;


#if defined _FLOAT
	typedef float Number;

#elif defined _DOUBLE
	typedef double Number;
#endif


// Used to get the length of an array on the stack, in the same context it is defined.
// Do _not_ use this on an array allocated in the heap, or on an array passed as parameter of a function.
#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(array) \
	(sizeof(array) / sizeof(*(array)))
#endif

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) ((x) < (y) ? (y) : (x))
#endif

// Returns the arg name as a string:
#ifndef TO_STRING
#define TO_STRING(STRING) #STRING
#endif


// Used to check the equality of lengths between two arrays. Returns said length.
// Warning: use this only in the same context of the definition of said arrays.
#ifndef ARRAYS_COMPARE_LENGTH
#define ARRAYS_COMPARE_LENGTH(array_1, array_2)														\
({																									\
	if (ARRAY_LENGTH(array_1) != ARRAY_LENGTH(array_2))												\
	{																								\
		printf("\nIncompatible lengths between '%s' and '%s': %ld vs %ld.\n",						\
			TO_STRING(array_1), TO_STRING(array_2), ARRAY_LENGTH(array_1), ARRAY_LENGTH(array_2));	\
		exit(EXIT_FAILURE);																			\
	}																								\
	(int) ARRAY_LENGTH(array_1);																	\
})
#endif


//////////////////////////////////////////////////////////
// activation.h
//////////////////////////////////////////////////////////


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


#define ACTIVATION_ID(ENUM) ENUM,

// Enum of activation functions:
typedef enum {ACTIVATION_APPLY(ACTIVATION_ID)} Activation;


//////////////////////////////////////////////////////////
// inputs.h
//////////////////////////////////////////////////////////


// Answers = NULL if the inputs are not for learning.
typedef struct
{
	const int InputNumber;
	const int QuestionsSize;
	const int AnswersSize;
	Number **Questions;		// InputNumber x QuestionsSize
	Number **Answers;		// InputNumber x AnswersSize
} Inputs;


Inputs* createInputs(int InputNumber, int QuestionsSize, int AnswersSize, Number** Questions, Number** Answers);


// Frees the given Inputs passed by address, and sets it to NULL.
void freeInputs(Inputs **inputs);


void printInputs(const Inputs *inputs, PrintOption opt);


int saveInputs(const Inputs *inputs, const char *foldername);


Inputs* loadInputs(const char *foldername);


// Finds the mean and standard deviation vectors of the given inputs, stored in a 2 x QuestionsSize matrix.
// This matrix is returned in order to be able to normalize other inputs by the previous mean and std deviation.
Number** find_mean_stddev_matrix(const Inputs *inputs);


// Normalizes the given inputs, so that: mean = 0 and standard deviation = 1.
void normalize(Inputs *inputs, Number* const* mean_stddev_matrix);


// Applying the Fisher–Yates shuffle on the given inputs:
void shuffleInputs(Inputs *inputs);


//////////////////////////////////////////////////////////
// neural_network.h
//////////////////////////////////////////////////////////


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


// Printing a network's content:
void printNetwork(const NeuralNetwork *network, PrintOption opt);


// Saving a neural network. N.B: MaxBatchSize doesn't need to be saved, for its value is arbitrary:
int saveNetwork(const NeuralNetwork *network, const char *foldername);


// Loading a neural network:
NeuralNetwork* loadNetwork(const char *foldername, int MaxBatchSize);


//////////////////////////////////////////////////////////
// recognition.h
//////////////////////////////////////////////////////////


// MAX_VALUE: multiclass problems only. Unique positive answer.
// MAX_CORRECT: binary or multiclass problems. At most one positive answer.
// ALL_CORRECT: binary, multiclass or non-mutually exclusive multiclass problems. Multiple positive answers allowed.
// Modes sorted by strictness: MAX_VALUE < MAX_CORRECT < ALL_CORRECT.
typedef enum {MAX_VALUE, MAX_CORRECT, ALL_CORRECT} RecognitionMode;


// Find the most probable class answer:
// For binary class problems, returns the binary answer.
// For mutally exclusive class problems, returns the index of the guessed class.
// For mixed problems, one can shift 'answer' and modify 'len' accordingly on each subset which are binary/exclusive.
// Optional: 'confidence_level' is filled if a non-NULL arg is passed.
int findMostProbable(const Number *answer, int len, Number *confidence_level);


// Finds the 'bufferLength' greater values in 'valuesArray', and store their index in 'buffer', in descending order:
void findGreaterValuesIndex(int *buffer, int bufferLength, const Number *valuesArray, int valuesArrayLength);


//////////////////////////////////////////////////////////
// learning.h
//////////////////////////////////////////////////////////


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


// Creates some learning settings, initialized with default values:
LearningParameters* initLearningParameters(void);


// Frees the given LearningParameters passed by address, and sets it to NULL.
void freeParameters(LearningParameters **params);


// Make the neural network learn the given inputs, while using the given parameters:
void learn(NeuralNetwork *network, Inputs *inputs, LearningParameters *params);


// Compare the network answers to the correct ones, and print the validation level.
void validation(NeuralNetwork *network, Inputs *inputs, RecognitionMode recog);


// Write the network answers in the given inputs.
void prediction(NeuralNetwork *network, Inputs *inputs);


//////////////////////////////////////////////////////////
// matrix.h
//////////////////////////////////////////////////////////


// Filled with 0s:
Number* createVector(int len);

// Filled with 0s:
Number** createMatrix(int rows, int cols);

// Frees a vector, passed by address, and sets it to NULL:
void freeVector(Number **vector);

// Frees a matrix, passed by address, and sets it to NULL:
void freeMatrix(Number ***matrix, int rows);

void printVector(const Number *vector, int len);

void printFlatMatrix(const Number *matrix, int rows, int cols);

void printMatrix(Number* const* matrix, int rows, int cols);

void copyVector(Number *dest, const Number *src, int len);

void copyMatrix(Number **dest, Number* const* src, int rows, int cols);


//////////////////////////////////////////////////////////
// saving.h
//////////////////////////////////////////////////////////


// Creating a folder:
void createFolder(const char *foldername);


// Moves a file (and eventually renames it) to another location.
// Returns 1 on success, 0 else (may happen if 'dest_path' contains non-existent folders).
int moveFile(const char *dest_path, const char *src_path);


// Deletes the given file. Returns 1 on success, 0 else.
int deleteFile(const char *filename);


// Get the size of the given binary file, in bytes:
long int getFileSize(const char *filename);


void save_flat_matrix(const Number *matrix, int rows, int cols, const char *filename);


void save_matrix(Number* const* matrix, int rows, int cols, const char *filename);


// Matrix must alrealy be allocated in memory:
void load_toFlatMatrix(Number *matrix, int rows, int cols, const char *filename);


// Matrix must alrealy be allocated in memory:
void load_toMatrix(Number **matrix, int rows, int cols, const char *filename);


//////////////////////////////////////////////////////////
// image_processing.h
//////////////////////////////////////////////////////////


typedef enum {MAX_POOLING, AVERAGE_POOLING} PoolingMode;


// Print in the console a grayscale image contained in a 1-dimensional array:
void printGrayscaleImage(const Number *image, int width, int height);


void pooling(Number *output, const Number *input, int output_width, int output_height,
	int input_width, int input_height, PoolingMode poolmeth);


void recenter(Number *dest, const Number *src, int width, int height);


//////////////////////////////////////////////////////////
// random.h
//////////////////////////////////////////////////////////


// Returns a random number in [min, max[.
Number uniform_random(Number min, Number max);


//////////////////////////////////////////////////////////
// benchmarking
//////////////////////////////////////////////////////////


// Used to measure elapsed time. Thread safe.
double get_time(void);


//////////////////////////////////////////////////////////
// endian.h
//////////////////////////////////////////////////////////


// The following functions are useful to translate float/double matrices
// from the little-endian format to the big-endian format, and vice versa.


// Swap the bytes order of every float in the given matrix:
void swap_matrix_float(float **matrix, int rows, int cols);


// Swap the bytes order of every double in the given matrix:
void swap_matrix_double(double **matrix, int rows, int cols);


#endif
