#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "demos.h"
#include "learning_dataset.h"
#include "parsing.h"
#include "api.h"
#include "auth.h"
#include "prediagnostic_file.h"
#include "diagnostic_making.h"


#define ADD_SEPARATOR() \
	printf("---------------------------------------------------------------------\n")


// Checking the creation, and reading of a password file:
int testPassword(void)
{
	ADD_SEPARATOR();
	printf("-> Checking the creation, and reading of a password file:\n");

	createPassword(AUTH_EXPL_FILE, 24);

	// This will need to be protected against running without admin rights:
	char *password = readPassword(AUTH_EXPL_FILE);

	int result = password != NULL;

	printf("\nTesting a password creation: %s\n\n", password);

	cleanAndFreePassword(&password);

	if (!result)
		printf("-> FAILED test: 'testPassword'.\n");

	return result;
}


// Checking the initial parsing success:
int demoParsingSuccess(void)
{
	ADD_SEPARATOR();
	printf("-> Checking the initial parsing success:\n");

	short illness_number = getIllnessNumber();
	short symptom_number = getSymptomNumber();

	printf("\nIllness number: %d\nSymptom number: %d\n", illness_number, symptom_number);

	printf("\nFirst illness: '%s'\n", getIllnessName(0));
	printf("\nFirst symptom: '%s'\n", getSymptomName(0));

	int result = illness_number > 0 && symptom_number > 0;

	if (!result)
		printf("-> FAILED test: 'demoParsingSuccess'.\n");

	return result;
}


// Printing several medical structs:
int testStructures(void)
{
	ADD_SEPARATOR();
	printf("-> Printing several medical structs:\n");

	// Creating some diagnostics:

	Diagnostic diag_1 =
	{
		.id_diag = 130,
		.criticity = 0.9,
		.date_diag = "2020-05-04",
		.illnessArray = {15, 3, 2, 7, 20},
		.illnessProbabilityArray = {0.1, 0.4, 0.2, 0., 0.3}
	};

	Diagnostic diag_2 =
	{
		.id_diag = 6,
		.criticity = 0.1,
		.date_diag = "2020-04-01",
		.illnessArray = {20, 1, 12, 5, 3},
		.illnessProbabilityArray = {0.1, 0.5, 0.05, 0., 0.1}
	};

	Diagnostic diag_3 =
	{
		.id_diag = 77,
		.criticity = 0.51,
		.date_diag = "2019-01-01",
		.illnessArray = {2, 8, 33, 17, 51},
		.illnessProbabilityArray = {0.3, 0.1, 0., 0., 0.5}
	};

	ARRAYS_COMPARE_LENGTH(diag_1.illnessArray, diag_1.illnessProbabilityArray);
	ARRAYS_COMPARE_LENGTH(diag_2.illnessArray, diag_2.illnessProbabilityArray);
	ARRAYS_COMPARE_LENGTH(diag_3.illnessArray, diag_3.illnessProbabilityArray);

	// Now creating a medical record:

	Diagnostic diagnosticArray[] = {diag_1, diag_2, diag_3};

	MedicalRecord medrec =
	{
		.id_medrec = 55,
		.gender = 1,
		.age = 42,
		.id_blgrp = 2,
		.diagnosticNumber = ARRAY_LENGTH(diagnosticArray),
		.diagnosticArray = diagnosticArray
	};

	printMedicalRecord(&medrec);

	return 1;
}


// Trying to write a handmade prediagnotic to a file, and then read it:
int test_WriteAndRead_PreDiagnosticFile(void)
{
	ADD_SEPARATOR();
	printf("-> Trying to write a handmade prediagnotic to a file, and then read it:\n");

	Symptom declaredSymptoms[] = {13, 25, 6};
	float declaredSymptomsConfidences[] = {0.5, 0.1, 0.9};

	PreDiagnostic prediag =
	{
		// .timestamp = time(NULL), // real case.
		.timestamp = 1234567890,
		.id_socdet = 42,
		.patientConfidenceLevel = 1.,
		.symptomNumber = ARRAYS_COMPARE_LENGTH(declaredSymptoms, declaredSymptomsConfidences),
		.declaredSymptoms = declaredSymptoms,
		.declaredSymptomsConfidences = declaredSymptomsConfidences
	};

	printPreDiagnostic(&prediag);

	int result = writePreDiagnosticFile(&prediag);

	printf("\nDone writing.\n");

	// Fetching the last generated filename:
	const char *filename = lastGeneratedPreDiagnosticFilename();

	printf("\nLast created file: %s\n", filename);

	PreDiagnostic *read_prediag = readPreDiagnosticFile(filename);

	printf("\nDone reading.\n");

	printPreDiagnostic(read_prediag);

	result += read_prediag != NULL && read_prediag -> id_socdet == 42;

	freePreDiagnostic(&read_prediag);

	if (result < 2)
		printf("-> FAILED test: 'test_WriteAndRead_PreDiagnosticFile'.\n");

	return result == 2 ? 1 : 0;
}


