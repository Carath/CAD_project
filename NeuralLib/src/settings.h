#ifndef SETTINGS_H
#define SETTINGS_H


// Maximum number of characters for the directories path:
#define MAX_PATH_LENGTH 300

#define EPSILON 0.000001


typedef enum {INFOS, ALL} PrintOption;


#if defined _FLOAT
	typedef float Number;
	#define number_pow powf
	#define number_sqrt sqrtf
	#define number_abs fabsf
	#define number_min fminf
	#define number_max fmaxf
	#define number_abs fabsf
	#define number_exp expf
	#define number_tanh tanhf

#elif defined _DOUBLE
	typedef double Number;
	#define number_pow pow
	#define number_sqrt sqrt
	#define number_abs fabs
	#define number_min fmin
	#define number_max fmax
	#define number_abs fabs
	#define number_exp exp
	#define number_tanh tanh
#endif


// Used to get the length of an array on the stack, in the same context it is defined.
// Do _not_ use this on an array allocated in the heap, or on an array passed as parameter of a function.
#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(array) \
	(sizeof(array) / sizeof(*(array)))
#endif

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) ((x) < (y) ? (y) : (x))
#endif

// Returns the arg name as a string:
#ifndef TO_STRING
#define TO_STRING(STRING) #STRING
#endif


// Used to check the equality of lengths between two arrays. Returns said length.
// Warning: use this only in the same context of the definition of said arrays.
#ifndef ARRAYS_COMPARE_LENGTH
#define ARRAYS_COMPARE_LENGTH(array_1, array_2)														\
({																									\
	if (ARRAY_LENGTH(array_1) != ARRAY_LENGTH(array_2))												\
	{																								\
		printf("\nIncompatible lengths between '%s' and '%s': %ld vs %ld.\n",						\
			TO_STRING(array_1), TO_STRING(array_2), ARRAY_LENGTH(array_1), ARRAY_LENGTH(array_2));	\
		exit(EXIT_FAILURE);																			\
	}																								\
	(int) ARRAY_LENGTH(array_1);																	\
})
#endif


#endif
