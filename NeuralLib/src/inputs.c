#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "inputs.h"
#include "matrix.h"
#include "saving.h"


Inputs* createInputs(int InputNumber, int QuestionsSize, int AnswersSize, Number** Questions, Number** Answers)
{
	Inputs *inputs = (Inputs*) calloc(1, sizeof(Inputs));

	*(int*) &(inputs -> InputNumber) = InputNumber;
	*(int*) &(inputs -> QuestionsSize) = QuestionsSize;
	*(int*) &(inputs -> AnswersSize) = AnswersSize;
	inputs -> Questions = Questions;
	inputs -> Answers = Answers;

	return inputs;
}


// Frees the given Inputs passed by address, and sets it to NULL.
void freeInputs(Inputs **inputs)
{
	if (inputs == NULL || *inputs == NULL)
		return;

	freeMatrix(&((*inputs) -> Questions), (*inputs) -> InputNumber);

	if ((*inputs) -> Answers != NULL)
		freeMatrix(&((*inputs) -> Answers), (*inputs) -> InputNumber);

	free(*inputs);
	*inputs = NULL;
}


void printInputs(const Inputs *inputs, PrintOption opt)
{
	if (inputs == NULL)
	{
		printf("\nCannot print informations of NULL inputs.\n\n");
		return;
	}

	printf("\nThe given inputs have the following attributes:\n\nInput number: %d\nQuestions size: %d\nAnswers size: %d\n\n",
		inputs -> InputNumber, inputs -> QuestionsSize, inputs -> AnswersSize);

	if (opt == ALL)
	{
		printf("Questions:\n\n");

		printMatrix(inputs -> Questions, inputs -> InputNumber, inputs -> QuestionsSize);

		printf("Answers:\n\n");

		printMatrix(inputs -> Answers, inputs -> InputNumber, inputs -> AnswersSize);
	}
}


int saveInputs(const Inputs *inputs, const char *foldername)
{
	if (inputs == NULL || inputs -> InputNumber == 0 || inputs -> Questions == NULL)
	{
		printf("\nNo data to be saved in the given inputs.\n\n");
		return 0;
	}

	if (foldername == NULL)
	{
		printf("\nNULL foldername.\n\n");
		return 0;
	}

	// Creating a folder in which the inputs will be saved:

	createFolder(foldername);

	// Creating a text file with the inputs' readable data:

	char infos_filename[MAX_PATH_LENGTH];
	sprintf(infos_filename, "%s/infos.txt", foldername);

	FILE *infos_file = fopen(infos_filename, "w");

	if (infos_file == NULL)
		exitFileError(infos_file, "Cannot create the file", infos_filename);

	fprintf(infos_file, "Inputs.\nSize of Number: %ld bytes\nInput number: %d\nQuestions size: %d\nAnswers size: %d\n",
		sizeof(Number), inputs -> InputNumber, inputs -> QuestionsSize, inputs -> AnswersSize);

	fclose(infos_file);

	// Saving those inputs' questions:

	char questions_filename[MAX_PATH_LENGTH];
	sprintf(questions_filename, "%s/questions.bin", foldername);

	save_matrix(inputs -> Questions, inputs -> InputNumber, inputs -> QuestionsSize, questions_filename);

	if (inputs -> AnswersSize != 0 && inputs -> Answers != NULL)
	{
		// Saving those inputs' answers:

		char answers_filename[MAX_PATH_LENGTH];
		sprintf(answers_filename, "%s/answers.bin", foldername);

		save_matrix(inputs -> Answers, inputs -> InputNumber, inputs -> AnswersSize, answers_filename);
	}

	printf("\nThe given inputs have been successfully saved to '%s'.\n\n", foldername);

	return 1;
}


