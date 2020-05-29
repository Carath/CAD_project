#ifndef DIAGNOSTIC_MAKING_H
#define DIAGNOSTIC_MAKING_H


#include "medical_structs.h"


// Returns the count of diagnostics which were successfully
// made and written on the database, since the program started.
unsigned int getWrittenDiagnosticCount(void);


// Returns the should-be-filled current diagnostic. In order for this to be relevant,
// a makeDiagnostic() (or a function calling it) must be issued beforehand. Do not
// try to free this ressource.
const Diagnostic* getFilledDiagnostic(void);


// Returns the loaded neural network. Don't forget to free it with
// a freeRecognitionRessources() call at the program's end.
const NeuralNetwork* getLoaded_NeuralNetwork(void);


// Loads into memory the neural network and the Inputs struct. This can be called when
// the program starts (although it will be called automatically), and one should avoid
// restarting the whole program for each diagnostic request, as calling this function
// frequently may slow down the whole application.
void initRecognitionRessources(void);


// Frees the neural network and the Inputs struct from memory. Call this upon program exit.
void freeRecognitionRessources(void);


// Whole event chain for 1 diagnostic: reads the prediagnostic file, fetches the linked
// medical record from the database, makes a diagnostic, and then writes it into the database,
// assuming there is at least one valid symptom in the given prediagnostic. Returns 1 on success,
// and 0 else. This should _not_ break the whole program in case of failure.
int diagnosticProcessing(const char *prediag_filename);


// Fills a diagnostic struct from a prediagnostic and a medical record, if there is
// at least one valid symptom in the given prediagnostic. A NULL medical record can be
// given, in order to work only with the prediagnostic. Returns 1 on success, 0 else.
int makeDiagnostic(const PreDiagnostic *prediag, const MedicalRecord *medrec);


// Fills the given question with the relevant data from a prediagnostic:
void fillQuestion(Number *question, const PreDiagnostic *prediag);


#endif
