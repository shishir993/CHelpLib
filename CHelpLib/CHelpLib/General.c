
// General.c
// Contains general helper functions
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      01/20/14 Initial version
//

#include "General.h"

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
BOOL fOwnMutex(HANDLE hMutex)
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
