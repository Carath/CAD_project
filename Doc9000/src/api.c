#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mysql.h>

#include "api.h"
#include "parsing.h"
#include "processing.h"


#ifdef LOCAL_TESTING
	#pragma message "Local database"
#else
	#pragma message "Real database"
#endif


#define DEBUG_MODE 0
#define ID_AUTO_INCREMENT 0
#define ID_DOC_DEFAULT 1


typedef enum {SYMPTOM_CHECK, ILLNESS_CHECK} CheckMode;


static MYSQL *mysql;
static char query_buffer[MAX_QUERY_LENGTH];


// MySQL queries:

#define READ_DIAGNOSTIC_QUERY															\
	"SELECT doct.id_diag, date_diag, doct.criticity, ill.id_ill, illp.probability\n"	\
	"FROM doctor_diagnostic as doct\n"													\
	"INNER JOIN illness_probability as illp ON doct.id_diag = illp.id_diag\n"			\
	"INNER JOIN illness as ill ON ill.id_ill = illp.id_ill\n"							\
	"WHERE doct.id_diag = %d;"															\


#define GET_MEDREC_ID_QUERY																\
	"SELECT patient.id_medrec\n"														\
	"FROM patient_medical_record as patient\n"											\
	"INNER JOIN social_details as social ON patient.id_socdet = social.id_socdet\n"		\
	"WHERE patient.id_socdet = %d;"														\


#define READ_MEDREC_DIAGS_NUMBER_QUERY													\
	"SELECT COUNT(doctor.id_medrec) as diagnosticNumber\n"								\
	"FROM patient_medical_record as patient\n"											\
	"INNER JOIN social_details as social ON patient.id_socdet = social.id_socdet\n"		\
	"INNER JOIN doctor_diagnostic as doctor ON patient.id_medrec = doctor.id_medrec\n"	\
	"WHERE patient.id_socdet = %d;"														\


#define READ_MEDREC_INFO_QUERY															\
	"SELECT patient.id_medrec, social.gender, social.birthdate, patient.id_blgrp,\n"	\
	"patient.weight, patient.height\n"													\
	"FROM patient_medical_record as patient\n"											\
	"INNER JOIN social_details as social ON patient.id_socdet = social.id_socdet\n"		\
	"WHERE patient.id_socdet = %d;"														\


#define READ_MEDREC_LAST_DIAGS_QUERY													\
	"SELECT doctor.id_diag\n"															\
	"FROM doctor_diagnostic as doctor\n"												\
	"WHERE doctor.id_medrec = %d\n"														\
	"ORDER BY doctor.id_diag DESC\n"													\
	"LIMIT %d;"																			\


// Args: ID_AUTO_INCREMENT, criticity, id_med_rec, ID_DOC_DEFAULT
#define INSERT_DIAG_INFO_QUERY															\
	"INSERT INTO doctor_diagnostic VALUES(%d, %f, CURDATE(), %d, %d);"					\


// Args: id_diag, illness, probability
#define INSERT_DIAG_ILLNESS_AND_PROBA_QUERY												\
	"INSERT INTO illness_probability VALUES(%d, %d, %f);"								\


// Args: id_diag, illness, probability, 5 times
#define INSERT_DIAG_ILLNESS_AND_PROBA_QUERY_PACKED_5									\
	"INSERT INTO illness_probability VALUES\n"											\
	"(%d, %d, %f), (%d, %d, %f), (%d, %d, %f), (%d, %d, %f), (%d, %d, %f);"				\


#define FETCH_ILLNESSES_NUMBER_QUERY													\
	"SELECT COUNT(*) FROM illness;"														\


#define FETCH_SYMPTOMS_NUMBER_QUERY														\
	"SELECT COUNT(*) FROM symptom;"														\


#define FETCH_ILLNESSES_QUERY															\
	"SELECT * FROM illness;"															\


#define FETCH_SYMPTOMS_QUERY															\
	"SELECT * FROM symptom;"															\


// Send the given query to the database. Returns 1 on success, 0 else:
#define sendQuery(...)																	\
({																						\
	int sendQuery_result = 1;															\
	int nb_writtenChar = snprintf(query_buffer, MAX_QUERY_LENGTH - 1, __VA_ARGS__);		\
																						\
	if (DEBUG_MODE)																		\
		printf("\nCurrent MySQL query:\n\n%s\n\n", query_buffer);						\
																						\
	if (nb_writtenChar >= MAX_QUERY_LENGTH - 1)											\
	{																					\
		printf("\n'query_buffer' too small to hold the current query.\n");				\
		sendQuery_result = 0;															\
	}																					\
																						\
	else if (mysql_query(mysql, query_buffer) != 0)										\
	{																					\
		printf("\nInvalid MySQL query:\n\n%s\n\n", query_buffer);						\
		sendQuery_result = 0;															\
	}																					\
																						\
	sendQuery_result;																	\
})


