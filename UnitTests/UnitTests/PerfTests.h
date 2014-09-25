
#ifndef _PERFTESTS_H
#define _PERFTESTS_H

#include "Common.h"
#include "Hashtable.h"

void doPerfTests();
BOOL testHashtable(CHL_KEYTYPE keytype, CHL_VALTYPE valType, int nEntries, BOOL fRandomize);
BOOL testHastable_NumNum(int nEntries, BOOL fRandomize);

#endif // _PERFTESTS_H
