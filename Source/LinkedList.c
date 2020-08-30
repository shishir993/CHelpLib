// LinkedList.cpp
// Contains functions that implement a linked list data structure
// Shishir Bhat (http://www.shishirbhat.com)
// History
//      04/05/14 Initial version
//      04/10/14 Changed to insert at end logic
//      09/12/14 Naming convention modifications
//

#include "InternalDefines.h"
#include "General.h"
#include "MemFunctions.h"
#include "LinkedList.h"

// Local functions
static void _InsertNode(PCHL_LLIST pLList, PLLNODE pNodeToInsert);
static void _UnlinkNode(PCHL_LLIST pLList, PLLNODE pNodeToRemove);
static void _FreeNodeMem(PLLNODE pnode, CHL_VALTYPE valType, BOOL fFreeValMem);

HRESULT CHL_DsCreateLL(_Out_ PCHL_LLIST *ppLList, _In_ CHL_VALTYPE valType, _In_opt_ int nEstEntries)
{
    PCHL_LLIST pListLocal = NULL;

    HRESULT hr = S_OK;

    // validate parameters
    if (!((valType > CHL_VT_START) && (valType < CHL_VT_END)))
    {
        hr = E_INVALIDARG;
        goto error_return;
    }

    hr = CHL_MmAlloc((PVOID*)&pListLocal, sizeof(CHL_LLIST), NULL);
    if (FAILED(hr))
    {
        // pListLocal will be NULL on error
        goto error_return;
    }

    // Populate members
    pListLocal->valType = valType;
    pListLocal->nMaxNodes = nEstEntries;

    pListLocal->Insert = CHL_DsInsertLL;
    pListLocal->Remove = CHL_DsRemoveLL;
    pListLocal->RemoveAt = CHL_DsRemoveAtLL;
    pListLocal->RemoveAtItr = CHL_DsRemoveAtItrLL;
    pListLocal->Peek = CHL_DsPeekAtLL;
    pListLocal->Find = CHL_DsFindLL;
    pListLocal->FindItr = CHL_DsFindItrLL;
    pListLocal->Destroy = CHL_DsDestroyLL;
    pListLocal->IsEmpty = CHL_DsIsEmptyLL;
    pListLocal->InitIterator = CHL_DsInitIteratorLL;

    *ppLList = pListLocal;
    return hr;

error_return:
    if (pListLocal)
    {
        CHL_MmFree((PVOID*)&pListLocal);
    }
    return hr;
}

HRESULT CHL_DsInsertLL(_In_ PCHL_LLIST pLList, _In_ PCVOID pvVal, _In_opt_ int iValSize)
{
    PLLNODE pNewNode = NULL;
    HRESULT hr = S_OK;

    // Size parameter validation
    if (iValSize <= 0 && FAILED(_GetValSize(pvVal, pLList->valType, &iValSize)))
    {
        logerr("%s(): Valsize unspecified or unable to determine.", __FUNCTION__);
        hr = E_INVALIDARG;
        goto error_return;
    }

    // Create new node
    hr = CHL_MmAlloc((PVOID*)&pNewNode, sizeof(LLNODE), NULL);
    if (FAILED(hr))
    {
        goto error_return;
    }

    hr = _CopyValIn(&pNewNode->chlVal, pLList->valType, pvVal, iValSize);
    if (FAILED(hr))
    {
        goto error_return;
    }

    _InsertNode(pLList, pNewNode);
    ++(pLList->nCurNodes);

    return hr;

error_return:
    if (pNewNode)
    {
        CHL_MmFree((PVOID*)&pNewNode);
    }
    return hr;
}

HRESULT CHL_DsRemoveLL
(
    _In_ PCHL_LLIST pLList,
    _In_ PCVOID pvValToFind,
    _In_ BOOL fStopOnFirstFind,
    _In_ CHL_CompareFn pfnComparer
)
{
    PLLNODE pCurNode = NULL;
    PVOID pvCurVal = NULL;
    CHL_VALTYPE valType = pLList->valType;

    HRESULT hr = S_OK;

    // Iterate through the list to find
    pCurNode = pLList->pHead;
    hr = E_NOT_SET;
    while (pCurNode)
    {
        _CopyValOut(&pCurNode->chlVal, valType, &pvCurVal, NULL, TRUE);
        if (pfnComparer(pvValToFind, pvCurVal) == 0)
        {
            hr = S_OK;

            _UnlinkNode(pLList, pCurNode);
            _FreeNodeMem(pCurNode, valType, TRUE);

            --(pLList->nCurNodes);

            if (fStopOnFirstFind)
            {
                break;
            }
        }

        pCurNode = pCurNode->pright;
    }

    return hr;
}

