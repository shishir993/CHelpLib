
// IOFunctions.h
// Contains functions that provide IO operation services
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      06/23/13 Initial version
//      09/09/14 Refactor to store defs in individual headers.
//

#ifndef _IOFUNCTIONS_H
#define _IOFUNCTIONS_H

#ifdef __cplusplus
extern "C" {  
#endif

#include "CommonInclude.h"
#include "MemFunctions.h"

// -------------------------------------------
// Functions exported

DllExpImp BOOL fChlIoReadLineFromStdin(__in DWORD dwBufSize, __out WCHAR *psBuffer);
DllExpImp BOOL fChlIoCreateFileWithSize(__in PWCHAR pszFilepath, __in int iSizeBytes, __out PHANDLE phFile);


#ifdef __cplusplus
}
#endif

#endif // _IOFUNCTIONS_H
