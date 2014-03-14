
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
