
// Defines.h
// Contains common #defines, typedefs and data structures
// Shishir Bhat (http://www.shishirbhat.com)
// History
//      09/20/2014 Standardize API experience.
//      01/19/2016 Provide a way to test if a CHL_VAL is occupied or not.
//

#include "CommonInclude.h"
#include "InternalDefines.h"
#include "MemFunctions.h"

#pragma region KeyFunctions

HRESULT _CopyKeyIn(_In_ PCHL_KEY pChlKey, _In_ CHL_KEYTYPE keyType, _In_ PCVOID pvKey, _Inout_opt_ int iKeySize)
{
    HRESULT hr = S_OK;

    ASSERT(pChlKey && (iKeySize > 0));
    ASSERT((keyType > CHL_KT_START) && (keyType < CHL_KT_END));

    pChlKey->iKeySize = iKeySize;
    switch(keyType)
    {
        case CHL_KT_INT32:
        {
            pChlKey->keyDef.iKey = (int)pvKey;
            break;
        }

        case CHL_KT_UINT32:
        {
            pChlKey->keyDef.uiKey = (UINT)pvKey;
            break;
        }

        case CHL_KT_POINTER:
        {
            hr = (pvKey != NULL) ? S_OK : E_INVALIDARG;
            if(SUCCEEDED(hr))
            {
                pChlKey->keyDef.pvKey = pvKey;
            }
            break;
        }

        case CHL_KT_STRING:
        {
            PSTR psz = (PSTR)pvKey;
            hr = (psz != NULL) ? S_OK : E_INVALIDARG;
            if(SUCCEEDED(hr))
            {
                hr = CHL_MmAlloc((PVOID*)&pChlKey->keyDef.pszKey, iKeySize, NULL);
                if(SUCCEEDED(hr))
                {
                    memcpy(pChlKey->keyDef.pszKey, psz, iKeySize);
                }
            }
            break;
        }

        case CHL_KT_WSTRING:
        {
            PWSTR pwsz = (PWSTR)pvKey;
            hr = (pwsz != NULL) ? S_OK : E_INVALIDARG;
            if(SUCCEEDED(hr))
            {
                hr = CHL_MmAlloc((PVOID*)&pChlKey->keyDef.pwszKey, iKeySize, NULL);
                if(SUCCEEDED(hr))
                {
                    memcpy(pChlKey->keyDef.pwszKey, pwsz, iKeySize);
                }
            }
            break;
        }

        default:
        {
            hr = E_NOTIMPL;
            logerr("%s(): Invalid keyType %d", __FUNCTION__, keyType);
            break;
        }
    }
    return hr;
}

HRESULT _CopyKeyOut(_In_ PCHL_KEY pChlKey, _In_ CHL_KEYTYPE keyType, _Inout_ PVOID pvKeyOut, _In_ BOOL fGetPointerOnly)
{
    HRESULT hr = S_OK;

    ASSERT(pChlKey && pvKeyOut);
    ASSERT((keyType > CHL_KT_START) && (keyType < CHL_KT_END));

    switch(keyType)
    {
        case CHL_KT_INT32:
        {
            *((int*)pvKeyOut) = pChlKey->keyDef.iKey;
            break;
        }

        case CHL_KT_UINT32:
        {
            *((PUINT)pvKeyOut) = pChlKey->keyDef.uiKey;
            break;
        }

        case CHL_KT_POINTER:
        {
            *((PVOID*)pvKeyOut) = pChlKey->keyDef.pvKey;
            break;
        }

        case CHL_KT_STRING:
        {
            if(fGetPointerOnly)
            {
                *((char**)pvKeyOut) = pChlKey->keyDef.pszKey;
            }
            else
            {
                memcpy(pvKeyOut, pChlKey->keyDef.pszKey, pChlKey->iKeySize);
            }
            break;
        }

        case CHL_KT_WSTRING:
        {
            if(fGetPointerOnly)
            {
                *((WCHAR**)pvKeyOut) = pChlKey->keyDef.pwszKey;
            }
            else
            {
                memcpy(pvKeyOut, pChlKey->keyDef.pwszKey, pChlKey->iKeySize);
            }
            break;
        }

        default:
        {
            hr = E_NOTIMPL;
            logerr("%s(): Invalid keyType %d", __FUNCTION__, keyType);
            break;
        }
    }
    return hr;
}

