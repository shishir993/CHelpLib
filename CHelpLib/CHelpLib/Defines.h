
// Defines.h
// Contains common #defines, typedefs and data structures
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      09/09/14 Refactor to store defs in individual headers.
//

#ifndef CHL_DEFINES_H
#define CHL_DEFINES_H

#include <Windows.h>

// -------------------------------------------
// #defs and typedefs

#ifdef CHELPLIB_EXPORTS
#define DllExpImp __declspec( dllexport )
#else
#define DllExpImp __declspec( dllimport )
#endif // CHELPLIB_EXPORTS

// Custom error codes
#define CHLE_MUTEX_TIMEOUT  17010
#define CHLE_EMPTY_FILE     17011

// Key Types
#define CHL_KT_STRING   10
#define CHL_KT_INT32    11

// Value Types
#define CHL_VT_STRING   20
#define CHL_VT_INT32    21
#define CHL_VT_PVOID    22

// Key type and Value type typedefs
typedef int CHL_KEYTYPE;
typedef int CHL_VALTYPE;

#define HT_MUTEX_NAME   (TEXT("CHL_MUTEX_NAME"))

// -------------------------------------------
// Structures

union _key {
    BYTE *skey;
    DWORD dwkey;
};

union _val {
    DWORD dwVal;
    void *pval;
    BYTE *pbVal;
};

#endif // CHL_DEFINES_H