// Does nothing if already connected! Returns 1 on success, 0 else.
int connectToDatabase(void)
{
	if (mysql != NULL) // already connected!
		return 1;

	mysql = (MYSQL*) calloc(1, sizeof(MYSQL));

	if (mysql == NULL)
	{
		printf("\nNot enough memory to create a new MYSQL object.\n");
		exit(EXIT_FAILURE);
	}

	mysql_init(mysql);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "option");

	// Password acquisition:

	char *password = NULL;

	#ifdef LOCAL_TESTING
		password = PASSWORD;
	#else
		password = readPassword(AUTH_KEY_FILE);
	#endif

	// Trying to connect to the database:

	MYSQL *connection_result = mysql_real_connect(mysql, HOSTNAME, USERNAME, password, DATABASE, 0, NULL, 0);

	// Cleaning the password if necessary:

	#ifndef LOCAL_TESTING
		cleanAndFreePassword(&password);
	#endif

	if (connection_result != NULL && mysql != NULL)
	{
		printf("\nSuccessful connection to the database.\n");
		return 1;
	}

	else
	{
		printf("\nConnection to the database FAILED!\n");
		disconnectFromDatabase();
		return 0;
	}
}


// Does nothing if not connected!
void disconnectFromDatabase(void)
{
	if (mysql == NULL)
		return;

	mysql_close(mysql);
	free(mysql);
	mysql = NULL;

	printf("\nDisconnected from the database.\n");
}


// Reads a diagnostic from the database. The diagnostic struct must be already
// allocated (optimization). Returns 1 on success, 0 else.
int readDiagnostic(Diagnostic *diag, int id_diag)
{
	if (diag == NULL)
	{
		printf("Cannot read a diagnostic: not allocated yet!\n");
		return 0;
	}

	if (!connectToDatabase())
	{
		printf("Cannot read a diagnostic: not connected to the database.\n");
		return 0;
	}

	MYSQL_RES *result = NULL;
	MYSQL_ROW row = NULL;

	if (!sendQuery(READ_DIAGNOSTIC_QUERY, id_diag))
		return 0;

	result = mysql_use_result(mysql);

	if (result == NULL)
	{
		printf("Cannot output a diagnostic: invalid MySQL query.\n\n");
		return 0;
	}

	int illness_index = 0;

	diag -> id_diag = id_diag;

	while (illness_index < DIAG_ILLNESS_NUMBER && (row = mysql_fetch_row(result))) // assignment and testing!
	{
		snprintf(diag -> date_diag, DATE_MAX_LENGTH - 1, "%s", row[1]);
		diag -> criticity = strtof(row[2], NULL);
		diag -> illnessArray[illness_index] = atoi(row[3]) - 1; // Illnesses starts from 1 in the database!
		diag -> illnessProbabilityArray[illness_index] = strtof(row[4], NULL);
		++illness_index;
	}

	mysql_free_result(result);

	return 1;
}


// Get 'id_medrec' from the patient 'id_socdet'.
int getMedicalRecordId(int id_socdet)
{
	if (!connectToDatabase())
	{
		printf("\nCannot get the 'id_medrec': not connected to the database.\n");
		return 0;
	}

	MYSQL_RES *result = NULL;
	MYSQL_ROW row = NULL;

	if (!sendQuery(GET_MEDREC_ID_QUERY, id_socdet))
		return 0;

	result = mysql_use_result(mysql);

	if (result == NULL)
	{
		printf("\nCannot get the 'id_medrec': invalid MySQL query.\n");
		return 0;
	}

	if ((row = mysql_fetch_row(result)) == NULL)
	{
		printf("\nCannot get the 'id_medrec': %d\n", id_socdet);
		mysql_free_result(result);
		return 0;
	}

	short id_medrec = atoi(row[0]);

	mysql_free_result(result);

	return id_medrec;
}