// Trying to make a diagnostic from a handmade prediagnostic:
int testDiagnosticProduction(void)
{
	ADD_SEPARATOR();
	printf("-> Trying to make a diagnostic from a handmade prediagnostic:\n");

	//////////////////////////////////////////////////////
	// A handmade prediagnostic:

	Illness illness_to_predict = getIllnessID("chronic_obstructive_pulmonary_disease");

	Symptom declaredSymptoms[] =
	{
		getSymptomID("cough"),
		getSymptomID("wheezing"),
		getSymptomID("dyspnea"),
		getSymptomID("shortness_of_breath"),
		getSymptomID("hypoxemia")
	};

	float declaredSymptomsConfidences[] = {1., 1., 1., 1., 1.};

	PreDiagnostic prediag =
	{
		.timestamp = time(NULL),
		.id_socdet = 51,
		.patientConfidenceLevel = 1.,
		.symptomNumber = ARRAYS_COMPARE_LENGTH(declaredSymptoms, declaredSymptomsConfidences),
		.declaredSymptoms = declaredSymptoms,
		.declaredSymptomsConfidences = declaredSymptomsConfidences
	};

	printPreDiagnostic(&prediag);

	//////////////////////////////////////////////////////
	// New diagnostic being made:

	if (!makeDiagnostic(&prediag, NULL)) // A NULL MedicalRecord is valid.
	{
		ADD_SEPARATOR();
		printf("-> FAILED test: 'testDiagnosticProduction' (could not make a diagnostic).\n");
		return 0;
	}

	const Diagnostic *filled_diag = getFilledDiagnostic();

	printDiagnostic(filled_diag);

	ADD_SEPARATOR();

	int predicted_illness = filled_diag -> illnessArray[0];
	float predicted_probability = filled_diag -> illnessProbabilityArray[0];

	int result = predicted_illness == illness_to_predict && predicted_probability > 0.80f;

	if (!result)
		printf("-> FAILED test: 'testDiagnosticProduction' (prediction of poor quality).\n");

	return result;
}


// Trying to read a diagnostic from the database:
int testReadDiagnostic(int id_diag)
{
	ADD_SEPARATOR();
	printf("-> Reading the diagnostic of 'id_diag' %d:\n", id_diag);

	Diagnostic read_diag = {0};

	int result = readDiagnostic(&read_diag, id_diag);

	printDiagnostic(&read_diag);

	if (!result)
		printf("-> FAILED test: 'testReadDiagnostic'.\n");

	return result;
}


// Trying to read a medical record from the database:
int testReadMedicalRecord(int id_socdet)
{
	ADD_SEPARATOR();
	printf("-> Reading the medical of record, for the patient of 'id_socdet' %d:\n", id_socdet);

	MedicalRecord *medrec = readMedicalRecord(id_socdet);

	int result = medrec != NULL;

	printMedicalRecord(medrec);

	freeMedicalRecord(&medrec);

	if (!result)
		printf("-> FAILED test: 'testReadMedicalRecord'.\n");

	return result;
}


// Tries to write a diagnostic to the local database,
// does nothing on the real one, to not clutter it.
int testWriteDiagnostic(int id_socdet)
{
	ADD_SEPARATOR();

	#ifndef LOCAL_TESTING
		printf("-> Not trying to write a diagnostic on the _real_ database. Use the _local_ one for this.\n");
		ADD_SEPARATOR();
		return 1;
	#endif

	printf("-> Trying to write a diagnostic to the _local_ database:\n");

	Diagnostic diag =
	{
		.id_diag = 0,
		.criticity = 0.9,
		.date_diag = "",
		.illnessArray = {15, 3, 2, 7, 20},
		.illnessProbabilityArray = {0.1, 0.4, 0.2, 0., 0.3}
	};

	ARRAYS_COMPARE_LENGTH(diag.illnessArray, diag.illnessProbabilityArray);

	printDiagnostic(&diag);

	int new_id = writeDiagnostic(&diag, id_socdet);

	if (new_id == 0)
	{
		printf("-> FAILED test: 'testWriteDiagnostic' (writing to the database failed).\n");
		ADD_SEPARATOR();
		return 0;
	}

	else
	{
		printf("\nSuccessful writing to the database (illnesses order may be changed):\n");

		int result = testReadDiagnostic(new_id);

		ADD_SEPARATOR();
		return result;
	}
}
