
#include "Queue.h"

HRESULT CHL_QueueCreate(_Out_ PCHL_QUEUE *ppQueueObj, _In_ CHL_VALTYPE valType, _In_opt_ int nEstimatedItems)
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
    if(!fChlDsCreateLL(&pq->pList, valTypeLL, nEstimatedItems))
    {
        // TODO: Update this when other functions are changed to use HRESULT
        hr = E_FAIL;
        goto done;
    }

    pq->Destroy = CHL_QueueDestroy;
    pq->Insert = CHL_QueueInsert;
    pq->Delete = CHL_QueueDelete;
    pq->Find = CHL_QueueFind;
    pq->Peek = CHL_QueuePeek;

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

HRESULT CHL_QueueDestroy(_In_ PCHL_QUEUE pQueueObj)
{
    HRESULT hr = S_OK;

    ASSERT(pQueueObj);
    if(pQueueObj->pList)
    {
        if(!fChlDsDestroyLL(pQueueObj->pList))
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

HRESULT CHL_QueueInsert(_In_ PCHL_QUEUE pQueueObj, _In_ PVOID pvValue, _In_ int nValSize)
{
    HRESULT hr = S_OK;

    ASSERT(pQueueObj && pQueueObj->pList);
    if(!fChlDsInsertLL(pQueueObj->pList, pvValue, nValSize))
    {
        hr = E_FAIL;
    }
    else
    {
        ++(pQueueObj->nCurItems);
    }
    return hr;
}

HRESULT CHL_QueueDelete(_In_ PCHL_QUEUE pQueueObj, _Out_ PVOID *ppvValue)
{
    HRESULT hr = S_OK;

    ASSERT(pQueueObj && pQueueObj->pList);
    ASSERT(ppvValue);
    if(pQueueObj->nCurItems <= 0)
    {
        hr = E_NOT_SET;
        *ppvValue = NULL;
    }
    else
    {
        // Linked list always inserts at the tail, so the first item is the one to be deleted
        if(!fChlDsRemoveAtLL(pQueueObj->pList, 0, ppvValue))
        {
            hr = E_FAIL;
            *ppvValue = NULL;
        }
        else
        {
            --(pQueueObj->nCurItems);
        }
    }
    return hr;
}

HRESULT CHL_QueuePeek(_In_ PCHL_QUEUE pQueueObj, _Out_ PVOID *ppvValue)
{
    HRESULT hr = S_OK;

    ASSERT(pQueueObj && pQueueObj->pList);
    ASSERT(ppvValue);
    // Linked list always inserts at the tail, so the first item is the one to be peek'd
    if(pQueueObj->nCurItems <= 0 || !fChlDsPeekAtLL(pQueueObj->pList, 0, ppvValue))
    {
        hr = E_FAIL;
        *ppvValue = NULL;
    }
    return hr;
}

HRESULT CHL_QueueFind(_In_ PCHL_QUEUE pQueueObj, _In_ PVOID pvValue, _In_opt_ BOOL (*pfnComparer)(void*, void*))
{
    HRESULT hr = S_OK;

    ASSERT(pQueueObj && pQueueObj->pList);
    ASSERT(pvValue);
    if(pQueueObj->nCurItems <= 0 || !fChlDsFindLL(pQueueObj->pList, pvValue, pfnComparer, NULL))
    {
        hr = E_NOT_SET;
    }
    return hr;
}
