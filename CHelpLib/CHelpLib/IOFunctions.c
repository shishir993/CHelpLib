
// IOFunctions.cpp
// Contains functions that provide IO operation services
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      06/23/13 Initial version
//

#include "IOFunctions.h"

// ChlfReadLineFromStdin()
// Reads input characters from stdin and fills the buffer psBuffer.
// psBuffer is guaranteed to be null terminated upon successful return.
// Atmost dwBufSize - 1 characters will be read and stored into buffer.
// psBuffer     : Pointer to buffer where input characters must be stored
// dwBufSize    : Size in bytes, including null terminator.
// 
DllExpImp BOOL ChlfReadLineFromStdin(OUT TCHAR *psBuffer, IN DWORD dwBufSize)
{
    TCHAR ch;
    DWORD dwReadChars = 0;

    ASSERT(psBuffer);

    if(dwBufSize <= 0)
        return FALSE;

    while(dwReadChars < (dwBufSize-1))
    {
        ch = (TCHAR)getchar();
        if(ch == '\n')
            break;
        psBuffer[dwReadChars] = ch;
        ++dwReadChars;
    }
    psBuffer[dwReadChars] = 0;
    return TRUE;

}// ChlfReadLineFromStdin()
