
// ProcessFunctions.h
// Contains functions that provide IO operation services
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      03/25/14 Initial version
//      09/09/14 Refactor to store defs in individual headers.
//

#ifndef _PROCESSFUNCTIONS_H
#define _PROCESSFUNCTIONS_H

#ifdef __cplusplus
extern "C" {  
#endif

#include "CommonInclude.h"

// Custom error codes
#define CHLE_PROC_DOSHEADER     17150
#define CHLE_PROC_TEXTSECHDR    17151
#define CHLE_PROC_NOEXEC        17152

// -------------------------------------------
// Functions exported

DllExpImp BOOL fChlPsGetProcNameFromID(DWORD pid, WCHAR *pwsProcName, DWORD dwBufSize);
DllExpImp BOOL fChlPsGetNtHeaders(HANDLE hMapView, __out PIMAGE_NT_HEADERS *ppstNtHeaders);
DllExpImp BOOL fChlPsGetPtrToCode(
    DWORD dwFileBase, 
    PIMAGE_NT_HEADERS pNTHeaders, 
    __out PDWORD pdwCodePtr, 
    __out PDWORD pdwSizeOfData,
    __out PDWORD pdwCodeSecVirtAddr);

DllExpImp BOOL fChlPsGetEnclosingSectionHeader(DWORD rva, PIMAGE_NT_HEADERS pNTHeader, __out PIMAGE_SECTION_HEADER *ppstSecHeader);

#ifdef __cplusplus
}
#endif

#endif // _PROCESSFUNCTIONS_H
