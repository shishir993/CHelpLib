
// MemFunctions.cpp
// Contains functions that provide memory alloc/dealloc services
// Shishir Bhat (http://www.shishirbhat.com)
// History
//      06/23/13 Initial version
//      09/09/14 Refactor to store defs in individual headers.
//      09/12/14 Naming convention modifications
//

#include "InternalDefines.h"
#include "MemFunctions.h"

// fChlMmAlloc()
// Allocates the requested size in bytes of memory.
// Allocated area is initialized to all zeroes.
// 
HRESULT CHL_MmAlloc(_Out_cap_(uSizeBytes) PVOID *ppvAddr, _In_ size_t uSizeBytes, _In_opt_ PDWORD pdwError)
{
    void *pv = NULL;

    ASSERT(ppvAddr);
    ASSERT(uSizeBytes > 0);

    if((pv = malloc(uSizeBytes)) == NULL)
    {
        goto error_return;
    }
    
    ZeroMemory(pv, uSizeBytes);

    *ppvAddr = pv;
    IFPTR_SETVAL(pdwError, 0);
    return S_OK;

error_return:
    *ppvAddr = NULL;
    IFPTR_SETVAL(pdwError, errno);
    return E_OUTOFMEMORY;
}


// CHL_MmFree()
// Deallocated memory pointed to by pvToFree and sets the provided pointer
// to NULL to prevent accessing free'd locations.
// 
void CHL_MmFree(_In_ PVOID *ppvToFree)
{
    ASSERT(ppvToFree);
    if (*ppvToFree != NULL)
    {
        free(*ppvToFree);
        *ppvToFree = NULL;
    }
    return;
}
