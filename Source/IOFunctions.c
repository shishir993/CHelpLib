
// IOFunctions.cpp
// Contains functions that provide IO operation services
// Shishir Bhat (http://www.shishirbhat.com)
// History
//      06/23/13 Initial version
//      09/12/14 Naming convention modifications
//

#include "InternalDefines.h"
#include "IOFunctions.h"

// fChlIoReadLineFromStdin()
// Reads input characters from stdin and fills the buffer psBuffer.
// psBuffer is guaranteed to be null terminated upon successful return.
// Atmost dwBufSize - 1 characters will be read and stored into buffer.
// psBuffer     : Pointer to buffer where input characters must be stored
// dwBufSize    : Size in bytes, including null terminator.
// 
HRESULT CHL_IoReadLineFromStdin(_Inout_z_bytecap_x_(dwBufSize) PWSTR pszBuffer, _In_ DWORD dwBufSize)
{
    WCHAR ch;
    DWORD dwReadChars = 0;

    ASSERT(pszBuffer);
    if (dwBufSize <= 0)
    {
        return E_INVALIDARG;
    }

    while (dwReadChars < (dwBufSize - 1))
    {
        ch = (WCHAR)getchar();
        if (ch == '\n')
        {
            break;
        }
        pszBuffer[dwReadChars] = ch;
        ++dwReadChars;
    }
    pszBuffer[dwReadChars] = 0;
    return S_OK;

}// fChlIoReadLineFromStdin()

// fChlIoCreateFileWithSize()
// Creates a file using the specified path (which includes the filename).
// The file is created with read and write access and FILE_ATTRIBUTE_NORMAL
// and the file is always created (CREATE_ALWAYS). If the file exists, it is
// truncated.
// If iSizeBytes parameter > 0, then the newly created file will be filled 
// zero bytes of size iSizeBytes, end of file is set and file pointer is 
// rewinded back to beginning of file.
// pszFilepath  : Pointer to null-terminated string that is the file path
// iSizeBytes   : A number >= 0 that will indicate whether the file should 
//                be set to a particular size or not.
// phFile       : Pointer to HANDLE that will receive the handle to the file
//
HRESULT CHL_IoCreateFileWithSize(_Out_ PHANDLE phFile, _In_z_ PWCHAR pszFilepath, _In_ int iSizeBytes)
{
    HANDLE hFile = NULL;

    PBYTE pbTemp = NULL;

    DWORD dwBytesWritten = 0;
    DWORD dwError = ERROR_SUCCESS;

    HRESULT hr = S_OK;

    // Parameter validation
    if (!pszFilepath || !phFile)
    {
        return E_INVALIDARG;
    }

    hFile = CreateFile(pszFilepath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (iSizeBytes > 0)
    {
        hr = CHL_MmAlloc((void**)&pbTemp, iSizeBytes, NULL);
        if (FAILED(hr))
        {
            goto error_return;
        }

        // Write iSizeBytes of zeroes to file so that we get the desired file size
        if (!WriteFile(hFile, pbTemp, iSizeBytes, &dwBytesWritten, NULL))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto error_return;
        }

        if (dwBytesWritten != (DWORD)iSizeBytes)
        {
            hr = E_FAIL;
            goto error_return;
        }

        CHL_MmFree((void**)&pbTemp);

        if (!SetEndOfFile(hFile))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto error_return;
        }

        // Reset file pointer to beginning of file
        dwError = SetFilePointer(hFile, (LONG)-iSizeBytes, NULL, FILE_CURRENT);
        if (dwError == INVALID_SET_FILE_POINTER)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto error_return;
        }
    }

    *phFile = hFile;
    return hr;

error_return:
    if (hFile)
    {
        CloseHandle(hFile);
        DeleteFile(pszFilepath);
    }

    if (pbTemp)
    {
        CHL_MmFree((void**)&pbTemp);
    }

    return hr;
}
