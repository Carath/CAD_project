#ifndef RECOGNITION_H
#define RECOGNITION_H


#include "settings.h"


typedef enum {VALIDATION, PREDICTION} RecogType;

// MAX_VALUE: multiclass problems only. Unique positive answer.
// MAX_CORRECT: binary or multiclass problems. At most one positive answer.
// ALL_CORRECT: binary, multiclass or non-mutually exclusive multiclass problems. Multiple positive answers allowed.
// Modes sorted by strictness: MAX_VALUE < MAX_CORRECT < ALL_CORRECT.
typedef enum {MAX_VALUE, MAX_CORRECT, ALL_CORRECT} RecognitionMode;


// Validation:

// MAX_VALUE -> Checks if the class with higher value is the good answer. Do not use for binary classification.

// MAX_CORRECT -> Checks if the class with higher value is the good answer, if said value is >= RECOG_THRESHOLD.
// Useful for binary classification. Stricter than 'valid_MaxValue'.

// ALL_CORRECT -> Checks if all classes have good answers. Useful for binary classification,
// and non-mutually exclusive classification problems. Stricter than 'valid_MaxCorrect'.

// Checks if the predicted answer is correct, whith respect to the chosen recognition method:
int validation_method(Number *good_answer, const Number *answer, int len, RecognitionMode recog);


// Framework for using the validation or prediction functions:
int recog_method(Number *goodOrToFill_answers, const Number *answer, int len, RecognitionMode recog, RecogType type);


// Find the most probable class answer:
// For binary class problems, returns the binary answer.
// For mutally exclusive class problems, returns the index of the guessed class.
// For mixed problems, one can shift 'answer' and modify 'len' accordingly on each subset which are binary/exclusive.
// Optional: 'confidence_level' is filled if a non-NULL arg is passed.
int findMostProbable(const Number *answer, int len, Number *confidence_level);


// Finds the 'bufferLength' greater values in 'valuesArray', and store their index in 'buffer', in descending order:
void findGreaterValuesIndex(int *buffer, int bufferLength, const Number *valuesArray, int valuesArrayLength);


#endif
