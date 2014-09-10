
// General.h
// Contains general helper functions
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      01/20/14 Initial version
//

#ifndef _GENERAL_H
#define _GENERAL_H

#ifdef __cplusplus
extern "C" {  
#endif

#include "CommonInclude.h"

// -------------------------------------------
// Functions exported

DllExpImp BOOL fChlGnIsOverflowINT(int a, int b);
DllExpImp BOOL fChlGnIsOverflowUINT(unsigned int a, unsigned int b);
DllExpImp BOOL fChlGnOwnMutex(HANDLE hMutex);
DllExpImp BOOL fChlGnCreateMemMapOfFile(HANDLE hFile, DWORD dwReqProtection, __out PHANDLE phMapObj, __out PHANDLE phMapView);

// -------------------------------------------
// Functions used only within this library

BOOL fOwnMutex(HANDLE hMutex);

#ifdef __cplusplus
}
#endif

#endif // _GENERAL_H
