
// General.c
// Contains general helper functions
// Shishir Bhat (http://www.shishirbhat.com)
// History
//      01/20/14 Initial version
//      09/12/14 Naming convention modifications
//

#include "InternalDefines.h"
#include "General.h"

#define MAPVIEW_NAME    L"Chl_FileMapViewName"

#if !defined(_WIN64)

// CHL_ChlGnIsOverflowINT()
// Given two integers, returns TRUE if adding them results
// in overflow, FALSE otherwise.
//
BOOL CHL_GnIsOverflowINT(_In_ int a, _In_ int b)
{
    __asm
    {
        mov eax, dword ptr[a]
        mov ecx, dword ptr[b]
        add eax, ecx
        jo ret_overflow
    }

    return FALSE;

ret_overflow:
    return TRUE;
}

// CHL_ChlGnIsOverflowUINT()
// Given two unsigned integers, returns TRUE if 
// adding them results in overflow, FALSE otherwise.
//
BOOL CHL_GnIsOverflowUINT(_In_ UINT a, _In_ UINT b)
{
    __asm
    {
        mov eax, dword ptr[a]
        mov ecx, dword ptr[b]
        add eax, ecx
        jc ret_overflow
    }

    return FALSE;

ret_overflow:
    return TRUE;
}

#endif // _WIN64

// Create a memory mapping given a handle to a file and return
// return the handle to the memory mapped area.
// 
HRESULT CHL_GnCreateMemMapOfFile(
    _In_ HANDLE hFile,
    _In_ DWORD dwReqProtection,
    _Out_ PHANDLE phMapObj,
    _Out_ PHANDLE phMapView)
{
    DWORD dwRetVal = 0;
    DWORD dwFileSize = 0;

    HANDLE hFileMapObj = NULL;
    HANDLE hFileMapView = NULL;

    // Validate arguments
    HRESULT hr = S_OK;
    if (!ISVALID_HANDLE(hFile) || !phMapObj || !phMapView)
    {
        hr = E_INVALIDARG;
        goto error_return;
    }

    DBG_UNREFERENCED_PARAMETER(dwReqProtection);
    if ((dwRetVal = GetFileSize(hFile, &dwFileSize)) == INVALID_FILE_SIZE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto error_return;
    }

    // dwRetVal is filesize LOW word and dwFileSize will have HIGH word
    if (dwRetVal == 0 && dwFileSize == 0)
    {
        hr = HRESULT_FROM_WIN32(ERROR_EMPTY);
        goto error_return;
    }

    // Create a memory mapping for the file with GENERIC_READ
    // that is, create the file mapping object
    if ((hFileMapObj = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, MAPVIEW_NAME)) == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto error_return;
    }

    // check for ERROR_ALREADY_EXISTS ??

    // map this file mapping object into our address space
    if ((hFileMapView = MapViewOfFile(hFileMapObj, FILE_MAP_READ, 0, 0, 0)) == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto error_return;
    }

    *phMapObj = hFileMapObj;
    *phMapView = hFileMapView;
    return hr;

error_return:
    // DO NOT close handle hFile because it was given to us by the caller
    if (ISVALID_HANDLE(hFileMapObj))
    {
        CloseHandle(hFileMapObj);
    }
    return hr;
}

// Try to get ownership of mutex. This function tries to get the mutex
// with 10 retries with a 500ms interval between each retry.
//
HRESULT CHL_GnOwnMutex(HANDLE hMutex)
{
    int nTries = 0;
    HRESULT hr = S_OK;

    ASSERT(hMutex && hMutex != INVALID_HANDLE_VALUE);
    while (nTries < 10)
    {
        switch (WaitForSingleObject(hMutex, 500))
        {
        case WAIT_OBJECT_0:
            goto got_it;

        case WAIT_ABANDONED:
            hr = HRESULT_FROM_WIN32(ERROR_ABANDONED_WAIT_0);
            goto got_it;

        case WAIT_TIMEOUT:
            hr = HRESULT_FROM_WIN32(ERROR_TIMEOUT);
            break;

        case WAIT_FAILED:
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }
        ++nTries;
    }

got_it:
    return hr;
}
