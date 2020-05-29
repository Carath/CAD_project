#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h> // for isalnum()

#include "auth.h"


// For testing purpose:
void createPassword(const char *filename, int length)
{
	if (length < 3)
	{
		printf("\nIncorrect length for a password.\n");
		return;
	}

	if (filename == NULL)
	{
		printf("\nInvalid filename.\n");
		return;
	}

	srand(time(NULL)); // Initializing the RNG.
	// This is necessary in order to generate a good password!!!

	FILE *file = fopen(filename, "wb");

	if (file == NULL)
	{
		printf("\nCould not write to: '%s'.\n", filename);
		exit(EXIT_FAILURE);
	}

	int i = 0;
	char char_rand = 0;

	while (i < length)
	{
		char_rand = rand() % 128;

		if (isalnum(char_rand) == 0) // checks for an alphanumeric character.
			continue;

		fwrite(&char_rand, sizeof(char), 1, file);
		++i;
	}

	fclose(file);
}


// This may need to be protected against running without admin rights,
// and will require a cleanAndFreePassword() call afterhand.
char* readPassword(const char *filename)
{
	if (filename == NULL)
	{
		printf("\nInvalid filename.\n");
		return NULL;
	}

	FILE *file = fopen(filename, "rb");

	if (file == NULL)
	{
		printf("\nCannot find file: %s\n", filename);
		return NULL;
	}

	// Getting the file size:

	fseek(file, 0L, SEEK_END);
	long int length = ftell(file);

	char *password = (char*) calloc(length + 1, sizeof(char));

	if (password == NULL)
	{
		printf("\nNot enough memory to store a password.\n");
		fclose(file);
		return NULL;
	}

	rewind(file);

	// Reading:

	if (fread(password, sizeof(char), length, file) != length)
	{
		printf("\nReading error!\n");
		cleanAndFreePassword(&password);
		fclose(file);
		return NULL;
	}

	fclose(file);

	// printf("\nPassword: %s\n\n", password);

	return password;
}


// Password passed by address:
void cleanAndFreePassword(char **password)
{
	if (password == NULL || *password == NULL)
		return;

	int i = 0;

	while ((*password)[i] != '\0')
	{
		(*password)[i] = '\0';
		++i;
	}

	free(*password);
	*password = NULL;
}
