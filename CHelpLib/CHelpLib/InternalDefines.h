
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

typedef union CHL_key {
    PSTR pszKey;
    PWSTR pwszKey;
    int iKey;
    UINT uiKey;
    PVOID pvKey;
}CHL_key;

typedef union CHL_val {
    int iVal;
    UINT uiVal;
    double dVal;
    PSTR pszVal;    // Pointer to ANSI string(value is allocated on heap)
    PWSTR pwszVal;  // Pointer to wide string(value is allocated on heap)
    PVOID pvPtr;    // Stores a pointer(any type)(value is allocated on heap)
    PVOID pvUserObj;   // Pointer to user object(value is allocated on heap)
}CHL_val;

#endif // CHL_INT_DEFINES_H