HRESULT CHL_DsRemoveAtLL(
    _In_ PCHL_LLIST pLList,
    _In_ int iIndexToRemove,
    _Inout_opt_ PVOID pvValOut,
    _Inout_opt_ PINT piValBufSize,
    _In_opt_ BOOL fGetPointerOnly)
{
    int itr;
    PLLNODE pCurNode;
    HRESULT hr = S_OK;

    if (iIndexToRemove < 0)
    {
        hr = E_INVALIDARG;
        goto fend;
    }

    if (iIndexToRemove >= pLList->nCurNodes)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
        goto fend;
    }

    // Loop until we have address of node to remove
    pCurNode = pLList->pHead;
    for (itr = 1; itr <= iIndexToRemove; ++itr)
    {
        ASSERT(pCurNode);
        pCurNode = pCurNode->pright;
    }

    ASSERT(pCurNode);

    if (pvValOut)
    {
        hr = _CopyValOut(&pCurNode->chlVal, pLList->valType, pvValOut, piValBufSize, fGetPointerOnly);
    }

    if (SUCCEEDED(hr))
    {
        // Unlink and only then free node memory
        _UnlinkNode(pLList, pCurNode);
        --(pLList->nCurNodes);

        _FreeNodeMem(pCurNode, pLList->valType, (!pvValOut || !fGetPointerOnly));
    }

fend:
    return hr;
}

DllExpImp HRESULT CHL_DsRemoveAtItrLL(_Inout_ CHL_ITERATOR_LL *pItr)
{
    HRESULT hr = (pItr->pCur != NULL) ? S_OK : E_NOT_SET;
    if (SUCCEEDED(hr))
    {
        // Unlink and only then free node memory
        PLLNODE pNextNode = pItr->pCur->pright;
        _UnlinkNode(pItr->pMyList, pItr->pCur);
        --(pItr->pMyList->nCurNodes);

        _FreeNodeMem(pItr->pCur, pItr->pMyList->valType, TRUE /*fFreeValMem*/);
        pItr->pCur = pNextNode;
    }
    return hr;
}

HRESULT CHL_DsPeekAtLL
(
    _In_ PCHL_LLIST pLList,
    _In_ int iIndexToPeek,
    _Inout_opt_ PVOID pvValOut,
    _Inout_opt_ PINT piValBufSize,
    _In_opt_ BOOL fGetPointerOnly
)
{
    PLLNODE pCurNode = NULL;

    HRESULT hr = S_OK;
    if (iIndexToPeek < 0 || iIndexToPeek >= pLList->nCurNodes)
    {
        hr = E_INVALIDARG;
        goto fend;
    }

    hr = E_NOT_SET; // Start with this, set to S_OK if node is found
    if (iIndexToPeek == 0)
    {
        hr = S_OK;
        pCurNode = pLList->pHead;
    }
    else if (iIndexToPeek == (pLList->nCurNodes - 1))
    {
        hr = S_OK;
        pCurNode = pLList->pTail;
    }
    else
    {
        int itr;

        // Iterate through the list to the specified index
        pCurNode = pLList->pHead;
        for (itr = 0; itr < iIndexToPeek; ++itr)
        {
            pCurNode = pCurNode->pright;
        }

        if (pCurNode != NULL)
        {
            hr = S_OK;
        }

        ASSERT(pCurNode != NULL);
    }

    if (SUCCEEDED(hr) && (pvValOut != NULL))
    {
        _CopyValOut(&pCurNode->chlVal, pLList->valType, pvValOut, piValBufSize, fGetPointerOnly);
    }

fend:
    return hr;
}

HRESULT CHL_DsFindLL
(
    _In_ PCHL_LLIST pLList,
    _In_ PCVOID pvValToFind,
    _In_ CHL_CompareFn pfnComparer,
    _Inout_opt_ PVOID pvValOut,
    _Inout_opt_ PINT piValBufSize,
    _In_opt_ BOOL fGetPointerOnly
)
{
    PLLNODE pCurNode = NULL;
    PVOID pvCurVal = NULL;
    CHL_VALTYPE valType = pLList->valType;

    HRESULT hr = S_OK;

    // Iterate through the list to find
    pCurNode = pLList->pHead;
    hr = E_NOT_SET;
    while (pCurNode)
    {
        _CopyValOut(&pCurNode->chlVal, valType, &pvCurVal, NULL, TRUE);
        if (pfnComparer(pvValToFind, pvCurVal) == 0)
        {
            hr = S_OK;
            break;
        }
        pCurNode = pCurNode->pright;
    }

    if (SUCCEEDED(hr) && (pvValOut != NULL))
    {
        ASSERT(pCurNode != NULL);
        _CopyValOut(&pCurNode->chlVal, valType, pvValOut, piValBufSize, fGetPointerOnly);
    }

    return hr;
}

