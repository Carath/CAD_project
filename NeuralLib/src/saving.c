#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// May be Unix dependant.
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "saving.h"


// Used to exit the program when a file error occur, raising the given message and filename:
void exitFileError(FILE *file, const char *error_message, const char *filename)
{
	if (error_message != NULL && filename != NULL)
		printf("\n%s '%s'.\n\n", error_message, filename);

	if (file != NULL) // file must have been opened to be closed!
		fclose(file);

	exit(EXIT_FAILURE);
}


// Creating a folder:
void createFolder(const char *foldername)
{
	if (foldername == NULL)
	{
		printf("\nNULL foldername.\n\n");
		return;
	}

	struct stat st = {0};

	// Check if directory already exists:
	if (stat(foldername, &st) == -1)
	{
		if (mkdir(foldername, 0700) == 0)
			printf("\nFolder '%s' has been created.\n\n", foldername);
		else
			printf("\nUnable to create the folder '%s'.\n\n", foldername);
	}
	else
	{
		// printf("\nFolder '%s' already exists.\n\n", foldername);
	}
}


// Moves a file (and eventually renames it) to another location.
// Returns 1 on success, 0 else (may happen if 'dest_path' contains non-existent folders).
int moveFile(const char *dest_path, const char *src_path)
{
	if (rename(src_path, dest_path) == 0)
	{
		// printf("File '%s' moved successfully to '%s'.\n", src_path, dest_path);
		return 1;
	}
	else
	{
		printf("\nUnable to move the file: '%s' to '%s'.\n", src_path, dest_path);
		return 0;
	}
}


// Deletes the given file. Returns 1 on success, 0 else.
int deleteFile(const char *filename)
{
	if (remove(filename) == 0)
	{
		// printf("Deleted successfully\n");
		return 1;
	}
	else
	{
		printf("\nUnable to delete the file: '%s'\n", filename);
		return 0;
	}
}


// Get the size of the given binary file, in bytes:
long int getFileSize(const char *filename)
{
	if (filename == NULL)
	{
		printf("\nNULL filename.\n\n");
		return -1;
	}

	FILE *file = fopen(filename, "rb");

	if (file == NULL)
	{
		printf("\nCannot find the file '%s'.\n", filename);
		return -1;
	}

	fseek(file, 0, SEEK_END);
	long int size = ftell(file);
	fclose(file);

	return size;
}


// Skip the given string in the opened file:
inline void skip(FILE *file, const char *string)
{
	if (file == NULL || string == NULL)
		return;

	if (fseek(file, strlen(string), SEEK_CUR) != 0)
		exitFileError(file, "Cannot skip the given string.", "");
}


// Writing:


static inline void save_vector(const Number *vector, int len, FILE *file)
{
	fwrite(vector, sizeof(Number), len, file); // faster by block.
}


void save_flat_matrix(const Number *matrix, int rows, int cols, const char *filename)
{
	if (matrix == NULL)
	{
		printf("\nCannot save a NULL matrix.\n\n");
		return;
	}

	if (filename == NULL)
	{
		printf("\nNULL filename.\n\n");
		return;
	}

	FILE *file = fopen(filename, "wb");

	if (file == NULL)
		exitFileError(file, "Cannot create the file", filename);

	save_vector(matrix, rows * cols, file);

	fclose(file);

	// printf("\nWriting in the file '%s' done.\n\n", filename);
}


void save_matrix(Number* const* matrix, int rows, int cols, const char *filename)
{
	if (matrix == NULL)
	{
		printf("\nCannot save a NULL matrix.\n\n");
		return;
	}

	if (filename == NULL)
	{
		printf("\nNULL filename.\n\n");
		return;
	}

	FILE *file = fopen(filename, "wb");

	if (file == NULL)
		exitFileError(file, "Cannot create the file", filename);

	for (int i = 0; i < rows; ++i)
		save_vector(matrix[i], cols, file);

	fclose(file);

	// printf("\nWriting in the file '%s' done.\n\n", filename);
}


// Reading:


static inline void load_toVector(Number *vector, int len, FILE *file)
{
	if (fread(vector, sizeof(Number), len, file) != len) // faster by block.
		exitFileError(file, "Not enough Numbers in the file.", "");
}


// Matrix must alrealy be allocated in memory:
void load_toFlatMatrix(Number *matrix, int rows, int cols, const char *filename)
{
	if (matrix == NULL)
	{
		printf("\nCannot load to a NULL matrix.\n\n");
		exit(EXIT_FAILURE);
	}

	if (filename == NULL)
	{
		printf("\nNULL filename.\n\n");
		exit(EXIT_FAILURE);
	}

	FILE *file = fopen(filename, "rb");

	if (file == NULL)
		exitFileError(file, "Cannot find the file", filename);

	load_toVector(matrix, rows * cols, file);

	fclose(file);

	// printf("\nReading the file '%s' done.\n\n", filename);
}


// Matrix must alrealy be allocated in memory:
void load_toMatrix(Number **matrix, int rows, int cols, const char *filename)
{
	if (matrix == NULL)
	{
		printf("\nCannot load to a NULL matrix.\n\n");
		exit(EXIT_FAILURE);
	}

	if (filename == NULL)
	{
		printf("\nNULL filename.\n\n");
		exit(EXIT_FAILURE);
	}

	FILE *file = fopen(filename, "rb");

	if (file == NULL)
		exitFileError(file, "Cannot find the file", filename);

	for (int i = 0; i < rows; ++i)
		load_toVector(matrix[i], cols, file);

	fclose(file);

	// printf("\nReading the file '%s' done.\n\n", filename);
}
