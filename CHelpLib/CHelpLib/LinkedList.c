// LinkedList.cpp
// Contains functions that implement a linked list data structure
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      04/05/14 Initial version
//

#include "LinkedList.h"
#include "General.h"

static void vCopyValOut(PLLNODE pnode, LL_VALTYPE valType, __out void **ppValOut);
static BOOL fPopulateNode(PLLNODE pNodeToPopulate, LL_VALTYPE valType, void *pval, int valsize);
static void vInsertNode(PCHL_LLIST pLList, PLLNODE pNodeToInsert);
static void vRemoveNode(PCHL_LLIST pLList, PLLNODE pNodeToRemove);

DllExpImp BOOL fChlDsCreateLL(__out PCHL_LLIST *ppLList, LL_VALTYPE valType, OPTIONAL int nEstEntries)
{
    unsigned int uiRand;
    WCHAR wsMutexName[32];

    PCHL_LLIST pListLocal = NULL;

    // validate parameters
    if(!ppLList)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        goto error_return;
    }
    
    // Create a unique number to be appended to the mutex name
    if( rand_s(&uiRand) != 0 )
    {
        logerr("Unable to get a random number");
        SetLastError(CHLE_LLIST_EINVAL);
        goto error_return;
    }

    swprintf_s(wsMutexName, _countof(wsMutexName), L"%s_%08X", HT_MUTEX_NAME, uiRand);

    if(!fChlMmAlloc((void**)&pListLocal, sizeof(CHL_LLIST), NULL))
    {
        goto error_return;
    }

    // Populate members
    pListLocal->valType = valType;
    pListLocal->nMaxNodes = nEstEntries;
    pListLocal->hMuAccess = CreateMutex(NULL, FALSE, wsMutexName);
    if(pListLocal->hMuAccess == NULL)
    {
        // last error already set by CreateMutex
        goto error_return;
    }

    *ppLList = pListLocal;
    return TRUE;

error_return:
    if(pListLocal)
    {
        vChlMmFree((void**)&pListLocal);
    }
    return FALSE;
}

BOOL fChlDsInsertLL(PCHL_LLIST pLList, void *pval, int valsize)
{
    void *pvValInNode = NULL;
    PLLNODE pnewnode = NULL;

    BOOL fMutexLocked = FALSE;

    // Parameter validation
    if(!pLList || !pval || valsize <= 0)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        goto error_return;
    }

    // Create new node
    if(!fChlMmAlloc((void**)&pnewnode, sizeof(LLNODE), NULL))
    {
        goto error_return;
    }

    // Populate value into new node
    if(!fPopulateNode(pnewnode, pLList->valType, pval, valsize))
    {
        goto error_return;
    }

    // Insert into linked list at the head
    if(!fOwnMutex(pLList->hMuAccess))
    {
        goto error_return;
    }

    fMutexLocked = TRUE;

    vInsertNode(pLList, pnewnode);

    ++(pLList->nCurNodes);

    ReleaseMutex(pLList->hMuAccess);
    return TRUE;

error_return:
    if(pvValInNode)
    {
        vChlMmFree((void**)&pvValInNode);
    }

    if(pnewnode)
    {
        vChlMmFree((void**)&pnewnode);
    }

    if(fMutexLocked)
    {
        ReleaseMutex(pLList->hMuAccess);
    }
    return FALSE;
}

BOOL fChlDsRemoveLL(PCHL_LLIST pLList, __in void *pvValToFind, BOOL fStopOnFirstFind, BOOL (*pfnComparer)(void*, void*))
{
    PLLNODE pCurNode = NULL;
    void *pvCurVal = NULL;
    LL_VALTYPE valType = pLList->valType;

    BOOL fFoundNode = FALSE;
    BOOL fMutexLocked = FALSE;

    if(!pLList || !pvValToFind || !pfnComparer)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        goto error_return;
    }

    if(!fOwnMutex(pLList->hMuAccess))
    {
        goto error_return;
    }

    fMutexLocked = TRUE;

    // Iterate through the list to find
    pCurNode = pLList->pHead;
    while(pCurNode)
    {
        vCopyValOut(pCurNode, valType, &pvCurVal);
        if( pfnComparer(pvValToFind, pvCurVal) )
        {
            fFoundNode = TRUE;

            vRemoveNode(pLList, pCurNode);
            --(pLList->nCurNodes);

            if(fStopOnFirstFind)
            {
                break;
            }
        }

        pCurNode = pCurNode->pright;
    }

    ReleaseMutex(pLList->hMuAccess);
    return fFoundNode;

