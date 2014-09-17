// LinkedList.cpp
// Contains functions that implement a linked list data structure
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      04/05/14 Initial version
//      04/10/14 Changed to insert at end logic
//      09/12/14 Naming convention modifications
//

#include "LinkedList.h"
#include "General.h"
#include "MemFunctions.h"

static void _CopyValOut(PLLNODE pnode, LL_VALTYPE valType, _Out_ PVOID *ppValOut);
static HRESULT _PopulateNode(PLLNODE pNodeToPopulate, LL_VALTYPE valType, PVOID pval, int valsize);
static void _InsertNode(PCHL_LLIST pLList, PLLNODE pNodeToInsert);
static void _UnlinkNode(PCHL_LLIST pLList, PLLNODE pNodeToRemove);
static void _FreeNodeMem(PLLNODE pnode, LL_VALTYPE valType, BOOL fFreeValMem);

HRESULT CHL_DsCreateLL(_Out_ PCHL_LLIST *ppLList, _In_ LL_VALTYPE valType, _In_opt_ int nEstEntries)
{
    unsigned int uiRand;
    WCHAR wsMutexName[32];

    PCHL_LLIST pListLocal = NULL;

    HRESULT hr = S_OK;

    // validate parameters
    if(!ppLList)
    {
        hr = E_INVALIDARG;
        goto error_return;
    }
    
    // Create a unique number to be appended to the mutex name
    if( rand_s(&uiRand) != 0 )
    {
        logerr("Unable to get a random number");
        hr = E_FAIL;
        goto error_return;
    }

    swprintf_s(wsMutexName, _countof(wsMutexName), L"%s_%08X", HT_MUTEX_NAME, uiRand);

    hr = CHL_MmAlloc((PVOID*)&pListLocal, sizeof(CHL_LLIST), NULL);
    if(FAILED(hr))
    {
        goto error_return;
    }

    // Populate members
    pListLocal->valType = valType;
    pListLocal->nMaxNodes = nEstEntries;
    pListLocal->hMuAccess = CreateMutex(NULL, FALSE, wsMutexName);
    if(pListLocal->hMuAccess == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto error_return;
    }

    *ppLList = pListLocal;
    return hr;

error_return:
    if(pListLocal)
    {
        CHL_MmFree((PVOID*)&pListLocal);
    }
    return hr;
}

HRESULT CHL_DsInsertLL(_In_ PCHL_LLIST pLList, _In_ PVOID pval, _In_ int valsize)
{
    PVOID pvValInNode = NULL;
    PLLNODE pnewnode = NULL;

    BOOL fMutexLocked = FALSE;

    HRESULT hr = S_OK;

    // Parameter validation
    if(!pLList || !pval || valsize <= 0)
    {
        hr = E_INVALIDARG;
        goto error_return;
    }

    // Create new node
    hr = CHL_MmAlloc((PVOID*)&pnewnode, sizeof(LLNODE), NULL);
    if(FAILED(hr))
    {
        goto error_return;
    }

    // Populate value into new node
    hr = _PopulateNode(pnewnode, pLList->valType, pval, valsize);
    if(FAILED(hr))
    {
        goto error_return;
    }

    hr = CHL_GnOwnMutex(pLList->hMuAccess);
    if(FAILED(hr))
    {
        goto error_return;
    }

    fMutexLocked = TRUE;

    _InsertNode(pLList, pnewnode);
    ++(pLList->nCurNodes);

    ReleaseMutex(pLList->hMuAccess);
    return hr;

error_return:
    if(pvValInNode)
    {
        CHL_MmFree((PVOID*)&pvValInNode);
    }

    if(pnewnode)
    {
        CHL_MmFree((PVOID*)&pnewnode);
    }

    if(fMutexLocked)
    {
        ReleaseMutex(pLList->hMuAccess);
    }
    return hr;
}

HRESULT CHL_DsRemoveLL(
    _In_ PCHL_LLIST pLList, 
    _In_ PVOID pvValToFind, 
    _In_ BOOL fStopOnFirstFind, 
    _In_ BOOL (*pfnComparer)(PVOID, PVOID), 
    _Inout_opt_ PVOID *ppval)
{
    PLLNODE pCurNode = NULL;
    PVOID pvCurVal = NULL;
    LL_VALTYPE valType = pLList->valType;

    BOOL fMutexLocked = FALSE;

    HRESULT hr = S_OK;
    if(!pLList || !pvValToFind || !pfnComparer)
    {
        hr = E_INVALIDARG;
        goto error_return;
    }

    hr = CHL_GnOwnMutex(pLList->hMuAccess);
    if(FAILED(hr))
    {
        goto error_return;
    }

    fMutexLocked = TRUE;

    // Iterate through the list to find
    pCurNode = pLList->pHead;
    hr = E_NOT_SET;
    while(pCurNode)
    {
        _CopyValOut(pCurNode, valType, &pvCurVal);
        if( pfnComparer(pvValToFind, pvCurVal) )
        {
            hr = S_OK;

            _UnlinkNode(pLList, pCurNode);
            
            if(ppval)
            {
                *ppval = pvCurVal;

                // DO NOT free value memory since we are returning it
                _FreeNodeMem(pCurNode, valType, FALSE);
            }
            else
            {
                _FreeNodeMem(pCurNode, valType, TRUE);
            }

            --(pLList->nCurNodes);

            if(fStopOnFirstFind)
            {
                break;
            }
        }

        pCurNode = pCurNode->pright;
    }

    ReleaseMutex(pLList->hMuAccess);
    return hr;

error_return:
    if(fMutexLocked)
    {
        ReleaseMutex(pLList->hMuAccess);    
    }
    return hr;
}

