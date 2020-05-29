#define _GNU_SOURCE // for qsort_r


#include <stdio.h>
#include <stdlib.h>

#include "recognition.h"
#include "matrix.h"


#define RECOG_THRESHOLD 0.5


// Returns the index of the greatest value in 'vector':
inline static int maxValueIndex(const Number *vector, int len)
{
	int index = 0;
	Number max = vector[0];

	for (int i = 1; i < len; ++i)
	{
		if (vector[i] > max)
		{
			max = vector[i];
			index = i;
		}
	}

	return index;
}


// Validation:

// MAX_VALUE -> Checks if the class with higher value is the good answer. Do not use for binary classification.

// MAX_CORRECT -> Checks if the class with higher value is the good answer, if said value is >= RECOG_THRESHOLD.
// Useful for binary classification. Stricter than 'valid_MaxValue'.

// ALL_CORRECT -> Checks if all classes have good answers. Useful for binary classification,
// and non-mutually exclusive classification problems. Stricter than 'valid_MaxCorrect'.

// Checks if the predicted answer is correct, whith respect to the chosen recognition method:
int validation_method(Number *good_answer, const Number *answer, int len, RecognitionMode recog)
{
	if (recog == MAX_VALUE)
		return good_answer[maxValueIndex(answer, len)] == 1;

	else if (recog == MAX_CORRECT)
	{
		const int max_index = maxValueIndex(answer, len);

		return good_answer[max_index] == (answer[max_index] >= RECOG_THRESHOLD);
	}

	else // ALL_CORRECT
	{
		int index = 0;

		while (index < len && good_answer[index] == (answer[index] >= RECOG_THRESHOLD))
			++index;

		return index == len;
	}
}


// Framework for using the validation or prediction functions:
int recog_method(Number *goodOrToFill_answers, const Number *answer, int len, RecognitionMode recog, RecogType type)
{
	if (type == VALIDATION)
		return validation_method(goodOrToFill_answers, answer, len, recog);

	else // PREDICTION
	{
		copyVector(goodOrToFill_answers, answer, len);
		return 0;
	}
}


// Find the most probable class answer:
// For binary class problems, returns the binary answer.
// For mutally exclusive class problems, returns the index of the guessed class.
// For mixed problems, one can shift 'answer' and modify 'len' accordingly on each subset which are binary/exclusive.
// Optional: 'confidence_level' is filled if a non-NULL arg is passed.
int findMostProbable(const Number *answer, int len, Number *confidence_level)
{
	if (len == 1)
	{
		int binary_answer = answer[0] >= RECOG_THRESHOLD;

		if (confidence_level != NULL)
			*confidence_level = binary_answer ? answer[0] : 1. - answer[0];

		return binary_answer;
	}

	const int max_value_index = maxValueIndex(answer, len);

	if (confidence_level != NULL)
	{
		Number sum_values = 0;

		for (int i = 0; i < len; ++i)
			sum_values += answer[i];

		*confidence_level = answer[max_value_index] / (sum_values + EPSILON);
	}

	return max_value_index;
}


// Comparison between the values of 'valuesArray' given by the two indexes. Used in qsort_r.
static int comparison_val_desc(const void *elem1, const void *elem2, void *valuesArray)
{
	int e1 = *((int*) elem1);
	int e2 = *((int*) elem2);

	Number *array = valuesArray;

	// Do not return a substraction, it needs to work on floats, and will be casted as an int.

	if (array[e1] < array[e2])
		return 1;

	if (array[e1] > array[e2])
		return -1;

	return 0;
}


// Finds the 'bufferLength' greater values in 'valuesArray', and store their index in 'buffer', in descending order:
void findGreaterValuesIndex(int *buffer, int bufferLength, const Number *valuesArray, int valuesArrayLength)
{
	if (!buffer || bufferLength <= 0 || !valuesArray || valuesArrayLength < bufferLength)
	{
		printf("\nIncorrect inputs in a findGreaterValuesIndex() call.\n");
		return;
	}

	// Initializing 'buffer' with the 'bufferLength' first values of 'valuesArray', sorted in decreasing order:

	for (int i = 0; i < bufferLength; ++i)
		buffer[i] = i;

	qsort_r((void*) buffer, bufferLength, sizeof(int), comparison_val_desc, (void*) valuesArray);

	// Parsing 'valuesArray':

	for (int i = bufferLength; i < valuesArrayLength; ++i)
	{
		int j = bufferLength - 1;

		if (valuesArray[i] > valuesArray[buffer[j]]) // optimization: only 1 test usually.
		{
			do {--j;}
			while (j >= 0 && valuesArray[i] > valuesArray[buffer[j]]);

			for (int k = bufferLength - 1; k > j + 1; --k)
				buffer[k] = buffer[k - 1];

			buffer[j + 1] = i;
		}
	}
}
