#ifndef INPUTS_H
#define INPUTS_H


#include "settings.h"


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


#endif
