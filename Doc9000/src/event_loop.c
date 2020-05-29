#ifndef _DEFAULT_SOURCE // for DT_REG
#define _DEFAULT_SOURCE
#endif

// Related macros:
// _POSIX_C_SOURCE
// _XOPEN_SOURCE
// _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// May be Unix dependant. Used for system calls:
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
// #include <sys/select.h>
// #include <errno.h>
// #include <fcntl.h>
// #include <time.h>

#include "event_loop.h"
#include "diagnostic_making.h"


#define ESC_KEY 27


// Low level functions to handle user input, taken from rosettacode.org:
static void set_mode(int want_key);
static int get_key(void);


static struct termios old, new;

static char Full_path_src[MAX_FILENAME_PATH_LENGTH];
static char Full_path_dest[MAX_FILENAME_PATH_LENGTH];

static const unsigned int fetchingCooldownInMicroSeconds = FETCHING_COOLDOWN * 1000000;
static const unsigned int CleanupThreshold = (float) CLEANUP_COOLDOWN / FETCHING_COOLDOWN + 0.5f;


///////////////////////////////////////////////////////////////////////////
// Frameworks to be used:


// Does what 'diagnosticFullProcess()' do, albeit periodically.
// This process can be stopped by pressing either the 'q' key or ESC.
void diagnosticEventLoop(void)
{
	printf("\nDiagnostic event loop:\n");

	// Just to be sure the folders exist:
	createFolder(PREDIAGS_SRC_FOLDER);
	createFolder(PREDIAGS_PROCESSED_FOLDER);
	createFolder(PREDIAGS_FAILED_FOLDER);

	printf("\n-> This process can be stopped by pressing either the 'q' key or ESC.\n\n");

	int read_char; // keep this an int!
	unsigned int epoch = 0;

	while (1)
	{
		set_mode(1); // call this with arg 0 to quit properly without input!

		while (!(read_char = get_key()))
		{
			// N.B: An opendir() call is needed here anyway, in order to refresh the source directory.
			double time_elapsed = diagnosticFullProcess();

			// Do _not_ store the following value as a 'useconds_t', for it is unsigned!
			double cooldown_left_microsec = fetchingCooldownInMicroSeconds - time_elapsed * 1000000.;

			if (cooldown_left_microsec < 0.)
				cooldown_left_microsec = 0.;

			// printf("Time left to wait: %.6f s\n", cooldown_left_microsec);

			usleep(cooldown_left_microsec); // gives more control than sleep().

			++epoch;

			if (ENABLE_CLEANUP && epoch >= CleanupThreshold)
			{
				cleanupProcess();

				epoch = 0;
			}
		}

		if (read_char == 'q' || read_char == 'Q' || read_char == ESC_KEY)
			break; // Ressources will have to be freed!
	}

	printf("\nEnd of the event loop.\n");
}


// Fetches the prediagnostics from their source directory, makes a diagnostic for each of them,
// stores the result into the database, moves the prediagnostics file to other directories,
// and collects some data along the way. Returns the elapsed time. Note that this shall work
// properly no matter if new prediagnostics are added in the meantime.
double diagnosticFullProcess(void)
{
	double time_start = get_time();

	DIR *directory = opendir(PREDIAGS_SRC_FOLDER);

	if (!directory)
	{
		printf("\nUnable to find the directory: '%s'.\n", PREDIAGS_SRC_FOLDER);
		return 0.;
	}

	int diags_number = 0, fails_number = 0;

	struct dirent *dir = NULL;

	while ((dir = readdir(directory)) != NULL)
	{
		if (dir -> d_type == DT_REG) // Condition to check regular file.
		{
			snprintf(Full_path_src, MAX_FILENAME_PATH_LENGTH, "%s%s", PREDIAGS_SRC_FOLDER, dir -> d_name);

			if (VERBOSE_MODE >= 2)
				printf("Trying to process the file: '%s'.\n", Full_path_src);

			int diag_result = diagnosticProcessing(Full_path_src);

			char *dest_dir = diag_result ? PREDIAGS_PROCESSED_FOLDER : PREDIAGS_FAILED_FOLDER;

			snprintf(Full_path_dest, MAX_FILENAME_PATH_LENGTH, "%s%s", dest_dir, dir -> d_name);

			int move_result = moveFile(Full_path_dest, Full_path_src);

			if (!diag_result || !move_result)
				++fails_number;

			++diags_number;
		}
	}

	closedir(directory);

	double time_elapsed = get_time() - time_start;

	if (VERBOSE_MODE >= 1 && diags_number > 0)
	{
		printf("Number of processed prediagnostics: %2d. Failures: %2d. (%.3f s)\n", diags_number, fails_number, time_elapsed);
	}

	return time_elapsed;
}


// Cleans the directory of processed prediagnostics, and collects some data along the way.
// The directory of failed prediagnostics is never cleaned up, for debugging purpose.
void cleanupProcess(void)
{
	DIR *directory = opendir(PREDIAGS_PROCESSED_FOLDER);

	if (!directory)
	{
		printf("\nUnable to find the directory: '%s'.\n", PREDIAGS_PROCESSED_FOLDER);
		return;
	}

	int toRemove_number = 0, fails_number = 0;

	struct dirent *dir = NULL;

	while ((dir = readdir(directory)) != NULL)
	{
		if (dir -> d_type == DT_REG) // Condition to check regular file.
		{
			snprintf(Full_path_src, MAX_FILENAME_PATH_LENGTH, "%s%s", PREDIAGS_PROCESSED_FOLDER, dir -> d_name);

			fails_number += !deleteFile(Full_path_src);

			++toRemove_number;
		}
	}

	closedir(directory);

	if (VERBOSE_MODE >= 1 && toRemove_number > 0)
	{
		printf("Number of removed files: %2d. Failures: %2d.\n", toRemove_number, fails_number);
	}
}


///////////////////////////////////////////////////////////////////////////
// Low level functions to handle user input, taken from rosettacode.org:


// From rosettacode.org:
static void set_mode(int want_key)
{
	if (!want_key)
	{
		tcsetattr(STDIN_FILENO, TCSANOW, &old);
		return;
	}

	tcgetattr(STDIN_FILENO, &old);
	new = old;
	new.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new);
}


// From rosettacode.org:
static int get_key(void)
{
	int c = 0;
	struct timeval tv;
	fd_set fs;
	tv.tv_usec = tv.tv_sec = 0;

	FD_ZERO(&fs);
	FD_SET(STDIN_FILENO, &fs);
	select(STDIN_FILENO + 1, &fs, 0, 0, &tv);

	if (FD_ISSET(STDIN_FILENO, &fs))
	{
		c = getchar();
		set_mode(0);
	}
	return c;
}


///////////////////////////////////////////////////////////////////////////
// Proof of concept:


// Loop which stops when the user presses 'q' or ESC.
void demoLooping(void)
{
	printf("------------------------------------------------------------------\n");
	printf("-> Demo of an event loop. This program can be stopped by pressing the 'q' key or ESC.\n");
	int read_char; // keep this an int!

	while (1)
	{
		set_mode(1); // call this with arg 0 to quit properly without input!

		while (!(read_char = get_key()))
		{
			printf("...\n");

			usleep(fetchingCooldownInMicroSeconds); // gives more control than sleep().
		}

		printf("Key: '%c' (%d)\n", read_char, read_char);

		if (read_char == 'q' || read_char == 'Q' || read_char == ESC_KEY)
			return;
	}
}