error_return:
    if(fMutexLocked)
    {
        ReleaseMutex(pLList->hMuAccess);    
    }
    return FALSE;
}

BOOL fChlDsRemoveAtLL(PCHL_LLIST pLList, int iIndexToRemove, __out OPTIONAL void **ppval)
{
    int itr;
    PLLNODE pCurNode;
    BOOL fMutexLocked = FALSE;

    // Validate arguments
    if(!pLList || iIndexToRemove < 0)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        goto error_return;
    }

    if(iIndexToRemove >= pLList->nCurNodes)
    {
        SetLastError(ERROR_INVALID_INDEX);
        goto error_return;
    }

    if(!fOwnMutex(pLList->hMuAccess))
    {
        goto error_return;
    }

    fMutexLocked = TRUE;

    // Loop until we have address of node to remove
    pCurNode = pLList->pHead;
    for(itr = 0; itr < iIndexToRemove; ++itr)
    {
        ASSERT(pCurNode);
        pCurNode = pCurNode->pright;
    }

    ASSERT(pCurNode);
    if(ppval)
    {
        vCopyValOut(pCurNode, pLList->valType, ppval);
    }

    vRemoveNode(pLList, pCurNode);

    ReleaseMutex(pLList->hMuAccess);
    return TRUE;

error_return:
    if(fMutexLocked)
    {
        ReleaseMutex(pLList->hMuAccess);    
    }
    return FALSE;
}

BOOL fChlDsFindLL(PCHL_LLIST pLList, __in void *pvValToFind, BOOL (*pfnComparer)(void*, void*), __out OPTIONAL void **ppval)
{
    BOOL fFoundNode = FALSE;
    BOOL fMutexLocked = FALSE;

    PLLNODE pCurNode = NULL;
    void *pvCurVal = NULL;
    LL_VALTYPE valType = pLList->valType;

    if(!pLList || !pfnComparer)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        goto error_return;
    }

    if(!fOwnMutex(pLList->hMuAccess))
    {
        goto error_return;
    }

    fMutexLocked = TRUE;

    // Iterate through the list to find
    pCurNode = pLList->pHead;
    while(pCurNode)
    {
        vCopyValOut(pCurNode, valType, &pvCurVal);
        if( pfnComparer(pvValToFind, pvCurVal) )
        {
            fFoundNode = TRUE;
            break;
        }

        pCurNode = pCurNode->pright;
    }

    if(fFoundNode)
    {
        if(ppval)
        {
            vCopyValOut(pCurNode, valType, ppval);
        }
    }

    ReleaseMutex(pLList->hMuAccess);
    return fFoundNode;

error_return:
    if(fMutexLocked)
    {
        ReleaseMutex(pLList->hMuAccess);    
    }
    return FALSE;
}

BOOL fChlDsDestroyLL(PCHL_LLIST pLList)
{
    PLLNODE pCurNode, pNextNode;
    LL_VALTYPE valType;

    if(!pLList)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        goto error_return;
    }

    if(!fOwnMutex(pLList->hMuAccess))
    {
        goto error_return;
    }

    valType = pLList->valType;

    // Iterate through the list and delete nodes
    pCurNode = pLList->pHead;
    while(pCurNode)
    {
        pNextNode = pCurNode->pright;

        if(valType == LL_VAL_PTR)
        {
            vChlMmFree((void**)&pCurNode->nodeval.pval);
        }

        vChlMmFree((void**)&pCurNode);
        pCurNode = pNextNode;
    }

    // Don't release mutex because no other thread must be able to use
    // this linked list now.
    CloseHandle(pLList->hMuAccess);

    vChlMmFree((void**)&pLList);
    
    return TRUE;

