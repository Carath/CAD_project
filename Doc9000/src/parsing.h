#ifndef PARSING_H
#define PARSING_H


#include "medical_structs.h"


typedef enum
{
	RESSOURCES_EXIST,
	REPARSING,
	PARSING_SUCCESSFUL,
	PARSING_FAILED,
	SAVING_SUCCESSFUL,
	LOADING_SUCCESSFUL
} ParsingStatus;


///////////////////////////////////////////////////////////////
// Getters:


// Returns the number of supported illnesses:
short getIllnessNumber(void);


// Returns the number of supported symptoms:
short getSymptomNumber(void);


// Get the name of a Illness:
const char* getIllnessName(Illness illness);


// Get the name of a Symptom:
const char* getSymptomName(Symptom symptom);


// Get an Illness from its name. Returns 0 by default:
Illness getIllnessID(const char *string);


// Get a Symptom from its name. Returns 0 by default:
Symptom getSymptomID(const char *string);


// Returns the array containing each illness criticity:
const float* getCriticityArray(void);


///////////////////////////////////////////////////////////////
// Freeing:


// Frees and resets to NULL.
void freeCriticityArray(void);


// Frees and resets to NULL.
void freeStringsArrays(void);


// Free the medical data array pointed by the given adress, and set it to NULL.
// 'data_number' needs to be a parameter instead of an hardcoded 'IllnessNumber', for freeMedicalData().
void freeMedicalData(MedicalData **medData, int data_number);


///////////////////////////////////////////////////////////////
// Parsing:


// Parse the base dataset, in order to generate the ilness and symptoms lists, along with the criticity file.
// If called with a non-NULL medData, it will be filled with an array of medical data used for the learning phase.
ParsingStatus parseBaseDataset(MedicalData **medData);


#endif
