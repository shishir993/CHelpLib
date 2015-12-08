
#include "InternalDefines.h"
#include "RArray.h"

#define RARRAY_MIN_SIZE             2
#define RARRAY_MAX_SIZE_NOLIMIT     0

static __inline UINT _CalcValArrayBytesForSize(_In_ UINT uiSize);
static __inline UINT _CalcNewSizeGrow(_In_ PCHL_RARRAY pra);
static __inline UINT _CalcNewSizeShrink(_In_ PCHL_RARRAY pra);


HRESULT CHL_DsCreateRA(_Out_ PCHL_RARRAY pra, _In_ CHL_VALTYPE valType, _In_opt_ UINT initSize, _In_opt_ UINT maxSize)
{
    HRESULT hr = S_OK;

    // validate parameters
    if ((pra == NULL) ||
        (valType < CHL_VT_START) || (valType > CHL_VT_END) ||
        ((maxSize > 0) && (maxSize < RARRAY_MIN_SIZE)))
    {
        hr = E_INVALIDARG;
        goto error_return;
    }

    memset(pra, 0, sizeof(*pra));

    pra->curSize = max(initSize, RARRAY_MIN_SIZE);
    pra->maxSize = maxSize;
    pra->vt = valType;

    pra->pValArray = (CHL_VAL*)malloc(_CalcValArrayBytesForSize(pra->curSize));
    if (pra->pValArray == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error_return;
    }

    pra->Create = CHL_DsCreateRA;
    pra->Destroy = CHL_DsDestroyRA;
    pra->Read = CHL_DsReadRA;
    pra->Write = CHL_DsWriteRA;
    pra->Resize = CHL_DsResizeRA;
    pra->Size = CHL_DsSizeRA;
    pra->MaxSize = CHL_DsMaxSizeRA;

    return S_OK;

error_return:
    memset(pra, 0, sizeof(*pra));
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

    if (pValBuf != NULL)
    {
        pValToRead = &(pra->pValArray[index]);
        if (!fGetPointerOnly)
        {
            hr = _EnsureSufficientValBuf(pValToRead, (piBufSize != NULL) ? *piBufSize : sizeof(pValBuf), piBufSize);
            if (FAILED(hr))
            {
                goto func_end;
            }
        }

        hr = _CopyValOut(pValToRead, pra->vt, pValBuf, fGetPointerOnly);
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

    if (pra->curSize == newSize)
    {
        goto func_end;
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
