#include <stdio.h>
#include <stdlib.h>

#include "learning_dataset.h"
#include "parsing.h"


static MedicalData *BaseDataset;
static int BaseDatasetLength;


// Register a symptom in the given question. Returns 1 on success.
// 'value' must be used according to the following pattern:
// Symptom detected, no specific value attached to it -> 1.
// Symptom detected, with a specific value attached (e.g temperature): between 0 and 1.5 depending on the severity.
// Symptom not tested, information missing -> 0.
// Symptom refuted -> -1.
int enterSymptom(Number *question, Symptom symptom, Number value)
{
	if (symptom < 0 || symptom >= getSymptomNumber())
	{
		printf("Invalid symptom.\n");
		return 0;
	}

	if (symptom == no_symptom) // 'no_symptom' ignored!
		return 0;

	question[symptom] = value;

	return 1;
}


// Generate a noisy value:
Number generateValue(void)
{
	float probability = uniform_random(0.f, 1.f);

	return probability < SYMPTOM_THESHOLD ? VALUE_ABSENT_SYMPTOM : 1.f;
}


// Adds a given quantity of the given illness into the learning dataset:
void addIllnesses(Number **questions, Number **answers, const MedicalData *data, int dataQuantity)
{
	for (int i = 0; i < dataQuantity; ++i)
	{
		for (int symptomIndex = 0; symptomIndex < data -> symptomNumber; ++symptomIndex)
		{
			const Symptom symptom = data -> symptomArray[symptomIndex];

			if (symptom != no_symptom)
			{
				enterSymptom(questions[i], symptom, generateValue());
			}
		}

		answers[i][data -> illness] = 1.f;
	}
}


// Creating a learning_dataset:
Inputs* createDataset(int inputs_number)
{
	initDatasetAssets();

	const int questions_size = getSymptomNumber();
	const int answers_size = getIllnessNumber();

	Number **questions = createMatrix(inputs_number, questions_size);
	Number **answers = createMatrix(inputs_number, answers_size);

	Inputs *inputs = createInputs(inputs_number, questions_size, answers_size, questions, answers);

	const int inputsPerIllness = inputs_number / answers_size;
	const int remainder = inputs_number % answers_size;

	for (int index = 0; index < inputs_number; ++index)
	{
		for (int symptom = 0; symptom < questions_size; ++symptom)
			questions[index][symptom] = VALUE_ABSENT_SYMPTOM;
	}

	int index = 0;

	for (int illnessIndex = 0; illnessIndex < answers_size; ++illnessIndex)
	{
		addIllnesses(questions + index, answers + index, BaseDataset + illnessIndex, inputsPerIllness);

		index += inputsPerIllness;
	}

	// In case remainder != 0. First illness in 'BaseDataset' added:
	addIllnesses(questions + index, answers + index, BaseDataset, remainder);

	return inputs;
}


// Fetch the json file to create the base dataset, assuming it hasn't been done already.
void initDatasetAssets(void)
{
	if (!BaseDataset && parseBaseDataset(&BaseDataset) == PARSING_FAILED)
	{
		printf("\nShutting down the application, due to a failed parsing.\n");
		exit(EXIT_FAILURE);
	}

	BaseDatasetLength = getIllnessNumber();
}


// This must only be done when the dataset has been created, and no learning is to be done.
void freeDatasetAssets(void)
{
	if (!BaseDataset)
		return;

	freeMedicalData(&BaseDataset, BaseDatasetLength);
}


// Printing the (quite long) base dataset, which is used to create the learning dataset:
void printBaseDataset(void)
{
	printf("------------------------------------------------------------------\n");
	printf("Printing the base dataset from which the learning one will be created:\n");

	initDatasetAssets();

	if (!BaseDataset)
	{
		printf("\nBase dataset failed to be fetched.\n");
		exit(EXIT_FAILURE);
	}

	printf("Number of data: %d\n", BaseDatasetLength);

	for (int data_index = 0; data_index < BaseDatasetLength; ++data_index)
	{
		MedicalData *data = BaseDataset + data_index;

		printf("\nIllness: %s\nSymptom number: %d\n\n", getIllnessName(data -> illness), data -> symptomNumber);

		for (int symptom_index = 0; symptom_index < data -> symptomNumber; ++symptom_index)
		{
			printf("- Symptom: %s\n", getSymptomName(data -> symptomArray[symptom_index]));
		}
	}
}
