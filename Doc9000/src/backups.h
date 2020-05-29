#ifndef BACKUPS_H
#define BACKUPS_H


#include "parsing.h"


// Saves all ressources to the related files.
ParsingStatus saveParsingResults(void);


// Load 'IllnessNumber', 'SymptomNumber', and the criticites from the related file.
ParsingStatus loadCriticities(void);


// Loads 'IllnessStringArray' and 'SymptomStringArray' in memory from the related files:
ParsingStatus loadStringNames(void);


#endif
