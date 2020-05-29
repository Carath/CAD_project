#ifndef PREDIAGNOSTIC_FILE_H
#define PREDIAGNOSTIC_FILE_H


#include "medical_structs.h"


// PreDiagnostic file:
// Filename: 'DIAG_FOLDER'/Patient_'id_socdet'_'timestamp'_BehaviorAnalysis.bin,
// where 'timestamp' is the number of seconds passed since January 1, 1970.
// File size: 4 * symptomNumber + 18 bytes. 'symptomNumber' is written inside the file, see below.
// File content, in the following order:
// - magic_number: short (2 bytes). See below for its fixed value. This is useful to test endianness and proper reading.
// - timestamp: unsigned long (8 bytes)
// - id_socdet: int (4 bytes)
// - (converted) Patient confidence level: short (2 bytes). Is always between 0 and 100.
// - symptomNumber: short (2 bytes). It's the number of couples (symptom, (converted) symptom confidence).
// - symptomNumber times:
// 	- symptom: short (2 bytes)
// 	- (converted) symptom confidence: short (2 bytes). Is always between 0 and 100.
// N.B: The symptom 'no_symptom' will never be written in a prediagnostic file.


#define MAGIC_NUMBER 100 // Arbitrary integer value on 2 bytes, but needs to not be divided by 257.

#define CONVERSION_COEFF 100.f // For short <--> float conversion.
// Will not truncate values in practice, since only rounded values will be saved in the first place.


// Useful only for testing purpose. Do _not_ free the result, as it is static.
const char* lastGeneratedPreDiagnosticFilename(void);


// Will generate a proper filename automatically. Last generated filename
// is stored in 'Generated_filename'. Returns 1 on success, 0 else.
int writePreDiagnosticFile(const PreDiagnostic *prediag);


// Reads a prediagnostic file, and returns a new prediagnostic on success,
// and NULL on failure. This will require a freePreDiagnostic() call afterhand.
PreDiagnostic* readPreDiagnosticFile(const char *filename);


// Checks the filenames of the prediagnostics files, which must be located in 'PREDIAGS_SRC_FOLDER'.
int prediagFilenameCheck(const char *filename);


#endif