HRESULT CHL_DsFindItrLL
(
    _In_ PCHL_LLIST pLList,
    _In_ PCVOID pvValToFind,
    _In_opt_ CHL_CompareFn pfnComparer,
    _Out_ CHL_ITERATOR_LL* pItr
)
{
    PLLNODE pCurNode = NULL;
    PVOID pvCurVal = NULL;
    CHL_VALTYPE valType = pLList->valType;

    HRESULT hr = S_OK;

    if (!pfnComparer)
    {
        pfnComparer = CHL_FindCompareFn(pLList->valType);
    }

    if (!pfnComparer)
    {
        return E_INVALIDARG;
    }

    // Iterate through the list to find
    pCurNode = pLList->pHead;
    hr = E_NOT_SET;
    while (pCurNode)
    {
        _CopyValOut(&pCurNode->chlVal, valType, &pvCurVal, NULL, TRUE);
        if (pfnComparer(pvValToFind, pvCurVal) == 0)
        {
            hr = CHL_DsInitIteratorLL(pLList, pItr);
            if (SUCCEEDED(hr))
            {
                pItr->pCur = pCurNode;
            }
            break;
        }
        pCurNode = pCurNode->pright;
    }
    return hr;
}

HRESULT CHL_DsDestroyLL(_In_ PCHL_LLIST pLList)
{
    PLLNODE pCurNode, pNextNode;
    CHL_VALTYPE valType;

    HRESULT hr = S_OK;

    valType = pLList->valType;

    // Iterate through the list and delete nodes
    pCurNode = pLList->pHead;
    while (pCurNode)
    {
        pNextNode = pCurNode->pright;

        _FreeNodeMem(pCurNode, valType, TRUE);

        pCurNode = pNextNode;
    }
    CHL_MmFree((PVOID*)&pLList);

    return hr;
}

DllExpImp BOOL CHL_DsIsEmptyLL(_In_ PCHL_LLIST pLList)
{
    return (pLList->nCurNodes == 0);
}

DllExpImp HRESULT CHL_DsInitIteratorLL(_In_ PCHL_LLIST pList, _Out_ CHL_ITERATOR_LL *pItr)
{
    ASSERT(pList);
    pItr->pCur = pList->pHead;
    pItr->pMyList = pList;
    pItr->GetCurrent = CHL_DsGetCurrentLL;
    pItr->MoveNext = CHL_DsMoveNextLL;
    return S_OK;
}

DllExpImp HRESULT CHL_DsMoveNextLL(_Inout_ CHL_ITERATOR_LL *pItr)
{
    ASSERT(pItr && pItr->pMyList);
    if (pItr->pCur != NULL)
    {
        pItr->pCur = pItr->pCur->pright;
    }
    return (pItr->pCur != NULL) ? S_OK : E_NOT_SET;
}

DllExpImp HRESULT CHL_DsGetCurrentLL
(
    _In_ CHL_ITERATOR_LL *pItr,
    _Inout_opt_ PVOID pvVal,
    _Inout_opt_ PINT piValSize,
    _In_opt_ BOOL fGetPointerOnly
)
{
    ASSERT(pItr && pItr->pMyList);

    if (pItr->pCur == NULL)
    {
        return E_NOT_SET;
    }

    return _CopyValOut(&pItr->pCur->chlVal, pItr->pMyList->valType, pvVal, piValSize, fGetPointerOnly);
}

static void _InsertNode(PCHL_LLIST pLList, PLLNODE pNodeToInsert)
{
    ASSERT(pLList);
    ASSERT(pNodeToInsert);

    // Linked list empty?
    if (pLList->pHead == NULL)
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
    if (pNodeToRemove->pleft == NULL)
    {
        // Removing the first node, so this should be true
        ASSERT(pLList->pHead == pNodeToRemove);

        pLList->pHead = pNodeToRemove->pright;
        if (pNodeToRemove->pright)
        {
            pNodeToRemove->pright->pleft = pNodeToRemove->pleft;
        }

        // Could be first and last node as well!
        if (pNodeToRemove == pLList->pTail)
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
        if (pNodeToRemove == pLList->pTail)
        {
            pLList->pTail = pNodeToRemove->pleft;
        }

        if (pNodeToRemove->pleft)
        {
            pNodeToRemove->pleft->pright = pNodeToRemove->pright;
        }

        if (pNodeToRemove->pright)
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
    if (fFreeValMem)
    {
        _DeleteVal(&pnode->chlVal, valType, FALSE);
    }

    // Finally, free the node itself
    CHL_MmFree((PVOID*)&pnode);
}
