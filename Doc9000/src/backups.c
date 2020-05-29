#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backups.h"
#include "utilities.h"


///////////////////////////////////////////////////////////////
// Private inclusions:

// Do not use these elsewhere, for it is unsafe!

extern short IllnessNumber;
extern short SymptomNumber;

extern float *CriticityArray;

extern char **IllnessStringArray;
extern char **SymptomStringArray;


// Checks if all the available ressources are loaded in memory.
ParsingStatus checkRessources(void);

// Checks if 'IllnessNumber' and 'SymptomNumber' are available.
ParsingStatus checkNumberValues(void);


///////////////////////////////////////////////////////////////


// Saves all ressources to the related files.
ParsingStatus saveParsingResults(void)
{
	if (checkRessources() != RESSOURCES_EXIST)
	{
		printf("\nParsing in order to save.\n");
		parseBaseDataset(NULL);
		return REPARSING;
	}

	// Illnesses:

	FILE *file_illness = fopen(ILLNESS_LIST_FILENAME, "w");

	if (file_illness == NULL)
	{
		printf("\nCould not write to: '%s'.\n", ILLNESS_LIST_FILENAME);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < IllnessNumber; ++i)
	{
		fprintf(file_illness, "%s\n", IllnessStringArray[i]);
	}

	fclose(file_illness);

	// Symptoms:

	FILE *file_symptom = fopen(SYMPTOM_LIST_FILENAME, "w");

	if (file_symptom == NULL)
	{
		printf("\nCould not write to: '%s'.\n", SYMPTOM_LIST_FILENAME);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < SymptomNumber; ++i)
	{
		fprintf(file_symptom, "%s\n", SymptomStringArray[i]);
	}

	fclose(file_symptom);

	// Criticities:

	FILE *file_criticity = fopen(CRITICITIES_FILENAME, "wb");

	if (file_criticity == NULL)
	{
		printf("\nCould not write to: '%s'.\n", CRITICITIES_FILENAME);
		exit(EXIT_FAILURE);
	}

	fwrite(&IllnessNumber, sizeof(IllnessNumber), 1, file_criticity);
	fwrite(&SymptomNumber, sizeof(SymptomNumber), 1, file_criticity);
	fwrite(CriticityArray, sizeof(float), IllnessNumber, file_criticity);

	fclose(file_criticity);

	return SAVING_SUCCESSFUL;
}


// Load 'IllnessNumber', 'SymptomNumber', and the criticites from the related file.
ParsingStatus loadCriticities(void)
{
	// Do not call 'checkNumberValues()' for this, it would cause an infinite loop:
	if (IllnessNumber != 0 && SymptomNumber != 0 && CriticityArray != NULL)
		return RESSOURCES_EXIST;

	freeCriticityArray(); // In case number values are incorrect.

	FILE *file_criticity = fopen(CRITICITIES_FILENAME, "rb");

	if (file_criticity == NULL)
	{
		printf("\nFile '%s' not found, reparsing the base dataset.\n", CRITICITIES_FILENAME);
		parseBaseDataset(NULL);
		return REPARSING;
	}

	if (fread(&IllnessNumber, sizeof(IllnessNumber), 1, file_criticity) != 1)
	{
		printf("\nReparsing! Could not retrieve 'IllnessNumber' from '%s'.\n", CRITICITIES_FILENAME);
		fclose(file_criticity);
		parseBaseDataset(NULL);
		return REPARSING;
	}

	if (fread(&SymptomNumber, sizeof(SymptomNumber), 1, file_criticity) != 1)
	{
		printf("\nReparsing! Could not retrieve 'SymptomNumber' from '%s'.\n", CRITICITIES_FILENAME);
		fclose(file_criticity);
		parseBaseDataset(NULL);
		return REPARSING;
	}

	// printf("\nIllness number: %d\nSymptom number: %d\n", IllnessNumber, SymptomNumber);

	CriticityArray = (float*) calloc(IllnessNumber, sizeof(float));

	size_t criticities_read = fread(CriticityArray, sizeof(float), IllnessNumber, file_criticity);

	if (criticities_read != IllnessNumber)
	{
		printf("\nReparsing! Could only read %ld criticities from '%s'.\n", criticities_read, CRITICITIES_FILENAME);
		freeCriticityArray();
		fclose(file_criticity);
		parseBaseDataset(NULL);
		return REPARSING;
	}

	fclose(file_criticity);

	printf("\nCriticities loaded succesfully.\n");

	return LOADING_SUCCESSFUL;
}


// Loads the strings from the given file:
static ParsingStatus loadStringFile(char *filename, short string_number, char **string_array, char *buffer)
{
	FILE *file = fopen(filename, "r");

	if (file == NULL)
	{
		printf("\nFile '%s' not found, reparsing the base dataset.\n", filename);
		freeStringsArrays();
		parseBaseDataset(NULL);
		return REPARSING;
	}

	short i = 0;

	while (i < string_number && fgets(buffer, MAX_NAME_LENGTH, file) != NULL)
	{
		buffer[strlen(buffer) - 1] = '\0'; // removing the ending '\n'. strlen(buffer) > 0.

		strcpy(string_array[i], buffer);
		++i;
	}

	fclose(file);

	if (i < string_number)
	{
		printf("\nReparsing! Too few strings read: %d vs %d.\n", i, string_number);
		freeStringsArrays();
		parseBaseDataset(NULL);
		return REPARSING;
	}

	return LOADING_SUCCESSFUL;
}


// Loads 'IllnessStringArray' and 'SymptomStringArray' in memory from the related files:
ParsingStatus loadStringNames(void)
{
	if (checkNumberValues() == REPARSING)
		return REPARSING;

	if (IllnessStringArray != NULL && SymptomStringArray != NULL)
		return RESSOURCES_EXIST;

	freeStringsArrays(); // In case only one is NULL.

	IllnessStringArray = createCharMatrix(IllnessNumber, MAX_NAME_LENGTH);
	SymptomStringArray = createCharMatrix(SymptomNumber, MAX_NAME_LENGTH);

	if (IllnessStringArray == NULL || SymptomStringArray == NULL)
	{
		printf("\nNot enough memory to create strings arrays.\n");
		exit(EXIT_FAILURE);
	}

	char buffer[MAX_NAME_LENGTH];

	ParsingStatus status_illness = loadStringFile(ILLNESS_LIST_FILENAME, IllnessNumber, IllnessStringArray, buffer);

	if (status_illness != LOADING_SUCCESSFUL)
	{
		return status_illness;
	}

	ParsingStatus status_symptom = loadStringFile(SYMPTOM_LIST_FILENAME, SymptomNumber, SymptomStringArray, buffer);

	if (status_symptom != LOADING_SUCCESSFUL)
	{
		return status_symptom;
	}

	printf("\nString names loaded succesfully.\n");
	return LOADING_SUCCESSFUL;
}
