
// General.c
// Contains general helper functions
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      01/20/14 Initial version
//

#include "General.h"

#define MAPVIEW_NAME    L"Chl_FileMapViewName"

// fChlGnIsOverflowINT()
// Given two integers, returns TRUE if adding them results
// in overflow, FALSE otherwise.
//
BOOL fChlGnIsOverflowINT(int a, int b)
{
    __asm
    {
        mov eax, dword ptr [a]
        mov ecx, dword ptr [b]
        add eax, ecx
        jo ret_overflow
    }

    return FALSE;
    
ret_overflow:
    return TRUE;
}

// fChlGnIsOverflowUINT()
// Given two unsigned integers, returns TRUE if 
// adding them results in overflow, FALSE otherwise.
//
BOOL fChlGnIsOverflowUINT(unsigned int a, unsigned int b)
{
    __asm 
    {
        mov eax, dword ptr [a]
        mov ecx, dword ptr [b]
        add eax, ecx
        jc ret_overflow
    }

    return FALSE;
    
ret_overflow:
    return TRUE;
}

// Try to get ownership of mutex. This function tries to get the mutex
// with 10 retries with a 500ms interval between each retry.
//
BOOL fChlGnOwnMutex(HANDLE hMutex)
{
    int nTries = 0;

    ASSERT(hMutex && hMutex != INVALID_HANDLE_VALUE);

    while(nTries < 10)
    {
        switch(WaitForSingleObject(hMutex, 500))
        {
            case WAIT_OBJECT_0:
                goto got_it;

            case WAIT_ABANDONED: // todo
                return FALSE;

            case WAIT_TIMEOUT:
            case WAIT_FAILED:
                break;
        }
        ++nTries;
    }

    if(nTries == 10)
    {
        SetLastError(CHLE_MUTEX_TIMEOUT);
        return FALSE;
    }

got_it:
    return TRUE;
}

// Create a memory mapping given a handle to a file and return
// return the handle to the memory mapped area.
// 
BOOL fChlGnCreateMemMapOfFile(HANDLE hFile,  DWORD dwReqProtection, __out PHANDLE phMapObj, __out PHANDLE phMapView)
{
    DWORD dwRetVal = 0;
	DWORD dwFileSize = 0;

    HANDLE hFileMapObj = NULL;
    HANDLE hFileMapView = NULL;

    // Validate arguments
    if(!ISVALID_HANDLE(hFile) || !phMapObj || !phMapView)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        goto error_return;
    }

    DBG_UNREFERENCED_PARAMETER(dwReqProtection);

	if( (dwRetVal = GetFileSize(hFile, &dwFileSize)) == INVALID_FILE_SIZE )
	{
		goto error_return;
	}

    // dwRetVal is filesize LOW word and dwFileSize will have HIGH word

	if(dwRetVal == 0 && dwFileSize == 0)
	{
        SetLastError(CHLE_EMPTY_FILE);
		goto error_return;
	}

	// Create a memory mapping for the file with GENERIC_READ
	// that is, create the file mapping object
	if( (hFileMapObj = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, MAPVIEW_NAME)) == NULL )
	{
		goto error_return;
	}

	// check for ERROR_ALREADY_EXISTS ??

	// map this file mapping object into our address space
	if( (hFileMapView = MapViewOfFile(hFileMapObj, FILE_MAP_READ, 0, 0, 0)) == NULL )
	{
		goto error_return;
	}

    *phMapObj = hFileMapObj;
    *phMapView = hFileMapView;

    return TRUE;

error_return:
    // DO NOT close handle hFile because it was given to us by the caller

    if(ISVALID_HANDLE(hFileMapObj))
    {
        CloseHandle(hFileMapObj);
    }

    return FALSE;
}