HRESULT CHL_DsRemoveAtLL(_In_ PCHL_LLIST pLList, _In_ int iIndexToRemove, _Inout_opt_ PVOID *ppval)
{
    int itr;
    PLLNODE pCurNode;
    BOOL fMutexLocked = FALSE;

    HRESULT hr = S_OK;
    if(!pLList || iIndexToRemove < 0)
    {
        hr = E_INVALIDARG;
        goto error_return;
    }

    if(iIndexToRemove >= pLList->nCurNodes)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
        goto error_return;
    }

    hr = CHL_GnOwnMutex(pLList->hMuAccess);
    if(FAILED(hr))
    {
        goto error_return;
    }

    fMutexLocked = TRUE;

    // Loop until we have address of node to remove
    pCurNode = pLList->pHead;
    for(itr = 1; itr <= iIndexToRemove; ++itr)
    {
        ASSERT(pCurNode);
        pCurNode = pCurNode->pright;
    }

    ASSERT(pCurNode);

    _UnlinkNode(pLList, pCurNode);
    --(pLList->nCurNodes);

    if(ppval)
    {
        _CopyValOut(pCurNode, pLList->valType, ppval);

        // DO NOT free value memory since we are returning it
        _FreeNodeMem(pCurNode, pLList->valType, FALSE);
    }
    else
    {
        _FreeNodeMem(pCurNode, pLList->valType, TRUE);
    }

    ReleaseMutex(pLList->hMuAccess);
    return hr;

error_return:
    if(fMutexLocked)
    {
        ReleaseMutex(pLList->hMuAccess);    
    }
    return hr;
}

HRESULT CHL_DsPeekAtLL(_In_ PCHL_LLIST pLList, _In_ int iIndexToPeek, _Inout_ PVOID *ppval)
{
    BOOL fMutexLocked = FALSE;
    PLLNODE pCurNode = NULL;

    HRESULT hr = S_OK;
    if(!pLList || iIndexToPeek < 0 || iIndexToPeek >= pLList->nCurNodes)
    {
        hr = E_INVALIDARG;
        goto error_return;
    }

    hr = CHL_GnOwnMutex(pLList->hMuAccess);
    if(FAILED(hr))
    {
        goto error_return;
    }
    fMutexLocked = TRUE;

    hr = E_NOT_SET; // Start with this, set to S_OK if node is found
    if(iIndexToPeek == 0)
    {
        hr = S_OK;
        _CopyValOut(pLList->pHead, pLList->valType, ppval);
    }
    else if(iIndexToPeek == (pLList->nCurNodes - 1))
    {
        hr = S_OK;
        _CopyValOut(pLList->pTail, pLList->valType, ppval);
    }
    else
    {
        int itr;

        hr = S_OK;

        // Iterate through the list to the specified index
        pCurNode = pLList->pHead;
        for(itr = 0; itr < iIndexToPeek; ++itr)
        {
            pCurNode = pCurNode->pright;
        }

        ASSERT(pCurNode != NULL);
        _CopyValOut(pCurNode, pLList->valType, ppval);
    }

    ReleaseMutex(pLList->hMuAccess);
    return hr;

error_return:
    if(fMutexLocked)
    {
        ReleaseMutex(pLList->hMuAccess);    
    }
    return hr;
}

HRESULT CHL_DsFindLL(
    _In_ PCHL_LLIST pLList, 
    _In_ PVOID pvValToFind, 
    _In_ BOOL (*pfnComparer)(PVOID, PVOID), 
    _Inout_opt_ PVOID *ppval)
{
    BOOL fMutexLocked = FALSE;

    PLLNODE pCurNode = NULL;
    PVOID pvCurVal = NULL;
    LL_VALTYPE valType = pLList->valType;

    HRESULT hr = S_OK;
    if(!pLList || !pfnComparer)
    {
        hr = E_INVALIDARG;
        goto error_return;
    }

    hr = CHL_GnOwnMutex(pLList->hMuAccess);
    if(FAILED(hr))
    {
        goto error_return;
    }

    fMutexLocked = TRUE;

    // Iterate through the list to find
    pCurNode = pLList->pHead;
    hr = E_NOT_SET;
    while(pCurNode)
    {
        _CopyValOut(pCurNode, valType, &pvCurVal);
        if( pfnComparer(pvValToFind, pvCurVal) )
        {
            hr = S_OK;
            break;
        }
        pCurNode = pCurNode->pright;
    }

    if(SUCCEEDED(hr) && ppval != NULL)
    {
        _CopyValOut(pCurNode, valType, ppval);
    }

    ReleaseMutex(pLList->hMuAccess);
    return hr;

error_return:
    if(fMutexLocked)
    {
        ReleaseMutex(pLList->hMuAccess);    
    }
    return hr;
}

