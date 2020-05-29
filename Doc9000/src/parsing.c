#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parsing.h"
#include "backups.h"
#include "processing.h"
#include "utilities.h"


// Do not use these elsewhere, for it is unsafe!

short IllnessNumber;
short SymptomNumber;

float *CriticityArray;

char **IllnessStringArray;
char **SymptomStringArray;


static int ParsingPass;

///////////////////////////////////////////////////////////////
// Checks:


// Checks if all the available ressources are loaded in memory.
ParsingStatus checkRessources(void)
{
	int check = IllnessNumber != 0 && SymptomNumber != 0 &&
		IllnessStringArray != NULL && SymptomStringArray != NULL && CriticityArray != NULL;

	return check == 1 ? RESSOURCES_EXIST : REPARSING;
}


// Checks if 'IllnessNumber' and 'SymptomNumber' are available.
ParsingStatus checkNumberValues(void)
{
	if (IllnessNumber != 0 && SymptomNumber != 0)
		return RESSOURCES_EXIST;

	return loadCriticities();
}


// Checks if 'CriticityArray' is available.
inline ParsingStatus checkCriticities(void)
{
	return loadCriticities();
}


// Checks if 'IllnessStringArray' and 'SymptomStringArray' are available.
inline ParsingStatus checkStringArrays(void)
{
	return loadStringNames();
}


///////////////////////////////////////////////////////////////
// Getters:


// Returns the number of supported illnesses:
inline short getIllnessNumber(void)
{
	checkNumberValues();

	return IllnessNumber;
}


// Returns the number of supported symptoms:
inline short getSymptomNumber(void)
{
	checkNumberValues();

	return SymptomNumber;
}


// Get the name of a Illness:
inline const char* getIllnessName(Illness illness)
{
	checkStringArrays();

	if (illness < 0 || illness >= getIllnessNumber())
	{
		return DEFAULT_SYMPTOM_ILLNESS_NAME;
	}

	return IllnessStringArray[illness];
}


// Get the name of a Symptom:
inline const char* getSymptomName(Symptom symptom)
{
	checkStringArrays();

	if (symptom < 0 || symptom >= getSymptomNumber())
	{
		return DEFAULT_SYMPTOM_ILLNESS_NAME;
	}

	return SymptomStringArray[symptom];
}


// Get an Illness from its name. Returns 0 by default:
inline Illness getIllnessID(const char *string)
{
	if (string == NULL)
	{
		printf("\nCannot find index of a NULL string.\n");
		return 0;
	}

	checkStringArrays();

	short illness_number = getIllnessNumber();

	int index = getStringIndex(string, IllnessStringArray, illness_number);

	if (index >= illness_number)
	{
		index = 0; // default.
		printf("\nUnrecognized illness name: '%s'\n", string);
	}

	return index;
}


// Get a Symptom from its name. Returns 0 by default:
inline Symptom getSymptomID(const char *string)
{
	if (string == NULL)
	{
		printf("\nCannot find index of a NULL string.\n");
		return 0;
	}

	checkStringArrays();

	short symptom_number = getSymptomNumber();

	int index = getStringIndex(string, SymptomStringArray, symptom_number);

	if (index >= symptom_number)
	{
		index = 0; // default.
		printf("\nUnrecognized symptom name: '%s'\n", string);
	}

	return index;
}


// Returns the array containing each illness criticity:
inline const float* getCriticityArray(void)
{
	checkCriticities();

	return CriticityArray;
}


///////////////////////////////////////////////////////////////
// Freeing:


// Frees and resets to NULL.
void freeCriticityArray(void)
{
	free(CriticityArray);
	CriticityArray = NULL;
}


// Frees and resets to NULL.
void freeStringsArrays(void)
{
	// Do not check 'IllnessNumber' and 'SymptomNumber' here, previous values must be used.
	freeCharMatrix(&IllnessStringArray, IllnessNumber);
	freeCharMatrix(&SymptomStringArray, SymptomNumber);
}