BOOL _IsDuplicateKey(_In_ PCHL_KEY pChlLeftKey, _In_ PCVOID pvRightKey, _In_ CHL_KEYTYPE keyType, _In_ int iKeySize)
{
    BOOL fMatch = FALSE;

    ASSERT(pChlLeftKey);
    ASSERT((keyType > CHL_KT_START) && (keyType < CHL_KT_END));

    switch(keyType)
    {
        case CHL_KT_INT32:
        {
            fMatch = (pChlLeftKey->keyDef.iKey == (int)pvRightKey);
            break;
        }

        case CHL_KT_UINT32:
        {
            fMatch = (pChlLeftKey->keyDef.uiKey == (UINT)pvRightKey);
            break;
        }

        case CHL_KT_POINTER:
        {
            fMatch = (pChlLeftKey->keyDef.pvKey == pvRightKey);
            break;
        }

        case CHL_KT_STRING:
        {
            if((iKeySize > 0) && (pvRightKey != NULL))
            {
                fMatch = strncmp(pChlLeftKey->keyDef.pszKey, (PCSTR)pvRightKey, (iKeySize / sizeof(char))) == 0;
            }
            else
            {
                logerr("%s(): Invalid keysize/NULL right-key for string", __FUNCTION__);
            }
            break;
        }

        case CHL_KT_WSTRING:
        {
            if(iKeySize > 0)
            {
                fMatch = wcsncmp(pChlLeftKey->keyDef.pwszKey, (PCWSTR)pvRightKey, (iKeySize / sizeof(WCHAR))) == 0;
            }
            else
            {
                logerr("%s(): Invalid keysize/NULL right-key for wstring", __FUNCTION__);
            }
            break;
        }

        default:
        {
            logerr("%s(): Invalid keyType %d", __FUNCTION__, keyType);
            break;
        }
    }
    return fMatch;
}

void _DeleteKey(_In_ PCHL_KEY pChlKey, _In_ CHL_KEYTYPE keyType)
{
    pChlKey->iKeySize = 0;
    switch(keyType)
    {
        case CHL_KT_STRING:
        {
            CHL_MmFree((PVOID*)&pChlKey->keyDef.pszKey);
            break;
        }

        case CHL_KT_WSTRING:
        {
            CHL_MmFree((PVOID*)&pChlKey->keyDef.pwszKey);
            break;
        }

        default:
        {
            break;
        }
    }
}

HRESULT _GetKeySize(_In_ PVOID pvKey, _In_ CHL_KEYTYPE keyType, _Inout_ PINT piKeySize)
{
    HRESULT hr = S_OK;

    ASSERT(piKeySize);
    ASSERT((keyType > CHL_KT_START) && (keyType < CHL_KT_END));

    switch(keyType)
    {
        case CHL_KT_INT32:
        {
            *piKeySize = sizeof(int);
            break;
        }

        case CHL_KT_UINT32:
        {
            *piKeySize = sizeof(UINT);
            break;
        }

        case CHL_KT_POINTER:
        {
            *piKeySize = sizeof(PVOID);
            break;
        }

        case CHL_KT_STRING:
        {
            *piKeySize = (strlen((PCSTR)pvKey) + 1) * sizeof(char);
            break;
        }

        case CHL_KT_WSTRING:
        {
            *piKeySize = (wcslen((PCWSTR)pvKey) + 1) * sizeof(WCHAR);
            break;
        }

        default:
        {
            hr = E_NOTIMPL;
            logerr("%s(): Invalid keyType %d", __FUNCTION__, keyType);
            break;
        }
    }
    return hr;

}

HRESULT _EnsureSufficientKeyBuf(
    _In_ PCHL_KEY pChlKey, 
    _In_ int iSpecBufSize, 
    _Inout_opt_ PINT piReqBufSize)
{
    HRESULT hr = S_OK;
    
    ASSERT(pChlKey);
    ASSERT(pChlKey->iKeySize > 0);

    hr = (iSpecBufSize >= pChlKey->iKeySize) ? S_OK : HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    if(piReqBufSize != NULL)
    {
        *piReqBufSize = pChlKey->iKeySize;
    }

    return hr;
}

#pragma endregion KeyFunctions

