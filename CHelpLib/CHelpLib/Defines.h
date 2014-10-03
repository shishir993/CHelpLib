
// Defines.h
// Contains common #defines, typedefs and data structures
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      09/09/14 Refactor to store defs in individual headers.
//

#ifndef CHL_DEFINES_H
#define CHL_DEFINES_H

#include <Windows.h>
#include "InternalDefines.h"

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

#ifndef PCVOID
typedef PVOID const PCVOID;
#endif

// Key Types
typedef enum
{
    // Invalid value type
    CHL_KT_START,

    // Signed 32bit integer
    CHL_KT_INT32,

    // Unsigned 32bit integer
    CHL_KT_UINT32,

    // A pointer whose pointed-to address is the key
    CHL_KT_POINTER,

    // Null-terminated ANSI string
    CHL_KT_STRING,

    // Null-terminated Unicode(wide char) string
    CHL_KT_WSTRING,

    // Invalid value type
    CHL_KT_END
}CHL_KEYTYPE;

// Value Types
typedef enum
{
    // Invalid value type
    CHL_VT_START,

    // Signed 32bit integer
    CHL_VT_INT32,

    // Unsigned 32bit integer
    CHL_VT_UINT32,

    // Pointer of any type(pointed-to address is the value stored)
    CHL_VT_POINTER,

    // Any user object(data structure).
    CHL_VT_USEROBJECT,

    // Null-terminated ANSI string
    CHL_VT_STRING,

    // Null-terminated Unicode(wide char) string
    CHL_VT_WSTRING,

    // Invalid value type
    CHL_VT_END
}CHL_VALTYPE;

// -------------------------------------------
// Functions internal only
HRESULT _CopyKeyIn(_In_ CHL_key *pChlKey, _In_ CHL_KEYTYPE keyType, _In_ PCVOID pvKey, _In_opt_ int iValSize);
HRESULT _CopyKeyOut(_In_ CHL_key *pChlKey, _In_ CHL_KEYTYPE keyType, _Inout_ PVOID pvKeyOut, _In_ BOOL fGetPointerOnly);
BOOL _IsDuplicateKey(_In_ CHL_key *pChlLeftKey, _In_ PCVOID pvRightKey, _In_ CHL_KEYTYPE keyType, _In_ int iKeySize);
void _DeleteKey(_In_ CHL_key *pChlKey, _In_ CHL_KEYTYPE keyType);

HRESULT _CopyValIn(_In_ CHL_val *pChlVal, _In_ CHL_VALTYPE valType, _In_ PCVOID pvVal, _In_opt_ int iValSize);
HRESULT _CopyValOut(_In_ CHL_val *pChlVal, _In_ CHL_VALTYPE valType, _Inout_ PVOID pvValOut, _In_ BOOL fGetPointerOnly);
BOOL _IsDuplicateVal(_In_ CHL_val *pLeftVal, _In_ PCVOID pRightVal, _In_ CHL_VALTYPE valType, _In_ int iValSize);
void _DeleteVal(_In_ CHL_val *pChlVal, _In_ CHL_VALTYPE valType);

#endif // CHL_DEFINES_H
