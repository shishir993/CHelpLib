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

#define MUTEX_NAME_LL   (TEXT("CHL_MUTEX_LL"))

// Local functions
static void _InsertNode(PCHL_LLIST pLList, PLLNODE pNodeToInsert);
static void _UnlinkNode(PCHL_LLIST pLList, PLLNODE pNodeToRemove);
static void _FreeNodeMem(PLLNODE pnode, CHL_VALTYPE valType, BOOL fFreeValMem);

HRESULT CHL_DsCreateLL(_Out_ PCHL_LLIST *ppLList, _In_ CHL_VALTYPE valType, _In_opt_ int nEstEntries)
{
    unsigned int uiRand;
    WCHAR wsMutexName[32];

    PCHL_LLIST pListLocal = NULL;

    HRESULT hr = S_OK;

    // validate parameters
    if(!ppLList || 
        !((valType > CHL_VT_START) && (valType < CHL_VT_END)))
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
    swprintf_s(wsMutexName, _countof(wsMutexName), L"%s_%08X", MUTEX_NAME_LL, uiRand);

    hr = CHL_MmAlloc((PVOID*)&pListLocal, sizeof(CHL_LLIST), NULL);
    if(FAILED(hr))
    {
        // pListLocal will be NULL on error
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

    pListLocal->Insert = CHL_DsInsertLL;
    pListLocal->Remove = CHL_DsRemoveLL;
    pListLocal->RemoveAt = CHL_DsRemoveAtLL;
    pListLocal->Peek = CHL_DsPeekAtLL;
    pListLocal->Find = CHL_DsFindLL;
    pListLocal->Destroy = CHL_DsDestroyLL;

    *ppLList = pListLocal;
    return hr;

error_return:
    if(pListLocal)
    {
        CHL_MmFree((PVOID*)&pListLocal);
    }
    return hr;
}

HRESULT CHL_DsInsertLL(_In_ PCHL_LLIST pLList, _In_ PCVOID pvVal, _In_opt_ int iValSize)
{
    PLLNODE pNewNode = NULL;

    BOOL fMutexLocked = FALSE;

    HRESULT hr = S_OK;

    // Parameter validation
    if(!pLList || !pvVal || iValSize <= 0)
    {
        hr = E_INVALIDARG;
        goto error_return;
    }

    // Create new node
    hr = CHL_MmAlloc((PVOID*)&pNewNode, sizeof(LLNODE), NULL);
    if(FAILED(hr))
    {
        goto error_return;
    }

    // Populate value into new node
    pNewNode->dwValSize = iValSize;
    hr = _CopyValIn(&pNewNode->chlVal, pLList->valType, pvVal, iValSize);
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

    _InsertNode(pLList, pNewNode);
    ++(pLList->nCurNodes);

    ReleaseMutex(pLList->hMuAccess);
    return hr;

error_return:
    if(pNewNode)
    {
        CHL_MmFree((PVOID*)&pNewNode);
    }

    if(fMutexLocked && !ReleaseMutex(pLList->hMuAccess))
    {
        logerr("%s(): error_return mutex unlock", __FUNCTION__);
    }
    return hr;
}

HRESULT CHL_DsRemoveLL(
    _In_ PCHL_LLIST pLList, 
    _In_ PCVOID pvValToFind, 
    _In_ BOOL fStopOnFirstFind, 
    _In_ BOOL (*pfnComparer)(PVOID, PVOID))
{
    PLLNODE pCurNode = NULL;
    PVOID pvCurVal = NULL;
    CHL_VALTYPE valType = pLList->valType;

    BOOL fMutexLocked = FALSE;

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
        _CopyValOut(&pCurNode->chlVal, valType, &pvCurVal, TRUE);
        if( pfnComparer(pvValToFind, pvCurVal) )
        {
            hr = S_OK;

            _UnlinkNode(pLList, pCurNode);
            _FreeNodeMem(pCurNode, valType, TRUE);

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
    if(fMutexLocked && !ReleaseMutex(pLList->hMuAccess))
    {
        logerr("%s(): error_return mutex unlock", __FUNCTION__);
    }
    return hr;
}

HRESULT CHL_DsRemoveAtLL(
    _In_ PCHL_LLIST pLList, 
    _In_ int iIndexToRemove, 
    _Inout_opt_ PVOID pvValOut, 
    _In_opt_ BOOL fGetPointerOnly)
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

    if(pvValOut)
    {
        hr = _CopyValOut(&pCurNode->chlVal, pLList->valType, pvValOut, fGetPointerOnly);
    }

    if(SUCCEEDED(hr))
    {
        // Unlink and only then free node memory
        _UnlinkNode(pLList, pCurNode);
        --(pLList->nCurNodes);

        _FreeNodeMem(pCurNode, pLList->valType, !pvValOut);
    }

    ReleaseMutex(pLList->hMuAccess);
    return hr;

error_return:
    if(fMutexLocked && !ReleaseMutex(pLList->hMuAccess))
    {
        logerr("%s(): error_return mutex unlock", __FUNCTION__);
    }
    return hr;
}

HRESULT CHL_DsPeekAtLL(_In_ PCHL_LLIST pLList, _In_ int iIndexToPeek, _Inout_ PVOID pvValOut, _In_opt_ BOOL fGetPointerOnly)
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
        pCurNode = pLList->pHead;
    }
    else if(iIndexToPeek == (pLList->nCurNodes - 1))
    {
        hr = S_OK;
        pCurNode = pLList->pTail;
    }
    else
    {
        int itr;

        // Iterate through the list to the specified index
        pCurNode = pLList->pHead;
        for(itr = 0; itr < iIndexToPeek; ++itr)
        {
            pCurNode = pCurNode->pright;
        }

        if(pCurNode != NULL)
        {
            hr = S_OK;
        }

        ASSERT(pCurNode != NULL);
    }

    if(SUCCEEDED(hr) && (pvValOut != NULL))
    {
        _CopyValOut(&pCurNode->chlVal, pLList->valType, pvValOut, fGetPointerOnly);
    }

    ReleaseMutex(pLList->hMuAccess);
    return hr;

