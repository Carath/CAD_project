#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> // for getAge()

#include "processing.h"
#include "parsing.h"


float getBMI_index(short weight, short height)
{
	return 10000.f * weight / ((float) height * height + EPSILON);
}


int getAge(const char *birthday)
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	// printf("Date: %d-%02d-%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

	int birth_y = atoi(birthday);
	int birth_m = atoi(birthday + 5);
	int birth_d = atoi(birthday + 8);

	return ((tm.tm_year + 1900 - birth_y) * 10000 + (tm.tm_mon + 1 - birth_m) * 100 + tm.tm_mday - birth_d) / 10000;
}


// Can be used to make sure each criticity is between 0. and 1.
float boundCriticity(float criticity)
{
	if (criticity < 0.f)
		return 0.f;

	if (criticity > 1.f)
		return 1.f;

	return criticity;
}


// Gives an estimate of the criticity level, between 0 and 1:
// 'confidenceArray' must be of length getIllnessNumber().
// Also, even if optional, the patient prediagnostic or his medical record can be passed as arguments.
float criticity(const Number *confidenceArray, const PreDiagnostic *prediag, const MedicalRecord *medrec)
{
	if (confidenceArray == NULL)
	{
		printf("\nCannot output a criticity: NULL confidence array.\n");
		return 0.f;
	}

	const short illness_number = getIllnessNumber();
	const float *criticityArray = getCriticityArray();

	double crit = 0; // double internal precision.

	for (int illness = 0; illness < illness_number; ++illness)
	{
		if (confidenceArray[illness] >= CONFIDENCE_NOISE_THRESHOLD)
			crit += criticityArray[illness] * confidenceArray[illness];
	}

	// Controlled by values in 'doc_settings.h':
	crit *= scale_byValidSymptomNumber(prediag);
	crit *= scale_byPatientConfidenceLevel(prediag);
	crit *= scale_byBMI_index(medrec);
	crit *= scale_byAge(medrec);

	return boundCriticity(crit);
}


// Counts the number of valid symptoms in the given prediagnostic:
int countValidSymptoms(const PreDiagnostic *prediag)
{
	if (prediag == NULL || prediag -> declaredSymptoms == NULL)
		return 0;

	int count = 0;

	for (int i = 0; i < prediag -> symptomNumber; ++i)
	{
		count += prediag -> declaredSymptoms[i] != no_symptom;
	}

	return count;
}


// Returns a scalar coefficient made from the valid symptoms number, to be applied onto the criticity.
float scale_byValidSymptomNumber(const PreDiagnostic *prediag)
{
	if (!ENABLE_VALID_SYMPT_NUMBER_CRIT_SCALING || !prediag)
		return 1.f;

	int valid_symptom_number = countValidSymptoms(prediag);

	return scaling_half(valid_symptom_number, 0.f, SYMPTOM_NUMBER_THRESHOLD, VSN_MIN_CRITCOEFF, VSN_MAX_CRITCOEFF);
}


// Returns a scalar coefficient made from the patient confidence level, to be applied onto the criticity.
float scale_byPatientConfidenceLevel(const PreDiagnostic *prediag)
{
	if (!ENABLE_PATIENT_CONF_LEVEL_CRIT_SCALING || !prediag)
		return 1.f;

	return scaling_half(prediag -> patientConfidenceLevel, 0.f, 1.f, PCF_MIN_CRITCOEFF, PCF_MAX_CRITCOEFF);
}


// Returns a scalar coefficient made from the BMI index, to be applied onto the criticity.
float scale_byBMI_index(const MedicalRecord *medrec)
{
	if (!ENABLE_BMI_INDEX_CRIT_SCALING || !medrec)
		return 1.f;

	float BMI_index = getBMI_index(medrec -> weight, medrec -> height);

	return scaling_by_thresholds(BMI_index, BMI_THRESH_A, BMI_THRESH_B, BMI_THRESH_C, BMI_THRESH_D, BMI_MIN_CRITCOEFF, BMI_MAX_CRITCOEFF);
}


// Returns a scalar coefficient made from the patient age, to be applied onto the criticity.
float scale_byAge(const MedicalRecord *medrec)
{
	if (!ENABLE_AGE_CRIT_SCALING || !medrec)
		return 1.f;

	return scaling_by_thresholds(medrec -> age, AGE_THRESH_A, AGE_THRESH_B, AGE_THRESH_C, AGE_THRESH_D, AGE_MIN_CRITCOEFF, AGE_MAX_CRITCOEFF);
}


// Given an input value 'x', returns a number between 'y_min' and 'y_max'.
// Specifically, returns 'y_min' when 'x' is lower than 'x_a', 'y_max' when 'x' is greater than 'x_b',
// and does a linear regression between the two, as to have a continuous transformation.
inline float scaling_half(float x, float x_a, float x_b, float y_min, float y_max)
{
	return scaling_by_thresholds(x, -INFINITY, -INFINITY, x_a, x_b, y_min, y_max);
}


// Given an input value 'x', returns a number between 'y_min' and 'y_max'.
// Specifically, returns 'y_max' when 'x' is lower than 'x_a' or greater than 'x_d',
// 'y_min' when 'x' is between 'x_b' and 'x_c', and does linear regressions for the other cases,
// as to have a continuous transformation.
float scaling_by_thresholds(float x, float x_a, float x_b, float x_c, float x_d, float y_min, float y_max)
{
	if (x < x_a)
		return y_max;

	if (x < x_b)
		return (float) (y_min - y_max) / (x_b - x_a + EPSILON) * (x - x_a) + y_max;

	if (x < x_c)
		return y_min;

	if (x < x_d)
		return (float) (y_max - y_min) / (x_d - x_c + EPSILON) * (x - x_d) + y_max;

	return y_max;
}
