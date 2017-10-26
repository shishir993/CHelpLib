
#include "InternalDefines.h"
#include "Stack.h"

#define CHL_STK_BOTTOM_IDX      0


// NOTE: 
//  TOS always points to an empty location.
//  This means, push = insert and then increment; pop = decrement and then read.
// 

HRESULT CHL_DsCreateSTK(_Out_ PCHL_STACK pstk, _In_ CHL_VALTYPE valType, _In_opt_ UINT maxSize)
{
    HRESULT hr = S_OK;

    if (IS_INVALID_CHL_VALTYPE(valType))
    {
        hr = E_INVALIDARG;
        goto func_end;
    }

    memset(pstk, 0, sizeof(*pstk));

    pstk->topIndex = CHL_STK_BOTTOM_IDX;
    hr = CHL_DsCreateRA(&pstk->rarray, valType, 0, maxSize);

func_end:
    return hr;
}

HRESULT CHL_DsDestroySTK(_In_ PCHL_STACK pstk)
{
    PCHL_RARRAY pra = &pstk->rarray;
    HRESULT hr = pra->Destroy(pra);
    if (SUCCEEDED(hr))
    {
        memset(pstk, 0, sizeof(*pstk));
    }
    return hr;
}

HRESULT CHL_DsPushSTK(_In_ PCHL_STACK pstk, _In_ PCVOID pVal, _In_opt_ int iBufSize)
{
    PCHL_RARRAY pra = &pstk->rarray;
    HRESULT hr = pra->Write(pra, pstk->topIndex, pVal, iBufSize);
    if (SUCCEEDED(hr))
    {
        ++(pstk->topIndex);
    }
    return hr;
}

HRESULT CHL_DsPopSTK(_In_ PCHL_STACK pstk, _Out_opt_ PVOID pValBuf, _Inout_opt_ PINT piBufSize)
{
    PCHL_RARRAY pra = &pstk->rarray;
    HRESULT hr = S_OK;
    if (pstk->topIndex == CHL_STK_BOTTOM_IDX)
    {
        hr = HRESULT_FROM_WIN32(ERROR_EMPTY);
        goto func_end;
    }

    // Getting a pointer only doesn't make sense for Pop operation because the value on stack
    // is going to be erased after popping.
    hr = pra->Read(pra, pstk->topIndex - 1, pValBuf, piBufSize, FALSE /*fGetPointerOnly*/);
    if (SUCCEEDED(hr))
    {
        --(pstk->topIndex);
#ifdef _DEBUG
        ASSERT(SUCCEEDED(pra->ClearAt(pra, pstk->topIndex)));
#else
        pra->ClearAt(pra, pstk->topIndex);
#endif
    }

    // Reduce size of underlying array to half if it is mostly empty (3/4th or more empty).
    // Idea borrowed from Algorithms 4th ed., by Sedgewick & Wayne.
    if (pstk->topIndex <= (pra->Size(pra) / 4))
    {
#ifdef _DEBUG
        ASSERT(SUCCEEDED(pra->Resize(pra, (pra->Size(pra) / 2))));
#else
        pra->Resize(pra, (pra->Size(pra) / 2));
#endif
    }

func_end:
    return hr;
}

HRESULT CHL_DsTopSTK(_In_ PCHL_STACK pstk, _Out_opt_ PVOID pValBuf, _Inout_opt_ PINT piBufSize,
    _In_ BOOL fGetPointerOnly)
{
    return pstk->Peek(pstk, 0, pValBuf, piBufSize, fGetPointerOnly);
}

HRESULT CHL_DsPeekSTK(_In_ PCHL_STACK pstk, _In_ UINT index, _Out_opt_ PVOID pValBuf, _Inout_opt_ PINT piBufSize,
    _In_ BOOL fGetPointerOnly)
{
    PCHL_RARRAY pra = &pstk->rarray;
    HRESULT hr = S_OK;
    if (pstk->topIndex == CHL_STK_BOTTOM_IDX)
    {
        hr = HRESULT_FROM_WIN32(ERROR_EMPTY);
        goto func_end;
    }

    if (index > (pstk->topIndex - 1))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
        goto func_end;
    }

    hr = pra->Read(pra, (pstk->topIndex - index - 1), pValBuf, piBufSize, fGetPointerOnly);

func_end:
    return hr;
}

UINT CHL_DsSizeSTK(_In_ PCHL_STACK pstk)
{
    return pstk->topIndex;
}
