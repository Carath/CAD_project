#ifndef PROCESSING_H
#define PROCESSING_H


#include "medical_structs.h"


float getBMI_index(short weight, short height);


int getAge(const char *birthday);


// Can be used to make sure each criticity is between 0. and 1.
float boundCriticity(float criticity);


// Gives an estimate of the criticity level, between 0 and 1:
// 'confidenceArray' must be of length getIllnessNumber().
// Also, even if optional, the patient prediagnostic or his medical record can be passed as arguments.
float criticity(const Number *confidenceArray, const PreDiagnostic *prediag, const MedicalRecord *medrec);


// Counts the number of valid symptoms in the given prediagnostic:
int countValidSymptoms(const PreDiagnostic *prediag);


// Returns a scalar coefficient made from the valid symptoms number, to be applied onto the criticity.
float scale_byValidSymptomNumber(const PreDiagnostic *prediag);


// Returns a scalar coefficient made from the patient confidence level, to be applied onto the criticity.
float scale_byPatientConfidenceLevel(const PreDiagnostic *prediag);


// Returns a scalar coefficient made from the BMI index, to be applied onto the criticity.
float scale_byBMI_index(const MedicalRecord *medrec);


// Returns a scalar coefficient made from the patient age, to be applied onto the criticity.
float scale_byAge(const MedicalRecord *medrec);


// Given an input value 'x', returns a number between 'y_min' and 'y_max'.
// Specifically, returns 'y_min' when 'x' is lower than 'x_a', 'y_max' when 'x' is greater than 'x_b',
// and does a linear regression between the two, as to have a continuous transformation.
float scaling_half(float x, float x_a, float x_b, float y_min, float y_max);


// Given an input value 'x', returns a number between 'y_min' and 'y_max'.
// Specifically, returns 'y_max' when 'x' is lower than 'x_a' or greater than 'x_d',
// 'y_min' when 'x' is between 'x_b' and 'x_c', and does linear regressions for the other cases,
// as to have a continuous transformation.
float scaling_by_thresholds(float x, float x_a, float x_b, float x_c, float x_d, float y_min, float y_max);


#endif