// Reads a medical record from the database, including (at most) the last 'MAX_DIAGS_READ' diagnostics.
// Returns NULL on failure. This will require a freeMedicalRecord() call afterhand.
MedicalRecord* readMedicalRecord(int id_socdet)
{
	if (!connectToDatabase())
	{
		printf("\nCannot read a medical record: not connected to the database.\n");
		return NULL;
	}

	MYSQL_RES *result = NULL;
	MYSQL_ROW row = NULL;

	/////////////////////////////////////////////////////
	// Getting the number of diagnostics from the medical record:

	short diagnosticNumber = 0; // default.

	if (MAX_DIAGS_READ > 0)
	{
		if (!sendQuery(READ_MEDREC_DIAGS_NUMBER_QUERY, id_socdet))
			return NULL;

		result = mysql_use_result(mysql);

		if (result == NULL)
		{
			printf("\nCannot read a medical record: invalid MySQL query.\n\n");
			return NULL;
		}

		if ((row = mysql_fetch_row(result)) == NULL)
		{
			printf("\nCannot get the number of diagnostics for the patient: %d\n", id_socdet);
			mysql_free_result(result);
			return NULL;
		}

		diagnosticNumber = atoi(row[0]);

		if (diagnosticNumber > MAX_DIAGS_READ)
			diagnosticNumber = MAX_DIAGS_READ;

		mysql_free_result(result); // Needs to be be freed before a new query is done !!!
	}

	/////////////////////////////////////////////////////
	// Getting general data from the medical record:

	MedicalRecord *medrec = allocateMedicalRecord(diagnosticNumber);

	if (!sendQuery(READ_MEDREC_INFO_QUERY, id_socdet))
	{
		freeMedicalRecord(&medrec);
		return NULL;
	}

	result = mysql_use_result(mysql);

	if (result == NULL)
	{
		printf("\nCannot read a medical record: invalid MySQL query.\n\n");
		freeMedicalRecord(&medrec);
		return NULL;
	}

	if ((row = mysql_fetch_row(result)) == NULL)
	{
		printf("\nCannot get the medical record info for the patient: %d\n", id_socdet);
		freeMedicalRecord(&medrec);
		mysql_free_result(result);
		return NULL;
	}

	medrec -> id_medrec = atoi(row[0]);
	medrec -> gender = atoi(row[1]);
	medrec -> age = getAge(row[2]);
	medrec -> id_blgrp = atoi(row[3]);
	medrec -> weight = atoi(row[4]);
	medrec -> height = atoi(row[5]);
	medrec -> diagnosticNumber = diagnosticNumber;

	mysql_free_result(result); // Needs to be be freed before a new query is done !!!

	/////////////////////////////////////////////////////
	// Getting the diagnostics from the medical record:

	const int id_medrec = getMedicalRecordId(id_socdet);

	int *id_diag_buffer = NULL; // declared here, so we can go to 'failure' if needed.

	if (!sendQuery(READ_MEDREC_LAST_DIAGS_QUERY, id_medrec, MAX_DIAGS_READ))
		goto failure;

	result = mysql_use_result(mysql);

	if (result == NULL)
		goto failure;

	// Buffering the id_diag values, in order to be able to call readDiagnostic()...

	id_diag_buffer = (int*) calloc(diagnosticNumber, sizeof(int));

	if (id_diag_buffer == NULL)
	{
		printf("\nNot enough memory to create a diagnostic buffer.\n");
		goto failure;
	}

	for (int i = 0; i < diagnosticNumber; ++i)
	{
		row = mysql_fetch_row(result);

		if (row == NULL)
			goto failure;

		id_diag_buffer[i] = atoi(row[0]);
	}

	mysql_free_result(result); // Needs to be be freed before a new query is done !!!

	// Now, fetching each diagnostic:

	for (int i = 0; i < diagnosticNumber; ++i)
	{
		if (!readDiagnostic(medrec -> diagnosticArray + i, id_diag_buffer[i]))
			goto failure;
	}

	free(id_diag_buffer);

	return medrec;

	failure:
		printf("\nCannot read a medical record.\n\n");
		free(id_diag_buffer);
		freeMedicalRecord(&medrec);
		mysql_free_result(result);
		return NULL;
}


// Writes a diagnostic on the database. Returns the new 'id_diag' on success, 0 else.
int writeDiagnostic(const Diagnostic *diagnostic, int id_socdet)
{
	if (diagnostic == NULL)
	{
		printf("\nNULL diagnostic.\n");
		return 0;
	}

	if (!connectToDatabase())
	{
		printf("\nCannot write a diagnostic: not connected to the database.\n");
		return 0;
	}

	const int id_medrec = getMedicalRecordId(id_socdet);

	/////////////////////////////////////////////////////
	// Inserting in the database the diagnostic info:

	if (!sendQuery(INSERT_DIAG_INFO_QUERY, ID_AUTO_INCREMENT, diagnostic -> criticity, id_medrec, ID_DOC_DEFAULT))
		return 0;

	int new_id = mysql_insert_id(mysql); // ID of the newly inserted entry.

	if (new_id == 0)
	{
		printf("\nCould not insert a diagnostic into the database!\n");
		return 0;
	}

	/////////////////////////////////////////////////////
	// Inserting in the database the diagnostic illnesses and probabilities:
	// Careful: illnesses starts from 1 in the database!

	#if PACK_ILLNESS_PROBA && DIAG_ILLNESS_NUMBER != 5
		#pragma message "Could not use the packed writing optimization."
	#endif

	if (PACK_ILLNESS_PROBA && DIAG_ILLNESS_NUMBER == 5) // faster, but hardcoded with size 5.
	{
		if (!sendQuery(INSERT_DIAG_ILLNESS_AND_PROBA_QUERY_PACKED_5,
			new_id, diagnostic -> illnessArray[0] + 1, diagnostic -> illnessProbabilityArray[0],
			new_id, diagnostic -> illnessArray[1] + 1, diagnostic -> illnessProbabilityArray[1],
			new_id, diagnostic -> illnessArray[2] + 1, diagnostic -> illnessProbabilityArray[2],
			new_id, diagnostic -> illnessArray[3] + 1, diagnostic -> illnessProbabilityArray[3],
			new_id, diagnostic -> illnessArray[4] + 1, diagnostic -> illnessProbabilityArray[4]))
		{
			return 0;
		}
	}

	else // generic.
	{
		for (int i = 0; i < DIAG_ILLNESS_NUMBER; ++i)
		{
			if (!sendQuery(INSERT_DIAG_ILLNESS_AND_PROBA_QUERY, new_id, diagnostic -> illnessArray[i] + 1,
				diagnostic -> illnessProbabilityArray[i]))
			{
				return 0;
			}
		}
	}

	return new_id;
}


