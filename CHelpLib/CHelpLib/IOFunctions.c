
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