Inputs* loadInputs(const char *foldername)
{
	if (foldername == NULL)
	{
		printf("\nNULL foldername.\n\n");
		exit(EXIT_FAILURE);
	}

	char infos_filename[MAX_PATH_LENGTH];
	char error_message[MAX_PATH_LENGTH];

	sprintf(infos_filename, "%s/infos.txt", foldername);

	// Reading the text file with the inputs' readable data:

	FILE *infos_file = fopen(infos_filename, "r");

	if (infos_file == NULL)
		exitFileError(infos_file, "Cannot read the file", infos_filename);

	skip(infos_file, "Inputs.\nSize of Number: ");

	int size_of_number, input_number, questions_size, answers_size;

	if (fscanf(infos_file, "%d", &size_of_number) != 1 || size_of_number != sizeof(Number))
	{
		sprintf(error_message, "Not found or incompatible 'Number' size (%d vs %ld) in", size_of_number, sizeof(Number));
		exitFileError(infos_file, error_message, infos_filename);
	}

	skip(infos_file, " bytes\nInput number: ");

	if (fscanf(infos_file, "%d", &input_number) != 1)
		exitFileError(infos_file, "Input number couldn't be retrieved from", infos_filename);

	skip(infos_file, "\nQuestions size: ");

	if (fscanf(infos_file, "%d", &questions_size) != 1)
		exitFileError(infos_file, "Questions size couldn't be retrieved from", infos_filename);

	skip(infos_file, "\nAnswers size: ");

	if (fscanf(infos_file, "%d", &answers_size) != 1)
		exitFileError(infos_file, "Answers size couldn't be retrieved from", infos_filename);

	fclose(infos_file);

	Number **questions = createMatrix(input_number, questions_size);

	Inputs *inputs = createInputs(input_number, questions_size, answers_size, questions, NULL);

	// Loading those inputs' questions:

	char questions_filename[MAX_PATH_LENGTH];
	sprintf(questions_filename, "%s/questions.bin", foldername);

	load_toMatrix(inputs -> Questions, input_number, questions_size, questions_filename);

	if (answers_size != 0)
	{
		// Loading those inputs' answers:

		char answers_filename[MAX_PATH_LENGTH];
		sprintf(answers_filename, "%s/answers.bin", foldername);

		inputs -> Answers = createMatrix(input_number, answers_size);

		load_toMatrix(inputs -> Answers, input_number, answers_size, answers_filename);
	}

	printf("\nThe given inputs have been successfully loaded from '%s'.\n\n", foldername);

	return inputs;
}


// Finds the mean and standard deviation vectors of the given inputs, stored in a 2 x QuestionsSize matrix.
// This matrix is returned in order to be able to normalize other inputs by the previous mean and std deviation.
Number** find_mean_stddev_matrix(const Inputs *inputs)
{
	if (inputs == NULL)
	{
		printf("\nCannot find stats of NULL inputs.\n\n");
		return NULL;
	}

	int input_number = inputs -> InputNumber;
	int input_size = inputs -> QuestionsSize;

	Number **questions = inputs -> Questions;
	Number **mean_stddev_matrix = createMatrix(2, input_size);

	Number *mean_vector = mean_stddev_matrix[0];
	Number *std_dev_vector = mean_stddev_matrix[1];

	// Computing the sum and sum of squares vectors:
	for (int i = 0; i < input_number; ++i)
	{
		for (int j = 0; j < input_size; ++j)
		{
			mean_vector[j] += questions[i][j];
			std_dev_vector[j] += questions[i][j] * questions[i][j];
		}
	}

	// Computing the mean vector and (biased) standard deviation vectors:
	for (int j = 0; j < input_size; ++j)
	{
		mean_vector[j] /= input_number;
		std_dev_vector[j] = sqrt(std_dev_vector[j] / input_number - mean_vector[j] * mean_vector[j] + EPSILON);
		// printf("mean[%i] = %.2f, std_dev[%i] = %.2f\n", j, mean_vector[j], j, std_dev_vector[j]);
	}

	return mean_stddev_matrix;
}


// Normalizes the given inputs, so that: mean = 0 and standard deviation = 1.
void normalize(Inputs *inputs, Number* const* mean_stddev_matrix)
{
	if (inputs == NULL)
	{
		printf("\nCannot normalize NULL inputs.\n\n");
		return;
	}

	if (mean_stddev_matrix == NULL)
	{
		printf("\nMean/std dev matrix has not been allocated.\n\n");
		return;
	}

	Number **questions = inputs -> Questions;
	Number *mean_vector = mean_stddev_matrix[0];
	Number *std_dev_vector = mean_stddev_matrix[1];

	for (int i = 0; i < inputs -> InputNumber; ++i)
	{
		for (int j = 0; j < inputs -> QuestionsSize; ++j)
			questions[i][j] = (questions[i][j] - mean_vector[j]) / std_dev_vector[j];
	}
}


// Applying the Fisher–Yates shuffle on the given inputs:
void shuffleInputs(Inputs *inputs)
{
	if (inputs == NULL)
	{
		printf("\nCannot shuffle NULL inputs.\n\n");
		return;
	}

	int len = inputs -> InputNumber;

	Number **Questions = inputs -> Questions;
	Number **Answers = inputs -> Answers;
	Number *temp;

	if (len - 1 > RAND_MAX)
		printf("\nNumber of inputs is greater than RAND_MAX!\n\n");

	// N.B: questions and answers receive the same shuffling!

	for (int i = len - 1; i >= 1; --i)
	{
		int j = rand() % (i + 1); // 0 ≤ j ≤ i. Biased, but negligeable.

		temp = Questions[i];

		Questions[i] = Questions[j];
		Questions[j] = temp;

		temp = Answers[i];

		Answers[i] = Answers[j];
		Answers[j] = temp;
	}
}