HRESULT _CopyValIn(_In_ PCHL_VAL pChlVal, _In_ CHL_VALTYPE valType, _In_ PCVOID pvVal, _In_opt_ int iValSize)
{
    HRESULT hr = S_OK;

    ASSERT(pChlVal && (iValSize > 0));
    ASSERT((valType > CHL_VT_START) && (valType < CHL_VT_END));

    pChlVal->iValSize = iValSize;
    switch(valType)
    {
        case CHL_VT_INT32:
        {
            pChlVal->valDef.iVal = (int)pvVal;
            break;
        }

        case CHL_VT_UINT32:
        {
            pChlVal->valDef.uiVal = (UINT)pvVal;
            break;
        }

        case CHL_VT_POINTER:
        {
            hr = (pvVal != NULL) ? S_OK : E_INVALIDARG;
            if(SUCCEEDED(hr))
            {
                pChlVal->valDef.pvPtr = pvVal;
            }
            break;
        }

        case CHL_VT_USEROBJECT:
        {
            hr = ((pvVal != NULL) && (iValSize > 0)) ? S_OK : E_INVALIDARG;
            if(SUCCEEDED(hr))
            {
                hr = CHL_MmAlloc((PVOID*)&pChlVal->valDef.pvUserObj, iValSize, NULL);
                if(SUCCEEDED(hr))
                {
                    memcpy(pChlVal->valDef.pvUserObj, pvVal, iValSize);
                }
            }
            break;
        }

        case CHL_VT_STRING:
        {
            PSTR psz = (PSTR)pvVal;
            hr = (psz != NULL) ? S_OK : E_INVALIDARG;
            if(SUCCEEDED(hr))
            {
                hr = CHL_MmAlloc((PVOID*)&pChlVal->valDef.pszVal, iValSize, NULL);
                if(SUCCEEDED(hr))
                {
                    memcpy(pChlVal->valDef.pszVal, psz, iValSize);
                }
            }
            break;
        }

        case CHL_VT_WSTRING:
        {
            PWSTR pwsz = (PWSTR)pvVal;
            hr = (pwsz != NULL) ? S_OK : E_INVALIDARG;
            if(SUCCEEDED(hr))
            {
                hr = CHL_MmAlloc((PVOID*)&pChlVal->valDef.pwszVal, iValSize, NULL);
                if(SUCCEEDED(hr))
                {
                    memcpy(pChlVal->valDef.pwszVal, pwsz, iValSize);
                }
            }
            break;
        }

        default:
        {
            hr = E_NOTIMPL;
            logerr("%s(): Invalid valType %d", __FUNCTION__, valType);
            break;
        }
    }

    if (SUCCEEDED(hr))
    {
        _MarkValOccupied(pChlVal);
    }
    return hr;
}

HRESULT _CopyValOut(_In_ PCHL_VAL pChlVal, _In_ CHL_VALTYPE valType, _Inout_ PVOID pvValOut, _In_ BOOL fGetPointerOnly)
{
    HRESULT hr = S_OK;

    ASSERT(pChlVal && pvValOut);
    ASSERT((valType > CHL_VT_START) && (valType < CHL_VT_END));

    switch(valType)
    {
        case CHL_VT_INT32:
        {
            *((int*)pvValOut) = pChlVal->valDef.iVal;
            break;
        }

        case CHL_VT_UINT32:
        {
            *((PUINT)pvValOut) = pChlVal->valDef.uiVal;
            break;
        }

        case CHL_VT_POINTER:
        {
            *((PVOID*)pvValOut) = pChlVal->valDef.pvPtr;
            break;
        }

        case CHL_VT_USEROBJECT:
        {
            if(fGetPointerOnly)
            {
                *((PVOID*)pvValOut) = pChlVal->valDef.pvUserObj;
            }
            else
            {
                memcpy(pvValOut, pChlVal->valDef.pvUserObj, pChlVal->iValSize);
            }
            break;
        }

        case CHL_VT_STRING:
        {
            if(fGetPointerOnly)
            {
                *((PVOID*)pvValOut) = pChlVal->valDef.pszVal;
            }
            else
            {
                memcpy(pvValOut, pChlVal->valDef.pszVal, pChlVal->iValSize);
            }
            break;
        }

        case CHL_VT_WSTRING:
        {
            if(fGetPointerOnly)
            {
                *((PVOID*)pvValOut) = pChlVal->valDef.pwszVal;
            }
            else
            {
                memcpy(pvValOut, pChlVal->valDef.pwszVal, pChlVal->iValSize);
            }            
            break;
        }

        default:
        {
            hr = E_NOTIMPL;
            logerr("%s(): Invalid valType %d", __FUNCTION__, valType);
            break;
        }
    }
    return hr;
}

