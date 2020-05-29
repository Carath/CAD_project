#ifndef LEARNING_DATASET_H
#define LEARNING_DATASET_H


#include "medical_structs.h"


// Register a symptom in the given question. Returns 1 on success.
// 'value' must be used according to the following pattern:
// Symptom detected, no specific value attached to it -> 1.
// Symptom detected, with a specific value attached (e.g temperature): between 0 and 1.5 depending on the severity.
// Symptom not tested, information missing -> 0.
// Symptom refuted -> -1.
int enterSymptom(Number *question, Symptom symptom, Number value);


// Generate a noisy value:
Number generateValue(void);


// Adds a given quantity of the given illness into the learning dataset:
void addIllnesses(Number **questions, Number **answers, const MedicalData *data, int dataQuantity);


// Creating a demo of a learning_dataset:
Inputs* createDataset(int inputs_number);


// Fetch the json file to create the base dataset, assuming it hasn't been done already.
void initDatasetAssets(void);


// This must only be done when the dataset has been created, and no learning is to be done.
void freeDatasetAssets(void);


// Printing the (quite long) base dataset, which is used to create the learning dataset:
void printBaseDataset(void);


#endif
