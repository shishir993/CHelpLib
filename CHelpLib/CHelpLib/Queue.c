
#include "Queue.h"

HRESULT CHL_DsCreateQ(_Out_ PCHL_QUEUE *ppQueueObj, _In_ CHL_VALTYPE valType, _In_opt_ int nEstimatedItems)
{
    HRESULT hr = S_OK;
    PCHL_QUEUE pq = NULL;
    LL_VALTYPE valTypeLL;

    ASSERT(ppQueueObj && valType >= CHL_VT_STRING && valType <= CHL_VT_PVOID);
    pq = (PCHL_QUEUE)malloc(sizeof(CHL_QUEUE));
    if(pq == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    ZeroMemory(pq, sizeof(CHL_QUEUE));

    // Temp mapping code until all data structures use the common Key and Value types
    switch(valType)
    {
    case CHL_VT_STRING:
    case CHL_VT_PVOID:
        valTypeLL = LL_VAL_PTR;
        break;

    default:
        hr = E_INVALIDARG;       
        goto done;
    }

    // Create the linked list
    hr = CHL_DsCreateLL(&pq->pList, valTypeLL, nEstimatedItems);
    if(FAILED(hr))
    {
        goto done;
    }

    pq->Destroy = CHL_DsDestroyQ;
    pq->Insert = CHL_DsInsertQ;
    pq->Delete = CHL_DsDeleteQ;
    pq->Find = CHL_DsFindQ;
    pq->Peek = CHL_DsPeekQ;

done:
    if(SUCCEEDED(hr))
    {
        *ppQueueObj = pq;
    }
    else
    {
        if(pq != NULL)
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
    if(pQueueObj->pList)
    {
        hr = CHL_DsDestroyLL(pQueueObj->pList);
        if(FAILED(hr))
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

HRESULT CHL_DsInsertQ(_In_ PCHL_QUEUE pQueueObj, _In_ PVOID pvValue, _In_ int nValSize)
{
    HRESULT hr = S_OK;
    ASSERT(pQueueObj && pQueueObj->pList);

    hr = CHL_DsInsertLL(pQueueObj->pList, pvValue, nValSize);
    if(SUCCEEDED(hr))
    {
        ++(pQueueObj->nCurItems);
    }
    return hr;
}

HRESULT CHL_DsDeleteQ(_In_ PCHL_QUEUE pQueueObj, _Out_ PVOID *ppvValue)
{
    HRESULT hr = S_OK;

    ASSERT(pQueueObj && pQueueObj->pList);
    ASSERT(ppvValue);

    *ppvValue = NULL;
    if(pQueueObj->nCurItems <= 0)
    {
        hr = E_NOT_SET;
    }
    else
    {
        // Linked list always inserts at the tail, so the first item is the one to be deleted
        hr = CHL_DsRemoveAtLL(pQueueObj->pList, 0, ppvValue);
        if(SUCCEEDED(hr))
        {
            --(pQueueObj->nCurItems);
        }
    }
    return hr;
}

HRESULT CHL_DsPeekQ(_In_ PCHL_QUEUE pQueueObj, _Out_ PVOID *ppvValue)
{
    HRESULT hr = S_OK;
    ASSERT(pQueueObj && pQueueObj->pList);
    ASSERT(ppvValue);

    // Linked list always inserts at the tail, so the first item is the one to be peek'd
    if(pQueueObj->nCurItems > 0)
    {
        hr = CHL_DsPeekAtLL(pQueueObj->pList, 0, ppvValue);
    }
    else
    {
        hr = E_NOT_SET;
        *ppvValue = NULL;
    }
    return hr;
}

HRESULT CHL_DsFindQ(_In_ PCHL_QUEUE pQueueObj, _In_ PVOID pvValue, _In_opt_ BOOL (*pfnComparer)(void*, void*))
{
    HRESULT hr = S_OK;
    ASSERT(pQueueObj && pQueueObj->pList);
    ASSERT(pvValue);

    if(pQueueObj->nCurItems > 0)
    {
        hr = CHL_DsFindLL(pQueueObj->pList, pvValue, pfnComparer, NULL);
    }
    else
    {
        hr = E_NOT_SET;
    }
    return hr;
}
