#ifndef DEMOS_H
#define DEMOS_H


// Checking the creation, and reading of a password file:
int testPassword(void);


// Checking the initial parsing success:
int demoParsingSuccess(void);


// Printing several medical structs:
int testStructures(void);


// Trying to write a handmade prediagnotic to a file, and then read it:
int test_WriteAndRead_PreDiagnosticFile(void);


// Trying to make a diagnostic from a handmade prediagnostic:
int testDiagnosticProduction(void);


// Trying to read a diagnostic from the database:
int testReadDiagnostic(int id_diag);


// Trying to read a medical record from the database:
int testReadMedicalRecord(int id_socdet);


// Tries to write a diagnostic to the local database,
// does nothing on the real one, to not clutter it.
int testWriteDiagnostic(int id_socdet);


#endif
