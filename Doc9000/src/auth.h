#ifndef AUTH_H
#define AUTH_H


// For testing purpose:
void createPassword(const char *filename, int length);


// This may need to be protected against running without admin rights,
// and will require a cleanAndFreePassword() call afterhand.
char* readPassword(const char *filename);


// Password passed by address:
void cleanAndFreePassword(char **password);


#endif
