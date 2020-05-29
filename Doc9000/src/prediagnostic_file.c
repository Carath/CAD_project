#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "prediagnostic_file.h"
#include "parsing.h"


static const short Static_magic_number = MAGIC_NUMBER;
static char Generated_filename[MAX_FILENAME_PATH_LENGTH];


// Useful only for testing purpose. Do _not_ free the result, as it is static.
const char* lastGeneratedPreDiagnosticFilename(void)
{
	return Generated_filename;
}


// Will generate a proper filename automatically. Last generated filename
// is stored in 'Generated_filename'. Returns 1 on success, 0 else.
int writePreDiagnosticFile(const PreDiagnostic *prediag)
{
	if (prediag == NULL)
		return 0;

	sprintf(Generated_filename, "%s"PREDIAGS_FILENAME_FORMAT,
		PREDIAGS_SRC_FOLDER, prediag -> id_socdet, prediag -> timestamp);

	FILE *file = fopen(Generated_filename, "wb");

	if (file == NULL)
	{
		printf("\nCould not write to: '%s'.\n", Generated_filename);
		return 0;
	}

	short convertedPatientConfidenceLevel = CONVERSION_COEFF * prediag -> patientConfidenceLevel;

	fwrite(&Static_magic_number, sizeof(short), 1, file);
	fwrite(&(prediag -> timestamp), sizeof(unsigned long), 1, file);
	fwrite(&(prediag -> id_socdet), sizeof(int), 1, file);
	fwrite(&convertedPatientConfidenceLevel, sizeof(short), 1, file);
	fwrite(&(prediag -> symptomNumber), sizeof(short), 1, file);

	for (int i = 0; i < prediag -> symptomNumber; ++i)
	{
		if (prediag -> declaredSymptoms[i] != no_symptom)
		{
			short convertedDeclaredSymptomsConfidences = CONVERSION_COEFF * prediag -> declaredSymptomsConfidences[i];

			fwrite(prediag -> declaredSymptoms + i, sizeof(Symptom), 1, file);
			fwrite(&convertedDeclaredSymptomsConfidences, sizeof(short), 1, file);
		}
	}

	fclose(file);

	return 1;
}


// Reads a prediagnostic file, and returns a new prediagnostic on success,
// and NULL on failure. This will require a freePreDiagnostic() call afterhand.
PreDiagnostic* readPreDiagnosticFile(const char *filename)
{
	if (CHECK_PREDIAG_FILENAMES && !prediagFilenameCheck(filename))
		return NULL;

	FILE *file = fopen(filename, "rb");

	if (file == NULL)
	{
		printf("\nCould not open file: '%s'.\n", filename);
		return NULL;
	}

	short magic_number;
	unsigned long timestamp;
	int id_socdet;
	short toConvertPatientConfidenceLevel;
	short symptomNumber;

	if (fread(&magic_number, sizeof(short), 1, file) != 1)
	{
		printf("\nCould not read 'magic_number'.\n");
		goto failure;
	}

	if (magic_number != Static_magic_number)
	{
		printf("\nWrong endianness or unsupported file.\n");
		goto failure;
	}

	if (fread(&timestamp, sizeof(unsigned long), 1, file) != 1)
	{
		printf("Could not read 'timestamp'.\n");
		goto failure;
	}

	if (fread(&id_socdet, sizeof(int), 1, file) != 1)
	{
		printf("Could not read 'id_socdet'.\n");
		goto failure;
	}

	if (fread(&toConvertPatientConfidenceLevel, sizeof(short), 1, file) != 1)
	{
		printf("Could not read 'toConvertPatientConfidenceLevel'.\n");
		goto failure;
	}

	if (fread(&symptomNumber, sizeof(short), 1, file) != 1)
	{
		printf("Could not read 'symptomNumber'.\n");
		goto failure;
	}

	symptomNumber = MAX(0, symptomNumber); // to be sure.

	PreDiagnostic *prediag = allocatePreDiagnostic(symptomNumber);

	if (prediag == NULL)
	{
		printf("Failure: prediagnostic not filled.\n");
		goto failure;
	}

	prediag -> timestamp = timestamp;
	prediag -> id_socdet = id_socdet;
	prediag -> patientConfidenceLevel = (float) toConvertPatientConfidenceLevel / CONVERSION_COEFF;
	prediag -> symptomNumber = symptomNumber;

	const short total_symptom_number = getSymptomNumber();

	for (int i = 0; i < symptomNumber; ++i)
	{
		if (fread(prediag -> declaredSymptoms + i, sizeof(Symptom), 1, file) != 1)
		{
			printf("Could not read 'declaredSymptoms[%d]'.\n", i);
			freePreDiagnostic(&prediag);
			goto failure;
		}

		if (prediag -> declaredSymptoms[i] < 0 || prediag -> declaredSymptoms[i] >= total_symptom_number) // to be sure.
			prediag -> declaredSymptoms[i] = no_symptom;

		short toConvertDeclaredSymptomsConfidences;

		if (fread(&toConvertDeclaredSymptomsConfidences, sizeof(short), 1, file) != 1)
		{
			printf("Could not read 'declaredSymptomsConfidences[%d]'.\n", i);
			freePreDiagnostic(&prediag);
			goto failure;
		}

		prediag -> declaredSymptomsConfidences[i] = (float) toConvertDeclaredSymptomsConfidences / CONVERSION_COEFF;
	}

	fclose(file);
	return prediag;

	failure:
		fclose(file);
		return NULL;
}


// Checks the filenames of the prediagnostics files, which must be located in 'PREDIAGS_SRC_FOLDER'.
int prediagFilenameCheck(const char *filename)
{
	if (!filename)
		return 0;

	char full_path_format[] = PREDIAGS_SRC_FOLDER PREDIAGS_FILENAME_FORMAT;

	const int start_check_length = 8, end_check_length = 21;
	const int len = strlen(filename);
	const int end_filename = MIN(len, end_check_length);

	int start_res = strncmp(full_path_format, filename, start_check_length + sizeof(PREDIAGS_SRC_FOLDER) - 1) == 0;
	int end_res = strcmp(full_path_format + sizeof(full_path_format) - 1 - end_check_length, filename + len - end_filename) == 0;

	if (VERBOSE_MODE >= 2 && !(start_res && end_res))
		printf("Incorrect prediagnostic filename: '%s'.\n", filename);

	return start_res && end_res;
}
