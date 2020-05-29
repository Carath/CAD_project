#ifndef MEDICAL_STRUCTS_H
#define MEDICAL_STRUCTS_H


#include "doc_settings.h"


///////////////////////////////////////////////////////////////
// Structs definitions:

typedef short Illness;
typedef short Symptom;


// Sort of pre-diagnostic, used only for the learning phase.
typedef struct
{
	Illness illness;
	short symptomNumber;
	Symptom *symptomArray;
} MedicalData;


typedef struct
{
	unsigned long timestamp;
	int id_socdet;
	float patientConfidenceLevel; // Between 0. and 1.
	short symptomNumber;
	Symptom *declaredSymptoms; // length: symptomNumber
	float *declaredSymptomsConfidences; // length: symptomNumber. Between 0. and 1.
} PreDiagnostic;


typedef struct
{
	int id_diag;
	char date_diag[DATE_MAX_LENGTH];
	float criticity; // Between 0. and 1.
	Illness illnessArray[DIAG_ILLNESS_NUMBER];
	float illnessProbabilityArray[DIAG_ILLNESS_NUMBER];
} Diagnostic;


// This must be freed by a freeMedicalRecord()' call:
typedef struct
{
	int id_medrec;
	short gender;
	short age;
	short id_blgrp;
	short weight;
	short height;
	short diagnosticNumber;
	Diagnostic *diagnosticArray; // length: 'diagnosticNumber'
} MedicalRecord;


/////////////////////////////////////////////////////
// PreDiagnostic functions:

// Allocates enough memory for a pre-diagnostic:
PreDiagnostic* allocatePreDiagnostic(short symptom_number);


// Frees a pre-diagnostic passed by address, and sets said address to NULL.
void freePreDiagnostic(PreDiagnostic **prediag);


// Prints a prediagnostic in the console:
void printPreDiagnostic(const PreDiagnostic *prediag);


/////////////////////////////////////////////////////
// Diagnostics functions:

// Allocates enough memory for a diagnostic:
Diagnostic* allocateDiagnostic(void);


// Frees a diagnostic passed by address, and sets said address to NULL.
void freeDiagnostic(Diagnostic **diag);


// Prints a diagnostic in the console:
void printDiagnostic(const Diagnostic *diag);


/////////////////////////////////////////////////////
// Medical records functions:

// Allocates enough memory for a medical record:
MedicalRecord* allocateMedicalRecord(short diagnostic_number);


// Frees a medical record passed by address, and sets said address to NULL.
void freeMedicalRecord(MedicalRecord **medrec);


// Prints a medical record in the console:
void printMedicalRecord(const MedicalRecord *medrec);


#endif
