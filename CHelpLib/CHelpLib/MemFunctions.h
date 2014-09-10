
// MemFunctions.h
// Contains functions that provide memory alloc/dealloc services
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      06/23/13 Initial version
//      09/09/14 Refactor to store defs in individual headers.
//

#ifndef _MEMFUNCTIONS_H
#define _MEMFUNCTIONS_H

#ifdef __cplusplus
extern "C" {  
#endif

#include "CommonInclude.h"

// Custom error codes
#define CHLE_MEM_ENOMEM     17000
#define CHLE_MEM_GEN        17001

// -------------------------------------------
// Functions exported

DllExpImp BOOL fChlMmAlloc(__out void **pvAddr, __in size_t uSizeBytes, OPTIONAL DWORD *pdwError);
DllExpImp void vChlMmFree(__in void **pvToFree);

#ifdef __cplusplus
}
#endif

#endif // _MEMFUNCTIONS_H
