
// MemFunctions.cpp
// Contains functions that provide memory alloc/dealloc services
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      06/23/13 Initial version
//

#include "MemFunctions.h"

// ChlfMemAlloc()
// Allocates the requested size in bytes of memory
// 
DllExpImp BOOL ChlfMemAlloc(OUT void **pvAddr, IN size_t uSizeBytes, OPTIONAL DWORD *pdwError)
{
    void *pv = NULL;

    ASSERT(pvAddr);
    ASSERT(uSizeBytes > 0);

    if((pv = malloc(uSizeBytes)) == NULL)
        goto error_return;
    
    *pvAddr = pv;
    IFPTR_SETVAL(pdwError, 0);
    return TRUE;

    error_return:
    *pvAddr = NULL;
    IFPTR_SETVAL(pdwError, 0);
    return FALSE;
}// ChlfMemAlloc()


// ChlvMemFree()
// Deallocated memory pointed to by pvToFree and sets the provided pointer
// to NULL to prevent accessing free'd locations.
// 
DllExpImp void ChlvMemFree(IN void **pvToFree)
{
    ASSERT(pvToFree && *pvToFree);
    free(*pvToFree);
    *pvToFree = NULL;
    return;
}