BOOL _IsDuplicateVal(_In_ PCHL_VAL pChlLeftVal, _In_ PCVOID pvRightVal, _In_ CHL_VALTYPE valType, _In_ int iValSize)
{
    BOOL fMatch = FALSE;

    ASSERT(pChlLeftVal);
    ASSERT((valType > CHL_VT_START) && (valType < CHL_VT_END));

    switch(valType)
    {
        case CHL_VT_INT32:
        {
            fMatch = (pChlLeftVal->valDef.iVal == (int)pvRightVal);
            break;
        }

        case CHL_VT_UINT32:
        {
            fMatch = (pChlLeftVal->valDef.uiVal == (UINT)pvRightVal);
            break;
        }

        case CHL_VT_POINTER:
        {
            fMatch = (pChlLeftVal->valDef.pvPtr == pvRightVal);
            break;
        }

        case CHL_VT_STRING:
        {
            if((iValSize > 0) && (pvRightVal != NULL))
            {
                size_t cch = (iValSize / sizeof(char)) - 1;
                fMatch = strncmp(pChlLeftVal->valDef.pszVal, (PCSTR)pvRightVal, cch) == 0;   
            }
            else
            {
                logerr("%s(): Invalid keysize/NULL right-key for string", __FUNCTION__);
            }
            break;
        }

        case CHL_VT_WSTRING:
        {
            if((iValSize > 0) && (pvRightVal != NULL))
            {
                size_t cch = (iValSize / sizeof(WCHAR)) - 1;
                fMatch = wcsncmp(pChlLeftVal->valDef.pwszVal, (PCWSTR)pvRightVal, cch) == 0;
            }
            else
            {
                logerr("%s(): Invalid keysize/NULL right-key for wstring", __FUNCTION__);
            }
            break;
        }

        default:
        {
            logerr("%s(): Invalid valType %d", __FUNCTION__, valType);
            break;
        }
    }
    return fMatch;
}

void _DeleteVal(_In_ PCHL_VAL pChlVal, _In_ CHL_VALTYPE valType, _In_opt_ BOOL fFreePointerType)
{
    if (_IsValOccupied(pChlVal))
    {
        switch (valType)
        {
        case CHL_VT_POINTER:
            {
                if (fFreePointerType)
                {
                    CHL_MmFree(&pChlVal->valDef.pvPtr);
                }
                break;
            }

        case CHL_VT_USEROBJECT:
            {
                CHL_MmFree((PVOID*)&pChlVal->valDef.pvUserObj);
                break;
            }

        case CHL_VT_STRING:
            {
                CHL_MmFree((PVOID*)&pChlVal->valDef.pszVal);
                break;
            }

        case CHL_VT_WSTRING:
            {
                CHL_MmFree((PVOID*)&pChlVal->valDef.pwszVal);
                break;
            }
        }

        _MarkValUnoccupied(pChlVal);
    }
}

void _MarkValUnoccupied(_In_ PCHL_VAL pChlVal)
{
    pChlVal->iValSize = pChlVal->magicOccupied = 0;
}

void _MarkValOccupied(_In_ PCHL_VAL pChlVal)
{
    pChlVal->magicOccupied = MAGIC_CHLVAL_OCCUPIED;
}

BOOL _IsValOccupied(_In_ PCHL_VAL pChlVal)
{
    return (pChlVal->magicOccupied == MAGIC_CHLVAL_OCCUPIED);
}

HRESULT _GetValSize(_In_ PVOID pvVal, _In_ CHL_VALTYPE valType, _Inout_ PINT piValSize)
{
    HRESULT hr = S_OK;

    ASSERT(piValSize);
    ASSERT((valType > CHL_VT_START) && (valType < CHL_VT_END));

    switch(valType)
    {
        case CHL_VT_INT32:
        {
            *piValSize = sizeof(int);
            break;
        }

        case CHL_VT_UINT32:
        {
            *piValSize = sizeof(UINT);
            break;
        }

        case CHL_VT_POINTER:
        {
            *piValSize = sizeof(PVOID);
            break;
        }

        case CHL_VT_USEROBJECT:
        {
            // Cannot determine size of user defined structure
            hr = E_FAIL;
            break;
        }

        case CHL_VT_STRING:
        {
            *piValSize = (strlen((PCSTR)pvVal) + 1) * sizeof(char);
            break;
        }

        case CHL_VT_WSTRING:
        {
            *piValSize = (wcslen((PCWSTR)pvVal) + 1) * sizeof(WCHAR);
            break;
        }

        default:
        {
            hr = E_NOTIMPL;
            logerr("%s(): Invalid valType %d", __FUNCTION__, valType);
            break;
        }
    }
    return hr;
}

HRESULT _EnsureSufficientValBuf(
    _In_ PCHL_VAL pChlVal, 
    _In_ int iSpecBufSize, 
    _Inout_opt_ PINT piReqBufSize)
{
    HRESULT hr = S_OK;
    
    ASSERT(pChlVal);
    ASSERT(pChlVal->iValSize > 0);

    hr = (iSpecBufSize >= pChlVal->iValSize) ? S_OK : HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    if(piReqBufSize != NULL)
    {
        *piReqBufSize = pChlVal->iValSize;
    }

    return hr;
}