// Free the medical data array pointed by the given adress, and set it to NULL.
// 'data_number' needs to be a parameter instead of an hardcoded 'IllnessNumber', for freeMedicalData().
void freeMedicalData(MedicalData **medData, int data_number)
{
	if (medData == NULL || *medData == NULL)
		return;

	checkNumberValues();

	for (int i = 0; i < data_number; ++i)
	{
		free((*medData)[i].symptomArray);
	}

	free(*medData);
	*medData = NULL;
}


///////////////////////////////////////////////////////////////
// Parsing:


// Parse the base dataset, in order to generate the ilness and symptoms lists, along with the criticity file.
// If called with a non-NULL medData, it will be filled with an array of medical data used for the learning phase.
ParsingStatus parseBaseDataset(MedicalData **medData)
{
	///////////////////////////////////////////////////////////////
	// Setup:

	ParsingStatus parsing_status = PARSING_SUCCESSFUL;

	if (medData == NULL && checkRessources() == RESSOURCES_EXIST)
		return RESSOURCES_EXIST;

	if (ParsingPass >= MAX_PARSING_PASSES) // protection against infinite loops in case of severe failure.
	{
		printf("\nMaximum parsing passes reached.\n");
		exit(EXIT_FAILURE);
	}

	++ParsingPass;

	FILE *src_file = fopen(SRC_BASE_DATASET, "r");

	if (src_file == NULL)
	{
		printf("\nCould not find file '%s'.\n", SRC_BASE_DATASET);
		exit(EXIT_FAILURE);
	}

	char buffer[MAX_NAME_LENGTH];

	char **illnessArray = createCharMatrix(MAX_ILLNESS_NUMBER, MAX_NAME_LENGTH);
	char **symptomArray = createCharMatrix(MAX_SYMPTOM_NUMBER, MAX_NAME_LENGTH);

	float criticityArray[MAX_ILLNESS_NUMBER];

	if (illnessArray == NULL || symptomArray == NULL)
	{
		printf("\nNot enough memory to create strings arrays.\n");
		fclose(src_file);
		exit(EXIT_FAILURE);
	}

	///////////////////////////////////////////////////////////////
	// Freeing, then resetting the static values beforehand:

	freeCriticityArray();
	freeStringsArrays();

	IllnessNumber = 0;
	SymptomNumber = 0;

	///////////////////////////////////////////////////////////////
	// Medical data buffer:

	MedicalData *medDataBuffer = NULL;

	if (medData != NULL)
	{
		medDataBuffer = (MedicalData*) calloc(MAX_ILLNESS_NUMBER, sizeof(MedicalData));

		if (medDataBuffer == NULL)
		{
			printf("\nNot enough memory to create 'medDataBuffer'.\n");
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < MAX_ILLNESS_NUMBER; ++i)
		{
			medDataBuffer[i].symptomArray = (Symptom*) calloc(MAX_SYMPTOM_PER_DATA, sizeof(Symptom));

			if (medDataBuffer[i].symptomArray == NULL)
			{
				printf("\nNot enough memory to create 'medDataBuffer[%d]'.\n", i);
				freeMedicalData(&medDataBuffer, MAX_ILLNESS_NUMBER);
				exit(EXIT_FAILURE);
			}
		}
	}

	///////////////////////////////////////////////////////////////
	// Parsing:

	int mode = 0, illness_rank = -1, symptom_rank = 0, symptom_data_rank = 0;

	strcpy(symptomArray[symptom_rank], TO_STRING(no_symptom));
	++symptom_rank;

	while (illness_rank < MAX_ILLNESS_NUMBER && symptom_rank < MAX_SYMPTOM_NUMBER)
	{
		if (fgets(buffer, MAX_NAME_LENGTH, src_file) == NULL) // End of file.
			break;

		if (buffer[0] == '\n')
		{
			mode = !mode;
			continue;
		}

		int length = strlen(buffer);

		if (length >= MAX_NAME_LENGTH)
		{
			printf("Maximum name length may be too small!\n");
			return PARSING_FAILED;
		}

		buffer[length - 1] = '\0'; // removing the ending '\n'. strlen(buffer) > 0.

		if (mode == 0) // illnesses & criticities
		{
			++illness_rank;

			strcpy(illnessArray[illness_rank], buffer);

			if (fgets(buffer, MAX_NAME_LENGTH, src_file) != NULL)
			{
				float criticity = atof(buffer);

				criticityArray[illness_rank] = boundCriticity(criticity);
			}

			if (medData != NULL)
			{
				medDataBuffer[illness_rank].illness = illness_rank;
				symptom_data_rank = 0;
			}
		}

		else // symptoms
		{
			int symptom_pos = getStringIndex(buffer, symptomArray, symptom_rank);

			if (symptom_pos == symptom_rank)
			{
				strcpy(symptomArray[symptom_rank], buffer);
				++symptom_rank;
			}

			if (medData != NULL)
			{
				++medDataBuffer[illness_rank].symptomNumber;
				medDataBuffer[illness_rank].symptomArray[symptom_data_rank] = symptom_pos;
				++symptom_data_rank;
			}
		}
	}

	fclose(src_file);

	///////////////////////////////////////////////////////////////
	// Filling, and freeing local buffers:

	IllnessNumber = illness_rank + 1; // started at -1.
	SymptomNumber = symptom_rank;

	// printf("\nIllness number: %d\nSymptom number: %d\n", IllnessNumber, SymptomNumber);

	if (IllnessNumber >= MAX_ILLNESS_NUMBER || SymptomNumber >= MAX_SYMPTOM_NUMBER)
	{
		printf("Maximum number of illnesses or symptoms may be too small!\n");
		return PARSING_FAILED;
	}

	IllnessStringArray = createCharMatrix(IllnessNumber, MAX_NAME_LENGTH);
	SymptomStringArray = createCharMatrix(SymptomNumber, MAX_NAME_LENGTH);
	CriticityArray = (float*) calloc(IllnessNumber, sizeof(float));

	if (IllnessStringArray == NULL || SymptomStringArray == NULL || CriticityArray == NULL)
	{
		printf("\nNot enough memory to create strings or criticities arrays.\n");
		exit(EXIT_FAILURE);
	}

	// Illnesses:

	for (int i = 0; i < IllnessNumber; ++i)
	{
		strcpy(IllnessStringArray[i], illnessArray[i]);
	}

	freeCharMatrix(&illnessArray, MAX_ILLNESS_NUMBER);

	// Symptoms:

	for (int i = 0; i < SymptomNumber; ++i)
	{
		strcpy(SymptomStringArray[i], symptomArray[i]);
	}

	freeCharMatrix(&symptomArray, MAX_SYMPTOM_NUMBER);

	// Criticities:

	for (int i = 0; i < IllnessNumber; ++i)
	{
		CriticityArray[i] = criticityArray[i];
	}

	///////////////////////////////////////////////////////////////
	// Preparing the medical data:

	if (medData != NULL)
	{
		*medData = (MedicalData*) calloc(IllnessNumber, sizeof(MedicalData));

		if (*medData == NULL)
		{
			printf("\nNot enough memory to allocate to 'medData'.\n");
			freeMedicalData(&medDataBuffer, MAX_ILLNESS_NUMBER);
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < IllnessNumber; ++i)
		{
			(*medData)[i].illness = medDataBuffer[i].illness;
			(*medData)[i].symptomNumber = medDataBuffer[i].symptomNumber;

			(*medData)[i].symptomArray = (Symptom*) calloc(medDataBuffer[i].symptomNumber, sizeof(Symptom));

			if ((*medData)[i].symptomArray == NULL)
			{
				printf("\nNot enough memory to allocate to 'medData[%d]'.\n", i);
				freeMedicalData(&medDataBuffer, MAX_ILLNESS_NUMBER);
				freeMedicalData(medData, IllnessNumber);
				exit(EXIT_FAILURE);
			}

			for (int j = 0; j < medDataBuffer[i].symptomNumber; ++j)
			{
				(*medData)[i].symptomArray[j] = medDataBuffer[i].symptomArray[j];
			}
		}

		freeMedicalData(&medDataBuffer, MAX_ILLNESS_NUMBER);
	}

	printf("\nParsing done. Saving the results.\n");

	if (saveParsingResults() != SAVING_SUCCESSFUL)
		parsing_status = PARSING_FAILED;

	return parsing_status;
}
