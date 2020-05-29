#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "parsing.h"
#include "learning_dataset.h"
#include "learning_phase.h"
#include "api.h"
#include "diagnostic_making.h"
#include "event_loop.h"
#include "demos.h"


void runtime(void);
void unitTests(void);


int main(void)
{
	// srand(time(NULL)); // Initialization of the pseudo-random number generator.

	/////////////////////////////////////////////////////////////
	// Learning phase. This may be slow without OpenBLAS for large neural nets:

	// learningPhase();

	/////////////////////////////////////////////////////////////
	// Testing:

	// unitTests();

	/////////////////////////////////////////////////////////////
	// Runtime:

	runtime();

	return EXIT_SUCCESS;
}


// Run this for Doc9000 to function:
void runtime(void)
{
	// Setup:

	if (!connectToDatabase())
		exit(EXIT_FAILURE);

	checkBackupsIntegrity();

	freeStringsArrays();

	initRecognitionRessources();

	// Main loop:

	diagnosticEventLoop();

	// Disconnect from the database, and free static ressources:

	disconnectFromDatabase();
	freeCriticityArray();
	freeRecognitionRessources();

	printf("Quitting.\n");
}


// Runs several tests to check on Doc9000 proper working.
// The test functions used here are found in the 'demos.c' file.
void unitTests(void)
{
	int failure_number = 0;

	failure_number += !testPassword();

	failure_number += !demoParsingSuccess();

	failure_number += !testStructures();

	failure_number += !test_WriteAndRead_PreDiagnosticFile();

	failure_number += !testDiagnosticProduction();

	failure_number += !checkBackupsIntegrity();

	failure_number += !testReadMedicalRecord(33); // several diagnostics on the local database.

	failure_number += !testReadMedicalRecord(36); // no past diagnostic on the local database.

	failure_number += !testWriteDiagnostic(1);

	// Disconnect from the database, and free static ressources:

	disconnectFromDatabase();

	freeCriticityArray();
	freeStringsArrays();
	freeDatasetAssets();

	freeRecognitionRessources();

	printf("\n -> Unit tests done. Failure number: %d\n", failure_number);
}
