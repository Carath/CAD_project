#ifndef UTILITIES_H
#define UTILITIES_H


// Reads an entire text file, and returns it as a string:
char* readFileContent(char *filename);


char** createCharMatrix(int rows, int cols);


// Free the matrix pointed by the given adress, and set it to NULL:
void freeCharMatrix(char ***matrix, int rows);


// Returns the index of a string in the given array if it exists, or 'bound' otherwise.
// 'bound' needs to not be greater than the array length.
int getStringIndex(const char *string, char* const *string_array, int bound);


#endif
