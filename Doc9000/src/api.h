#ifndef API_H
#define API_H


#include "medical_structs.h"


// Does nothing if already connected! Returns 1 on success, 0 else.
int connectToDatabase(void);


// Does nothing if not connected!
void disconnectFromDatabase(void);


// Reads a diagnostic from the database. The diagnostic struct must be already
// allocated (optimization). Returns 1 on success, 0 else.
int readDiagnostic(Diagnostic *diag, int id_diag);


// Get 'id_medrec' from the patient 'id_socdet'.
int getMedicalRecordId(int id_socdet);


// Reads a medical record from the database, including (at most) the last 'MAX_DIAGS_READ' diagnostics.
// Returns NULL on failure. This will require a freeMedicalRecord() call afterhand.
MedicalRecord* readMedicalRecord(int id_socdet);


// Writes a diagnostic on the database. Returns the new 'id_diag' on success, 0 else.
int writeDiagnostic(const Diagnostic *diagnostic, int id_socdet);


// Compares backups strings to those on the database, for all the relevant tables.
// Returns 1 on success, 0 else.
int checkBackupsIntegrity(void);


#endif
