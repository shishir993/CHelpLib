
// IOFunctions.cpp
// Contains functions that provide IO operation services
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      06/23/13 Initial version
//

#include "IOFunctions.h"

// fChlIoReadLineFromStdin()
// Reads input characters from stdin and fills the buffer psBuffer.
// psBuffer is guaranteed to be null terminated upon successful return.
// Atmost dwBufSize - 1 characters will be read and stored into buffer.
// psBuffer     : Pointer to buffer where input characters must be stored
// dwBufSize    : Size in bytes, including null terminator.
// 
DllExpImp BOOL fChlIoReadLineFromStdin(__in DWORD dwBufSize, __out WCHAR *psBuffer)
{
    WCHAR ch;
    DWORD dwReadChars = 0;

    ASSERT(psBuffer);

    if(dwBufSize <= 0)
        return FALSE;

    while(dwReadChars < (dwBufSize-1))
    {
        ch = (WCHAR)getchar();
        if(ch == '\n')
        {
            break;
        }
        psBuffer[dwReadChars] = ch;
        ++dwReadChars;
    }
    psBuffer[dwReadChars] = 0;
    return TRUE;

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
BOOL fChlIoCreateFileWithSize(__in PWCHAR pszFilepath, __in int iSizeBytes, __out PHANDLE phFile)
{
    HANDLE hFile = NULL;

    PBYTE pbTemp = NULL;

    DWORD dwBytesWritten = 0;
    DWORD dwError = ERROR_SUCCESS;

    // Parameter validation
    if(!pszFilepath || iSizeBytes < 0 || !phFile)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        return FALSE;
    }

    hFile = CreateFile(pszFilepath, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if(iSizeBytes > 0)
    {
        if(!fChlMmAlloc((void**)&pbTemp, iSizeBytes, NULL))
        {
            goto error_return;
        }

        // Write iSizeBytes of zeroes to file so that we get the desired file size
        if(!WriteFile(hFile, pbTemp, iSizeBytes, &dwBytesWritten, NULL))
        {
            goto error_return;
        }

        if(dwBytesWritten != iSizeBytes)
        {
            goto error_return;
        }

        vChlMmFree((void**)&pbTemp);

        if(!SetEndOfFile(hFile))
        {
            goto error_return;
        }

        // Reset file pointer to beginning of file
        dwError = SetFilePointer(hFile, (LONG)-iSizeBytes, NULL, FILE_CURRENT);
        if(dwError == INVALID_SET_FILE_POINTER)
        {
            goto error_return;
        }
    }

    *phFile = hFile;

    return TRUE;

error_return:
    if(hFile)
    {
        CloseHandle(hFile);
        DeleteFile(pszFilepath);
    }

    if(pbTemp)
    {
        vChlMmFree((void**)&pbTemp);
    }

    return FALSE;
}