error_return:
    return FALSE;
}

static void vCopyValOut(PLLNODE pnode, LL_VALTYPE valType, __out void **ppValOut)
{
    ASSERT(pnode);
    ASSERT(valType >= LL_VAL_BYTE && valType <= LL_VAL_PTR);
    ASSERT(ppValOut);

    switch(valType)
    {
        case LL_VAL_BYTE:
        case LL_VAL_UINT:
        case LL_VAL_DWORD:
        case LL_VAL_LONGLONG:
        {
            SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
            ASSERT(FALSE);
        }

        case LL_VAL_PTR:
        {
            *ppValOut = pnode->nodeval.pval;
            break;
        }

        default:
        {
            ASSERT(FALSE);
            break;
        }
    }
}

static BOOL fPopulateNode(PLLNODE pNodeToPopulate, LL_VALTYPE valType, void *pval, int valsize)
{
    ASSERT(pNodeToPopulate);
    ASSERT(pval);
    ASSERT(valsize > 0);
    
    pNodeToPopulate->dwValSize = valsize;
    switch(valType)
    {
        case LL_VAL_BYTE:
        case LL_VAL_UINT:
        case LL_VAL_DWORD:
        case LL_VAL_LONGLONG:
        {
            SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
            return FALSE;
        }

        case LL_VAL_PTR:
        {
            if(!fChlMmAlloc((void**)&pNodeToPopulate->nodeval.pval, valsize, NULL))
            {
                return FALSE;
            }

            memcpy(pNodeToPopulate->nodeval.pval, pval, valsize);
            break;
        }

        default:
        {
            SetLastError(CHLE_LLIST_VALTYPE);
            return FALSE;
        }
    }

    return TRUE;
}

static void vInsertNode(PCHL_LLIST pLList, PLLNODE pNodeToInsert)
{
    ASSERT(pLList);
    ASSERT(pNodeToInsert);

    // Linked list empty?
    if(pLList->pHead == NULL)
    {
        ASSERT(pLList->nCurNodes == 0);

        pNodeToInsert->pleft = pNodeToInsert->pright = NULL;
        pLList->pHead = pNodeToInsert;
    }
    else
    {
        ASSERT(pLList->nCurNodes > 0);

        // Insert at the beginning
        pNodeToInsert->pright = pLList->pHead;
        pNodeToInsert->pleft = NULL;

        pLList->pHead->pleft = pNodeToInsert;
        pLList->pHead = pNodeToInsert;
    }
}

static void vRemoveNode(PCHL_LLIST pLList, PLLNODE pNodeToRemove)
{
    ASSERT(pLList);
    ASSERT(pNodeToRemove);

    // Removing the first one?
    if(pNodeToRemove->pleft == NULL)
    {
        // Removing the first node, so this should be true
        ASSERT(pLList->pHead == pNodeToRemove);

        pLList->pHead = pNodeToRemove->pright;
        if(pNodeToRemove->pright)
        {
            pNodeToRemove->pright->pleft = pNodeToRemove->pleft;
        }
    }
    else
    {
        // Not removing the first node, so this should be true
        ASSERT(pLList->pHead != pNodeToRemove);

        if(pNodeToRemove->pleft)
        {
            pNodeToRemove->pleft->pright = pNodeToRemove->pright;
        }

        if(pNodeToRemove->pright)
        {
            pNodeToRemove->pright->pleft = pNodeToRemove->pleft;
        }
    }

    // Free memory occupied by the value, if applicable
    if(pLList->valType == LL_VAL_PTR)
    {
        vChlMmFree((void**)&pNodeToRemove->nodeval.pval);
    }

    // Finally, free the node itself
    vChlMmFree((void**)pNodeToRemove);
}
