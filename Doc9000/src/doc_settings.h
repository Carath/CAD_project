#ifndef DOC_SETTINGS_H
#define DOC_SETTINGS_H


#include "../../NeuralLib.h"


// Turn this ON for local testing only.
#define LOCAL_TESTING


///////////////////////////////////////////////////////////////
// Files and folders:

#define MAX_FILENAME_PATH_LENGTH 300

#define SRC_BASE_DATASET      "../data/base/base_dataset.data"

#define ILLNESS_LIST_FILENAME "../data/generated/backups/illness_list.data"
#define SYMPTOM_LIST_FILENAME "../data/generated/backups/symptom_list.data"
#define CRITICITIES_FILENAME  "../data/generated/backups/criticities.bin"

#define NEURAL_NET_DIR_PATH "../data/generated/Doc_brain/"

#define AUTH_KEY_FILE  "../data/auth/key.bin"
#define AUTH_EXPL_FILE "../data/auth/auth_example.bin"

// Prediagnostics folders:

#ifdef LOCAL_TESTING
	#define PREDIAGS_SRC_FOLDER "../prediags/prediags_src_test/"
#else
	#define PREDIAGS_SRC_FOLDER "../../../ALEDAPP/EmittedData/"
#endif

#define PREDIAGS_PROCESSED_FOLDER "../prediags/prediags_processed/"
#define PREDIAGS_FAILED_FOLDER    "../prediags/prediags_failed/"

#define PREDIAGS_FILENAME_FORMAT "Patient_%d_%ld_BehaviorAnalysis.bin"


///////////////////////////////////////////////////////////////
// API:

// Database login:
#ifdef LOCAL_TESTING
	#define HOSTNAME "localhost"
	#define USERNAME "root"
	#define PASSWORD "root"
	#define DATABASE "CAD"
#else
	#define HOSTNAME "localhost"
	#define USERNAME "non-root"
	#define DATABASE "projet"

	#include "auth.h"
#endif

#define MAX_QUERY_LENGTH 500

// Helpful for using less RAM, and speed up data fetching from the database. Can be set to 0:
#define MAX_DIAGS_READ 3

// Used for trying to pack the writing of illnesses and their probabilities.
// The packing is hardcoded internally, and will be enabled only when 'DIAG_ILLNESS_NUMBER' = 5.
#define PACK_ILLNESS_PROBA 1


///////////////////////////////////////////////////////////////
// Event loop:

#define ENABLE_CLEANUP 1 // For enabling the automatic cleanup of successfully processed prediagnostics.
#define VERBOSE_MODE 1 // For messages in the console, during the event loop.
	// 0 -> nothing, 1 -> successes/failures count, 2 -> messages from 1, read filenames, plus prediagnostics and diagnostics.

#define FETCHING_COOLDOWN 1.0 // In seconds.
#define CLEANUP_COOLDOWN (3600. * 24. * 7.) // 1 week worth of seconds
// #define CLEANUP_COOLDOWN 7 // For testing: 7 seconds.


///////////////////////////////////////////////////////////////
// Files reading:

#define CHECK_PREDIAG_FILENAMES 1 // For checking prediagnostics filenames.


///////////////////////////////////////////////////////////////
// Parsing:

#define MAX_NAME_LENGTH 50
#define MAX_ILLNESS_NUMBER 500
#define MAX_SYMPTOM_NUMBER 1000 // First symptom is hardcoded to no_symptom
#define MAX_SYMPTOM_PER_DATA 50
#define MAX_PARSING_PASSES 10

#define no_symptom 0 // Do _not_ modify this.

#define DEFAULT_SYMPTOM_ILLNESS_NAME ""
// #define DEFAULT_SYMPTOM_ILLNESS_NAME NULL


///////////////////////////////////////////////////////////////
// Training:

#define VALUE_ABSENT_SYMPTOM 0.f // Default value written in inputs.
#define SYMPTOM_THESHOLD 0.5f // Useful for generating the learning dataset.


///////////////////////////////////////////////////////////////
// Medical structs settings:

#define DATE_MAX_LENGTH 15
#define DIAG_ILLNESS_NUMBER 5


///////////////////////////////////////////////////////////////
// Processing settings:

// Do _not_ modify those 2 lines, for values are hardcoded in the database!
#define GENDER_MALE 0
#define GENDER_FEMALE 1

#define EPSILON 0.0001f

#define ENABLE_VALID_SYMPT_NUMBER_CRIT_SCALING 1 // Enables criticity scaling by the number of valid symptoms.
#define ENABLE_PATIENT_CONF_LEVEL_CRIT_SCALING 1 // Enables criticity scaling by the patient's confidence level.
#define ENABLE_BMI_INDEX_CRIT_SCALING 1 // Enables criticity scaling by the patient's BMI index.
#define ENABLE_AGE_CRIT_SCALING 1 // Enables criticity scaling by the patient's age.

#define SYMPTOM_NUMBER_THRESHOLD 3 // The criticity is reduced, when the number of valid symptoms is lower than this value.

// CONFIDENCE_NOISE_THRESHOLD: upon making a new diagnostic, illnesses which have a confidence level
// lower than this value will have no impact on the criticity computation. Setting it to 0. means
// all illnesses will have an impact, even if their confidence level is negligeable, for the sum of
// those confidence levels may not be. On the other hand, chosing a value > 0. may act as a noise reduction.
#define CONFIDENCE_NOISE_THRESHOLD 0.f // unfiltered
// #define CONFIDENCE_NOISE_THRESHOLD 0.01f // noise reduction

// Criticity modifier range for the valid symptom number:
#define VSN_MIN_CRITCOEFF 0.f
#define VSN_MAX_CRITCOEFF 1.f

// Criticity modifier range for the patient confidence level:
#define PCF_MIN_CRITCOEFF 0.75f
#define PCF_MAX_CRITCOEFF 1.f

// Criticity modifier range for the patient's BMI index:
#define BMI_MIN_CRITCOEFF 1.f
#define BMI_MAX_CRITCOEFF 1.333f

// Criticity modifier range for the patient's age:
#define AGE_MIN_CRITCOEFF 1.f
#define AGE_MAX_CRITCOEFF 1.333f

// Abscissa areas for the BMI index:
#define BMI_THRESH_A 16.f
#define BMI_THRESH_B 18.5f
#define BMI_THRESH_C 30.f
#define BMI_THRESH_D 35.f

// Abscissa areas for the patient age:
#define AGE_THRESH_A 5.f
#define AGE_THRESH_B 10.f
#define AGE_THRESH_C 60.f
#define AGE_THRESH_D 80.f


#endif