error_return:
    if(fMutexLocked && !ReleaseMutex(pLList->hMuAccess))
    {
        logerr("%s(): error_return mutex unlock", __FUNCTION__);
    }
    return hr;
}

HRESULT CHL_DsFindLL(
    _In_ PCHL_LLIST pLList, 
    _In_ PCVOID pvValToFind, 
    _In_ BOOL (*pfnComparer)(PCVOID, PCVOID))
{
    BOOL fMutexLocked = FALSE;

    PLLNODE pCurNode = NULL;
    PVOID pvCurVal = NULL;
    CHL_VALTYPE valType = pLList->valType;

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
        _CopyValOut(&pCurNode->chlVal, valType, &pvCurVal, TRUE);
        if( pfnComparer(pvValToFind, pvCurVal) )
        {
            hr = S_OK;
            break;
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

DllExpImp HRESULT CHL_DsDestroyLL(_In_ PCHL_LLIST pLList)
{
    PLLNODE pCurNode, pNextNode;
    CHL_VALTYPE valType;

    HRESULT hr = S_OK;
    if(!pLList)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = CHL_GnOwnMutex(pLList->hMuAccess);
    if(FAILED(hr))
    {
        goto done;
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
    
done:
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

static void _FreeNodeMem(PLLNODE pnode, CHL_VALTYPE valType, BOOL fFreeValMem)
{
    ASSERT(pnode);
    ASSERT((valType > CHL_VT_START) && (valType < CHL_VT_END));

    // Free memory occupied by the value, if applicable
    // LinkedList doesn't have the facility to indicate that a 
    // heap memory must be freed when the valtype is CHL_VT_POINTER
    // like the hash table has.
    // TODO: Revisit and see if this is a necessary feature.
    if(fFreeValMem)
    {
        _DeleteVal(&pnode->chlVal, valType);
    }

    // Finally, free the node itself
    CHL_MmFree((PVOID*)&pnode);
}
