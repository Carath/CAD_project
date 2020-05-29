#include <stdio.h>
#include <stdlib.h>

#include "medical_structs.h"
#include "parsing.h"


static const char* GenderString[] = {"male", "female"};


/////////////////////////////////////////////////////
// PreDiagnostic functions:


// Allocates enough memory for a pre-diagnostic:
PreDiagnostic* allocatePreDiagnostic(short symptom_number)
{
	if (symptom_number < 0)
		return NULL;

	PreDiagnostic *prediag = (PreDiagnostic*) calloc(1, sizeof(PreDiagnostic));

	if (prediag == NULL)
	{
		printf("\nNot enough memory to create a new prediagnostic.\n");
		exit(EXIT_FAILURE);
	}

	prediag -> symptomNumber = symptom_number;

	if (symptom_number == 0)
	{
		prediag -> declaredSymptoms = NULL;
		prediag -> declaredSymptomsConfidences = NULL;
	}

	else
	{
		prediag -> declaredSymptoms = (Symptom*) calloc(symptom_number, sizeof(Symptom));
		prediag -> declaredSymptomsConfidences = (float*) calloc(symptom_number, sizeof(float));

		if (prediag -> declaredSymptoms == NULL || prediag -> declaredSymptomsConfidences == NULL)
		{
			printf("\nNot enough memory to create the prediagnostic' symptoms array.\n");
			freePreDiagnostic(&prediag);
			return NULL;
		}
	}

	return prediag;
}


// Frees a pre-diagnostic passed by address, and sets said address to NULL.
void freePreDiagnostic(PreDiagnostic **prediag)
{
	if (prediag == NULL || *prediag == NULL)
		return;

	free((*prediag) -> declaredSymptomsConfidences);
	free((*prediag) -> declaredSymptoms);
	free(*prediag);
	*prediag = NULL;
}


// Prints a prediagnostic in the console:
void printPreDiagnostic(const PreDiagnostic *prediag)
{
	if (prediag == NULL)
	{
		printf("\nNULL prediagnostic.\n");
		return;
	}

	printf("\nPrediagnostic:\n - Timestamp: %ld\n - id_socdet: %d\n - Patient confidence level: %.3f\n - Symptoms number: %d\n\n",
		prediag -> timestamp, prediag -> id_socdet, prediag -> patientConfidenceLevel, prediag -> symptomNumber);

	if (prediag -> symptomNumber > 0 && (prediag -> declaredSymptoms == NULL || prediag -> declaredSymptomsConfidences == NULL))
	{
		printf("NULL array!\n\n");
		return;
	}

	for (int i = 0; i < prediag -> symptomNumber; ++i)
	{
		printf("  %2d) %4d  ->  %.3f  (%s)\n", i, prediag -> declaredSymptoms[i],
			prediag -> declaredSymptomsConfidences[i], getSymptomName(prediag -> declaredSymptoms[i]));
	}

	printf("\n");
}


/////////////////////////////////////////////////////
// Diagnostics functions:


// Allocates enough memory for a diagnostic:
Diagnostic* allocateDiagnostic(void)
{
	Diagnostic *diag = (Diagnostic*) calloc(1, sizeof(Diagnostic));

	if (diag == NULL)
	{
		printf("\nNot enough memory to create a new diagnostic.\n");
		exit(EXIT_FAILURE);
	}

	return diag;
}


// Frees a diagnostic passed by address, and sets said address to NULL.
void freeDiagnostic(Diagnostic **diag)
{
	if (diag == NULL || *diag == NULL)
		return;

	free(*diag);
	*diag = NULL;
}


// Prints a diagnostic in the console:
void printDiagnostic(const Diagnostic *diag)
{
	if (diag == NULL)
	{
		printf("\nNULL diagnostic.\n");
		return;
	}

	printf("\nDiagnostic:\n - id_diag: %d\n - date_diag: %s\n - Criticity: %.3f\n - Illness number: %d\n - Illnesses:\n\n",
		diag -> id_diag, diag -> date_diag, diag -> criticity, DIAG_ILLNESS_NUMBER);

	if (diag -> illnessArray == NULL || diag -> illnessProbabilityArray == NULL)
	{
		printf("NULL array!\n\n");
		return;
	}

	for (int i = 0; i < DIAG_ILLNESS_NUMBER; ++i)
	{
		printf("  %2d) %4d  ->  %.3f  (%s)\n", i, diag -> illnessArray[i],
			diag -> illnessProbabilityArray[i], getIllnessName(diag -> illnessArray[i]));
	}

	printf("\n");
}


/////////////////////////////////////////////////////
// Medical records functions:


// Allocates enough memory for a medical record:
MedicalRecord* allocateMedicalRecord(short diagnostic_number)
{
	if (diagnostic_number < 0)
		return NULL;

	MedicalRecord *medrec = (MedicalRecord*) calloc(1, sizeof(MedicalRecord));

	if (medrec == NULL)
	{
		printf("\nNot enough memory to create a new medical record.\n");
		return NULL;
	}

	medrec -> diagnosticNumber = diagnostic_number;

	if (diagnostic_number == 0)
		medrec -> diagnosticArray = NULL;

	else
	{
		medrec -> diagnosticArray = (Diagnostic*) calloc(diagnostic_number, sizeof(Diagnostic));

		if (medrec -> diagnosticArray == NULL)
		{
			printf("\nNot enough memory to create a new diagnostic array.\n");
			freeMedicalRecord(&medrec);
			return NULL;
		}
	}

	return medrec;
}


// Frees a medical record passed by address, and sets said address to NULL.
void freeMedicalRecord(MedicalRecord **medrec)
{
	if (medrec == NULL || *medrec == NULL)
		return;

	free((*medrec) -> diagnosticArray);
	free(*medrec);
	*medrec = NULL;
}


// Prints a medical record in the console:
void printMedicalRecord(const MedicalRecord *medrec)
{
	if (medrec == NULL)
	{
		printf("\nNULL medical record.\n");
		return;
	}

	const short gender_index = MAX(0, MIN(1, medrec -> gender));

	printf("\nMedical record\n - id_medrec: %d\n - Gender: %s\n - Age: %d\n - id_blgrp: %d\n"
		" - Weight: %d (kg)\n - Height: %d (cm)\n - Diagnostic number: %d\n\nDiagnostics:\n\n",
		medrec -> id_medrec, GenderString[gender_index], medrec -> age, medrec -> id_blgrp,
		medrec -> weight, medrec -> height, medrec -> diagnosticNumber);

	if (medrec -> diagnosticNumber > 0 && medrec -> diagnosticArray == NULL)
	{
		printf("NULL array!\n\n");
		return;
	}

	for (int i = 0; i < medrec -> diagnosticNumber; ++i)
	{
		printf("%d)", i);
		printDiagnostic(medrec -> diagnosticArray + i);
	}

	printf("\n");
}
