
#include "InternalDefines.h"
#include "RArray.h"

#define RARRAY_MIN_SIZE             1
#define RARRAY_MAX_SIZE_NOLIMIT     0

static __inline UINT _CalcValArrayBytesForSize(_In_ UINT uiSize);
static __inline UINT _CalcNewSizeGrow(_In_ PCHL_RARRAY pra);
static __inline UINT _CalcNewSizeShrink(_In_ PCHL_RARRAY pra);


HRESULT CHL_DsCreateRA(_Out_ PCHL_RARRAY pra, _In_ CHL_VALTYPE valType, _In_opt_ UINT initSize, _In_opt_ UINT maxSize)
{
    HRESULT hr = S_OK;

    // validate parameters
    if ((pra == NULL) || IS_INVALID_CHL_VALTYPE(valType))
    {
        hr = E_INVALIDARG;
        goto func_end;
    }

    memset(pra, 0, sizeof(*pra));

    pra->curSize = max(initSize, RARRAY_MIN_SIZE);
    pra->maxSize = maxSize;
    pra->vt = valType;

    pra->pValArray = (CHL_VAL*)malloc(_CalcValArrayBytesForSize(pra->curSize));
    if (pra->pValArray == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto func_end;
    }

    pra->Create = CHL_DsCreateRA;
    pra->Destroy = CHL_DsDestroyRA;
    pra->Read = CHL_DsReadRA;
    pra->Write = CHL_DsWriteRA;
    pra->ClearAt = CHL_DsClearAtRA;
    pra->Resize = CHL_DsResizeRA;
    pra->Size = CHL_DsSizeRA;
    pra->MaxSize = CHL_DsMaxSizeRA;

func_end:
    return hr;
}

HRESULT CHL_DsDestroyRA(_In_ PCHL_RARRAY pra)
{
    if (pra->pValArray != NULL)
    {
        free(pra->pValArray);
    }
    memset(pra, 0, sizeof(*pra));
    return S_OK;
}

HRESULT CHL_DsReadRA(_In_ PCHL_RARRAY pra, _In_ UINT index, _Out_opt_ PVOID pValBuf,
    _Inout_opt_ PINT piBufSize, _In_ BOOL fGetPointerOnly)
{
    ASSERT(pra->pValArray != NULL);
    ASSERT(pra->curSize >= RARRAY_MIN_SIZE);
    ASSERT((pra->vt > CHL_VT_START) && (pra->vt < CHL_VT_END));

    HRESULT hr = S_OK;
    PCHL_VAL pValToRead = NULL;

    if (index >= pra->curSize)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
        goto func_end;
    }

	pValToRead = &(pra->pValArray[index]);
	if (_IsValOccupied(pValToRead) == FALSE)
	{
		hr = E_NOT_SET;
		goto func_end;
	}

	if (pValBuf != NULL)
	{
		hr = _CopyValOut(pValToRead, pra->vt, pValBuf, piBufSize, fGetPointerOnly);
	}

func_end:
    return hr;
}

HRESULT CHL_DsWriteRA(_In_ PCHL_RARRAY pra, _In_ UINT index, _In_ PCVOID pVal, _In_opt_ int iBufSize)
{
    ASSERT(pra->pValArray != NULL);
    ASSERT(pra->curSize >= RARRAY_MIN_SIZE);
    ASSERT((pra->vt > CHL_VT_START) && (pra->vt < CHL_VT_END));

    HRESULT hr = S_OK;
    
    if (index >= pra->curSize)
    {
        UINT newSize = _CalcNewSizeGrow(pra);
        hr = pra->Resize(pra, newSize);
    }

    if (SUCCEEDED(hr) && (index >= pra->curSize))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }
    
    if (FAILED(hr))
    {
        goto func_end;
    }

    ASSERT(index < pra->curSize);

    hr = _CopyValIn(&pra->pValArray[index], pra->vt, pVal, iBufSize);

func_end:
    return hr;
}

HRESULT CHL_DsClearAtRA(_In_ PCHL_RARRAY pra, _In_ UINT index)
{
    ASSERT(pra->pValArray != NULL);

    HRESULT hr = S_OK;

    if (index >= pra->curSize)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
        goto func_end;
    }

    PCHL_VAL pChlVal = &pra->pValArray[index];
    _DeleteVal(pChlVal, pra->vt, FALSE);
    // TODO: Revisit and see if fValIsInHeap is a necessary feature

func_end:
    return hr;
}

HRESULT CHL_DsResizeRA(_In_ PCHL_RARRAY pra, _In_ UINT newSize)
{
    ASSERT(pra->pValArray != NULL);
    ASSERT((newSize >= RARRAY_MIN_SIZE) &&
        ((pra->maxSize == RARRAY_MAX_SIZE_NOLIMIT) || (newSize <= pra->maxSize)));

    HRESULT hr = S_OK;
    PVOID pvNew = NULL;

    if ((newSize < RARRAY_MIN_SIZE) ||
        ((pra->maxSize != RARRAY_MAX_SIZE_NOLIMIT) && (newSize > pra->maxSize)))
    {
        hr = E_INVALIDARG;
        goto func_end;
    }

    UINT curSize = CHL_DsSizeRA(pra);

    if (curSize == newSize)
    {
        goto func_end;
    }

    if (curSize > newSize)
    {
        // Array is shrinking, see if there are any stored values to be cleared
        for (UINT idx = newSize; idx < curSize; ++idx)
        {
#ifdef _DEBUG
            ASSERT(SUCCEEDED(CHL_DsClearAtRA(pra, idx)));
#else
            CHL_DsClearAtRA(pra, idx);
#endif
        }
    }

    pvNew = realloc(pra->pValArray, _CalcValArrayBytesForSize(newSize));
    if (pvNew != NULL)
    {
        pra->pValArray = pvNew;
        pra->curSize = newSize;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

func_end:
    return hr;
}

UINT CHL_DsSizeRA(_In_ PCHL_RARRAY pra)
{
    return pra->curSize;
}

UINT CHL_DsMaxSizeRA(_In_ PCHL_RARRAY pra)
{
    return pra->maxSize;
}

__inline UINT _CalcValArrayBytesForSize(_In_ UINT uiSize)
{
    ASSERT(uiSize > 0);
    return (uiSize * sizeof(CHL_VAL));
}

__inline UINT _CalcNewSizeGrow(_In_ PCHL_RARRAY pra)
{
    return (pra->maxSize > 0) ? min(pra->curSize * 2, pra->maxSize) : (pra->curSize * 2);
}

__inline UINT _CalcNewSizeShrink(_In_ PCHL_RARRAY pra)
{
    return max(pra->curSize / 2, RARRAY_MIN_SIZE);
}
