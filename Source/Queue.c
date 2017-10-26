
#include "InternalDefines.h"
#include "Queue.h"

HRESULT CHL_DsCreateQ
(
    _Out_ PCHL_QUEUE *ppQueueObj,
    _In_ CHL_VALTYPE valType,
    _In_opt_ int nEstimatedItems
)
{
    PCHL_QUEUE pq;
    HRESULT hr = CHL_MmAlloc(&pq, sizeof(*pq), NULL);
    if (FAILED(hr))
    {
        goto done;
    }

    // Create the linked list
    hr = CHL_DsCreateLL(&pq->pList, valType, nEstimatedItems);
    if (FAILED(hr))
    {
        goto done;
    }

    pq->Create = CHL_DsCreateQ;
    pq->Destroy = CHL_DsDestroyQ;
    pq->Insert = CHL_DsInsertQ;
    pq->Delete = CHL_DsDeleteQ;
    pq->Find = CHL_DsFindQ;
    pq->Peek = CHL_DsPeekQ;

done:
    if (SUCCEEDED(hr))
    {
        *ppQueueObj = pq;
    }
    else
    {
        if (pq != NULL)
        {
            free(pq);
        }
        *ppQueueObj = NULL;
    }
    return hr;
}

HRESULT CHL_DsDestroyQ(_In_ PCHL_QUEUE pQueueObj)
{
    HRESULT hr = S_OK;

    ASSERT(pQueueObj);
    if (pQueueObj->pList)
    {
        hr = CHL_DsDestroyLL(pQueueObj->pList);
        if (FAILED(hr))
        {
            logwarn("Failed to destroy the list(queue).");
        }
        free(pQueueObj);
    }
    else
    {
        hr = E_NOT_VALID_STATE;
    }
    return hr;
}

HRESULT CHL_DsInsertQ
(
    _In_ PCHL_QUEUE pQueueObj,
    _In_ PCVOID pvValue,
    _In_ int nValSize
)
{
    HRESULT hr = S_OK;
    ASSERT(pQueueObj && pQueueObj->pList);

    hr = CHL_DsInsertLL(pQueueObj->pList, pvValue, nValSize);
    if (SUCCEEDED(hr))
    {
        ++(pQueueObj->nCurItems);
    }
    return hr;
}

HRESULT CHL_DsDeleteQ
(
    _In_ PCHL_QUEUE pQueueObj,
    _Inout_opt_ PVOID pvValOut,
    _Inout_opt_ PINT piValBufSize,
    _In_opt_ BOOL fGetPointerOnly
)
{
    HRESULT hr = S_OK;

    ASSERT(pQueueObj && pQueueObj->pList);
    ASSERT(pvValOut);

    if (pQueueObj->nCurItems <= 0)
    {
        hr = E_NOT_SET;
    }
    else
    {
        if (pvValOut) // TODO: BUG return correct result even if value out isn't specified
        {
            // Linked list always inserts at the tail, so the first item is the one to be deleted
            hr = CHL_DsRemoveAtLL(pQueueObj->pList, 0, pvValOut, piValBufSize, fGetPointerOnly);
            if (SUCCEEDED(hr))
            {
                --(pQueueObj->nCurItems);
            }
        }
    }
    return hr;
}

HRESULT CHL_DsPeekQ
(
    _In_ PCHL_QUEUE pQueueObj,
    _Inout_opt_ PVOID pvValOut,
    _Inout_opt_ PINT piValBufSize,
    _In_opt_ BOOL fGetPointerOnly
)
{
    HRESULT hr = S_OK;
    ASSERT(pQueueObj && pQueueObj->pList);
    ASSERT(pvValOut);

    // Linked list always inserts at the tail, so the first item is the one to be peek'd
    if (pQueueObj->nCurItems > 0)
    {
        hr = CHL_DsPeekAtLL(pQueueObj->pList, 0, pvValOut, piValBufSize, fGetPointerOnly);
    }
    else
    {
        hr = E_NOT_SET;
        pvValOut = NULL;
    }
    return hr;
}

HRESULT CHL_DsFindQ
(
    _In_ PCHL_QUEUE pQueueObj,
    _In_ PCVOID pvValue,
    _In_ CHL_CompareFn pfnComparer,
    _In_opt_ PVOID pvValOut,
    _In_opt_ PINT piValBufSize,
    _In_opt_ BOOL fGetPointerOnly
)
{
    HRESULT hr = S_OK;
    ASSERT(pQueueObj && pQueueObj->pList);
    ASSERT(pvValue);

    if (pQueueObj->nCurItems > 0)
    {
        hr = CHL_DsFindLL(pQueueObj->pList, pvValue, pfnComparer, pvValOut, piValBufSize, fGetPointerOnly);
    }
    else
    {
        hr = E_NOT_SET;
    }
    return hr;
}
