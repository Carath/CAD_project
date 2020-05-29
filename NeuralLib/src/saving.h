#ifndef SAVING_H
#define SAVING_H


#include "settings.h"


// Used to exit the program when a file error occur, raising the given message and filename:
void exitFileError(FILE *file, const char *error_message, const char *filename);


// Creating a folder:
void createFolder(const char *foldername);


// Moves a file (and eventually renames it) to another location.
// Returns 1 on success, 0 else (may happen if 'dest_path' contains non-existent folders).
int moveFile(const char *dest_path, const char *src_path);


// Deletes the given file. Returns 1 on success, 0 else.
int deleteFile(const char *filename);


// Get the size of the given binary file, in bytes:
long int getFileSize(const char *filename);


// Skip the given string in the opened file:
void skip(FILE *file, const char *string);


// Writing:

void save_flat_matrix(const Number *matrix, int rows, int cols, const char *filename);


void save_matrix(Number* const* matrix, int rows, int cols, const char *filename);


// Reading:

// Matrix must alrealy be allocated in memory:
void load_toFlatMatrix(Number *matrix, int rows, int cols, const char *filename);


// Matrix must alrealy be allocated in memory:
void load_toMatrix(Number **matrix, int rows, int cols, const char *filename);


#endif
