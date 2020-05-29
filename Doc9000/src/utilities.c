#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utilities.h"
#include "doc_settings.h"


// Reads an entire text file, and returns it as a string:
char* readFileContent(char *filename)
{
	const long int char_number = getFileSize(filename);

	char *string_result = (char*) calloc(char_number, sizeof(char));

	if (string_result == NULL)
	{
		printf("Not enough memory to hold the content of file: %s\n", filename);
		return NULL;
	}

	FILE *file = fopen(filename, "r");

	if (file == NULL)
	{
		printf("File '%s' not found.\n", filename);
		return NULL;
	}

	if (char_number != fread(string_result, sizeof(char), char_number, file))
	{
		printf("An error happened during the reading of the file: %s\n", filename);
		fclose(file);
		free(string_result);
		return NULL;
	}

	fclose(file);

	return string_result;
}


char** createCharMatrix(int rows, int cols)
{
	char **matrix = (char**) calloc(rows, sizeof(char*));

	if (matrix == NULL)
	{
		printf("\nNot enough memory to create a new matrix.\n");
		return NULL;
	}

	for (int i = 0; i < rows; ++i)
	{
		matrix[i] = (char*) calloc(cols, sizeof(char));

		if (matrix[i] == NULL)
		{
			printf("\nNot enough memory to create a new vector.\n");
			freeCharMatrix(&matrix, rows);
			return NULL;
		}
	}

	return matrix;
}


// Free the matrix pointed by the given adress, and set it to NULL:
void freeCharMatrix(char ***matrix, int rows)
{
	if (matrix == NULL || *matrix == NULL)
		return;

	for (int i = 0; i < rows; ++i)
	{
		free((*matrix)[i]);
	}

	free(*matrix);
	*matrix = NULL;
}


// Returns the index of a string in the given array if it exists, or 'bound' otherwise.
// 'bound' needs to not be greater than the array length.
int getStringIndex(const char *string, char* const *string_array, int bound)
{
	int i = 0;

	while (i < bound && strcmp(string, string_array[i]) != 0)
		++i;

	return i;
}