// Compares local strings to those on the given database table:
static int checkTableIntegrity(CheckMode check)
{
	if (!connectToDatabase())
	{
		printf("\nCannot check backups integrity: not connected to the database.\n");
		return 0;
	}

	short entriesNumber;
	char *message_string;

	const char* (*getStringName)(short);

	int fetch_table_length;

	if (check == SYMPTOM_CHECK)
	{
		fetch_table_length = sendQuery(FETCH_SYMPTOMS_NUMBER_QUERY);
		entriesNumber = getSymptomNumber();
		getStringName = getSymptomName;
		message_string = "symptoms";
	}
	else // ILLNESS_CHECK
	{
		fetch_table_length = sendQuery(FETCH_ILLNESSES_NUMBER_QUERY);
		entriesNumber = getIllnessNumber();
		getStringName = getIllnessName;
		message_string = "illnesses";
	}

	if (!fetch_table_length)
		return 0;

	MYSQL_RES *result = NULL;
	MYSQL_ROW row = NULL;

	/////////////////////////////////////////////////////
	// Comparing the local entries number to the database:

	result = mysql_use_result(mysql);

	if (result == NULL)
	{
		printf("\nCannot get the table's entries number: invalid MySQL query.\n\n");
		return 0;
	}

	if ((row = mysql_fetch_row(result)) == NULL)
	{
		printf("\nCannot check the table integrity for %s.\n", message_string);
		mysql_free_result(result);
		return 0;
	}

	int database_entries_number = atoi(row[0]);

	if (entriesNumber > database_entries_number)
	{
		printf("\nToo few %s in the database! Doc9000 may not work properly. (%d vs %d)\n",
			message_string, entriesNumber, database_entries_number);
		mysql_free_result(result);
		return 0;
	}

	else if (entriesNumber < database_entries_number)
	{
		printf("\nThere is more %s in the database than in local files,\n"
			"however this should not impact Doc9000 internal working. (%d vs %d)\n",
			message_string, entriesNumber, database_entries_number);
	}

	else
	{
		printf("\nNumber of %s match between the database and backups (%d).\n", message_string, entriesNumber);
	}

	mysql_free_result(result);

	/////////////////////////////////////////////////////
	// Comparing the local strings to the database:

	int fetch_table_result = check == SYMPTOM_CHECK ? sendQuery(FETCH_SYMPTOMS_QUERY) : sendQuery(FETCH_ILLNESSES_QUERY);

	if (!fetch_table_result)
		return 0;

	result = mysql_use_result(mysql);

	if (result == NULL)
	{
		printf("\nCannot check integrity: invalid MySQL query.\n\n");
		return 0;
	}

	short index = 0;

	while (index < entriesNumber && (row = mysql_fetch_row(result)))
	{
		if (strcmp(getStringName(index), row[1]) != 0)
			break;

		++index;
	}

	if (index != entriesNumber)
	{
		printf("\nIntegrity check for %s: FAILED at database index %d\n -> %s vs %s\n\n",
			message_string, index + 1, getStringName(index), row[1]);
		mysql_free_result(result);
		return 0;
	}

	mysql_free_result(result);
	printf("\nIntegrity check for %s: successful.\n\n", message_string);
	return 1;
}


// Compares backups strings to those on the database, for all the relevant tables.
// Returns 1 on success, 0 else.
int checkBackupsIntegrity(void)
{
	printf("\n-> Checking backups integrity:\n");

	int illness_check = checkTableIntegrity(ILLNESS_CHECK);
	int symptom_check = checkTableIntegrity(SYMPTOM_CHECK);

	return illness_check && symptom_check;
}
