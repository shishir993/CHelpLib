
// Defines.h
// Contains common #defines, typedefs and data structures
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      09/18/2014 Standardize keys and value types
//

#ifndef CHL_INT_DEFINES_H
#define CHL_INT_DEFINES_H

#include "CommonInclude.h"

// -------------------------------------------
// Structures

union _tagCHL_KEYDEF {
    PSTR pszKey;
    PWSTR pwszKey;
    int iKey;
    UINT uiKey;
    PVOID pvKey;
};

union _tagCHL_VALDEF {
    int iVal;
    UINT uiVal;
    double dVal;
    PSTR pszVal;    // Pointer to ANSI string(value is allocated on heap)
    PWSTR pwszVal;  // Pointer to wide string(value is allocated on heap)
    PVOID pvPtr;    // Stores a pointer(any type)(value is allocated on heap)
    PVOID pvUserObj;   // Pointer to user object(value is allocated on heap)
};

typedef struct CHL_KEY
{
    // Num. of bytes as size of key
    int iKeySize;

    // Storage for the key
    union _tagCHL_KEYDEF keyDef;

}CHL_KEY, *PCHL_KEY;

typedef struct CHL_VAL
{
    // Num. of bytes as size of val
    int iValSize;

    // Storage for the value
    union _tagCHL_VALDEF valDef;

}CHL_VAL, *PCHL_VAL;

#endif // CHL_INT_DEFINES_H