DllExpImp HRESULT CHL_DsDestroyLL(_In_ PCHL_LLIST pLList)
{
    PLLNODE pCurNode, pNextNode;
    LL_VALTYPE valType;

    HRESULT hr = S_OK;
    if(!pLList)
    {
        hr = E_INVALIDARG;
        goto error_return;
    }

    hr = CHL_GnOwnMutex(pLList->hMuAccess);
    if(FAILED(hr))
    {
        goto error_return;
    }

    valType = pLList->valType;

    // Iterate through the list and delete nodes
    pCurNode = pLList->pHead;
    while(pCurNode)
    {
        pNextNode = pCurNode->pright;

        _FreeNodeMem(pCurNode, valType, TRUE);

        pCurNode = pNextNode;
    }

    // Don't release mutex because no other thread must be able to use
    // this linked list now.
    CloseHandle(pLList->hMuAccess);
    CHL_MmFree((PVOID*)&pLList);
    return hr;

error_return:
    return hr;
}

static void _CopyValOut(PLLNODE pnode, LL_VALTYPE valType, _Out_ PVOID *ppValOut)
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
            memcpy(ppValOut, pnode->nodeval.pval, pnode->dwValSize);
            //*ppValOut = pnode->nodeval.pval;
            break;
        }

        default:
        {
            ASSERT(FALSE);
            break;
        }
    }
}

static HRESULT _PopulateNode(PLLNODE pNodeToPopulate, LL_VALTYPE valType, PVOID pval, int valsize)
{
    HRESULT hr = S_OK;

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
            hr = E_NOTIMPL;
            break;
        }

        case LL_VAL_PTR:
        {
            hr = CHL_MmAlloc((PVOID*)&pNodeToPopulate->nodeval.pval, valsize, NULL);
            if(SUCCEEDED(hr))
            {
                memcpy(pNodeToPopulate->nodeval.pval, pval, valsize);
            }
            break;
        }

        default:
        {
            hr = E_UNEXPECTED;
            break;
        }
    }

    return hr;
}

static void _InsertNode(PCHL_LLIST pLList, PLLNODE pNodeToInsert)
{
    ASSERT(pLList);
    ASSERT(pNodeToInsert);

    // Linked list empty?
    if(pLList->pHead == NULL)
    {
        ASSERT(pLList->nCurNodes == 0);

        pNodeToInsert->pleft = pNodeToInsert->pright = NULL;
        pLList->pHead = pNodeToInsert;
        pLList->pTail = pNodeToInsert;
    }
    else
    {
        ASSERT(pLList->nCurNodes > 0);
        ASSERT(pLList->pTail);

        // Insert at the end

        // First, set new node's links
        pNodeToInsert->pright = NULL;
        pNodeToInsert->pleft = pLList->pTail;

        // Link to tail and reassign
        pLList->pTail->pright = pNodeToInsert;
        pLList->pTail = pNodeToInsert;
    }
}

static void _UnlinkNode(PCHL_LLIST pLList, PLLNODE pNodeToRemove)
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

        // Could be first and last node as well!
        if(pNodeToRemove == pLList->pTail)
        {
            ASSERT(pNodeToRemove->pright == NULL);

            pLList->pTail = NULL;
        }
    }
    else
    {
        // Not removing the first node, so this should be true
        ASSERT(pLList->pHead != pNodeToRemove);

        // Removing the last one?
        if(pNodeToRemove == pLList->pTail)
        {
            pLList->pTail = pNodeToRemove->pleft;
        }

        if(pNodeToRemove->pleft)
        {
            pNodeToRemove->pleft->pright = pNodeToRemove->pright;
        }

        if(pNodeToRemove->pright)
        {
            pNodeToRemove->pright->pleft = pNodeToRemove->pleft;
        }
    }
}

static void _FreeNodeMem(PLLNODE pnode, LL_VALTYPE valType, BOOL fFreeValMem)
{
    ASSERT(pnode);

    // Free memory occupied by the value, if applicable
    if(fFreeValMem && valType == LL_VAL_PTR)
    {
        CHL_MmFree((PVOID*)&pnode->nodeval.pval);
    }

    // Finally, free the node itself
    CHL_MmFree((PVOID*)&pnode);
}
