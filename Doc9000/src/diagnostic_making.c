#include <stdio.h>
#include <stdlib.h>

#include "diagnostic_making.h"
#include "parsing.h"
#include "processing.h"
#include "learning_dataset.h"
#include "prediagnostic_file.h"
#include "api.h"


static NeuralNetwork *NetworkLoaded;
static Inputs *InputsToFill;
static Diagnostic DiagnosticToFill;
static int BufferIndexGreaterValues[DIAG_ILLNESS_NUMBER];
static unsigned int WrittenDiagCount;


// Returns the count of diagnostics which were successfully
// made and written on the database, since the program started.
unsigned int getWrittenDiagnosticCount(void)
{
	return WrittenDiagCount;
}


// Returns the should-be-filled current diagnostic. In order for this to be relevant,
// a makeDiagnostic() (or a function calling it) must be issued beforehand. Do not
// try to free this ressource.
const Diagnostic* getFilledDiagnostic(void)
{
	return &DiagnosticToFill;
}


// Returns the loaded neural network. Don't forget to free it with
// a freeRecognitionRessources() call at the program's end.
const NeuralNetwork* getLoaded_NeuralNetwork(void)
{
	initRecognitionRessources(); // to be sure ressouces are loaded.

	return NetworkLoaded;
}


// Loads into memory the neural network and the Inputs struct. This can be called when
// the program starts (although it will be called automatically), and one should avoid
// restarting the whole program for each diagnostic request, as calling this function
// frequently may slow down the whole application.
void initRecognitionRessources(void)
{
	if (NetworkLoaded != NULL && InputsToFill != NULL)
		return;

	freeRecognitionRessources(); // In case only one is NULL!

	////////////////////////////////////////////////////////////
	// Loading the neural network:

	const int max_batch_size = 1; // 1 is enough here, the learning phase is over.

	NetworkLoaded = loadNetwork(NEURAL_NET_DIR_PATH, max_batch_size);

	if (NetworkLoaded == NULL)
	{
		printf("\nThe neural network could not be properly loaded.\n\n");
		exit(EXIT_FAILURE);
	}

	////////////////////////////////////////////////////////////
	// Creating an input to fill with the prediagnostic data:

	const int questions_number = 1;
	const int questions_size = network_inputSize(NetworkLoaded);
	const int answers_size = network_outputSize(NetworkLoaded);

	if (questions_size != getSymptomNumber())
	{
		printf("\nIncompatible sizes of questions size: %d vs %d\n", questions_size, getSymptomNumber());
		exit(EXIT_FAILURE);
	}

	if (answers_size != getIllnessNumber())
	{
		printf("\nIncompatible sizes of answers size: %d vs %d\n", answers_size, getIllnessNumber());
		exit(EXIT_FAILURE);
	}

	Number **questions = createMatrix(questions_number, questions_size);
	InputsToFill = createInputs(questions_number, questions_size, answers_size, questions, NULL);

	printf("Recognition ressouces were successfully loaded.\n");
}


// Frees the neural network and the Inputs struct from memory. Call this upon program exit.
void freeRecognitionRessources(void)
{
	freeInputs(&InputsToFill); // frees the question array! Careful...
	freeNetwork(&NetworkLoaded);

	// N.B: 'InputsToFill' and 'NetworkLoaded' have been reset to NULL.
}


// Whole event chain for 1 diagnostic: reads the prediagnostic file, fetches the linked
// medical record from the database, makes a diagnostic, and then writes it into the database,
// assuming there is at least one valid symptom in the given prediagnostic. Returns 1 on success,
// and 0 else. This should _not_ break the whole program in case of failure.
int diagnosticProcessing(const char *prediag_filename)
{
	PreDiagnostic *prediag = readPreDiagnosticFile(prediag_filename);

	if (prediag == NULL)
		return 0;

	if (VERBOSE_MODE >= 2)
		printPreDiagnostic(prediag);

	const int id_socdet = prediag -> id_socdet;

	MedicalRecord *medrec = readMedicalRecord(id_socdet); // returns NULL if id_socdet = 0.

	if (!makeDiagnostic(prediag, medrec)) // accepts a NULL medrec.
		goto failure;

	if (!writeDiagnostic(&DiagnosticToFill, id_socdet))
		goto failure;

	++WrittenDiagCount;

	freeMedicalRecord(&medrec);
	freePreDiagnostic(&prediag);
	return 1;

	failure:
		freeMedicalRecord(&medrec);
		freePreDiagnostic(&prediag);
		return 0;
}


// Fills a diagnostic struct from a prediagnostic and a medical record, if there is
// at least one valid symptom in the given prediagnostic. A NULL medical record can be
// given, in order to work only with the prediagnostic. Returns 1 on success, 0 else.
int makeDiagnostic(const PreDiagnostic *prediag, const MedicalRecord *medrec)
{
	initRecognitionRessources(); // to be sure ressouces are loaded.

	if (prediag == NULL)
	{
		printf("\nCannot output a new diagnostic: NULL inputs.\n");
		return 0;
	}

	if (countValidSymptoms(prediag) == 0)
	{
		printf("Cannot output a diagnostic: no valid symptom as input.\n");
		return 0;
	}

	//////////////////////////////////////////////////////
	// Feed the inputs content:

	fillQuestion(InputsToFill -> Questions[0], prediag);

	//////////////////////////////////////////////////////
	// Recognition:

	prediction(NetworkLoaded, InputsToFill);

	const Number *the_answer = InputsToFill -> Answers[0];

	// Finding the 'DIAG_ILLNESS_NUMBER' most probable illnesses:

	findGreaterValuesIndex(BufferIndexGreaterValues, DIAG_ILLNESS_NUMBER,
		the_answer, network_outputSize(NetworkLoaded));

	//////////////////////////////////////////////////////
	// Filling the Diagnostic:

	DiagnosticToFill.id_diag = 0; // Will be set automatically by the database.
	snprintf(DiagnosticToFill.date_diag, DATE_MAX_LENGTH, "%s", ""); // same.

	DiagnosticToFill.criticity = criticity(the_answer, prediag, medrec);

	for (int i = 0; i < DIAG_ILLNESS_NUMBER; ++i)
	{
		short illness_index = BufferIndexGreaterValues[i];

		DiagnosticToFill.illnessArray[i] = illness_index;
		DiagnosticToFill.illnessProbabilityArray[i] = the_answer[illness_index];
	}

	if (VERBOSE_MODE >= 2)
		printDiagnostic(&DiagnosticToFill);

	return 1;
}


// Fills the given question with the relevant data from a prediagnostic:
void fillQuestion(Number *question, const PreDiagnostic *prediag)
{
	if (!question || !prediag)
		return;

	const int symptom_number = getSymptomNumber();

	for (int i = 0; i < symptom_number; ++i)
	{
		question[i] = VALUE_ABSENT_SYMPTOM;
	}

	for (int i = 0; i < prediag -> symptomNumber; ++i)
	{
		enterSymptom(question, prediag -> declaredSymptoms[i], prediag -> declaredSymptomsConfidences[i]);
	}
}
