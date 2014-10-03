
// Defines.h
// Contains common #defines, typedefs and data structures
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      09/20/14 Standardize API experience
//

#include "CommonInclude.h"
#include "Defines.h"
#include "MemFunctions.h"

HRESULT _CopyKeyIn(_In_ CHL_key *pChlKey, _In_ CHL_KEYTYPE keyType, _In_ PCVOID pvKey, _Inout_opt_ int iKeySize)
{
    HRESULT hr = S_OK;

    ASSERT(pChlKey && (iKeySize > 0));
    ASSERT((keyType > CHL_KT_START) && (keyType < CHL_KT_END));

    switch(keyType)
    {
        case CHL_KT_INT32:
        {
            pChlKey->iKey = (int)pvKey;
            break;
        }

        case CHL_KT_UINT32:
        {
            pChlKey->uiKey = (UINT)pvKey;
            break;
        }

        case CHL_KT_POINTER:
        {
            hr = (pvKey != NULL) ? S_OK : E_INVALIDARG;
            if(SUCCEEDED(hr))
            {
                pChlKey->pvKey = pvKey;
            }
            break;
        }

        case CHL_KT_STRING:
        {
            PSTR psz = (PSTR)pvKey;
            hr = (psz != NULL) ? S_OK : E_INVALIDARG;
            if(SUCCEEDED(hr))
            {
                hr = CHL_MmAlloc((PVOID*)&pChlKey->pszKey, iKeySize, NULL);
                if(SUCCEEDED(hr))
                {
                    memcpy(pChlKey->pszKey, psz, iKeySize);
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
                hr = CHL_MmAlloc((PVOID*)&pChlKey->pwszKey, iKeySize, NULL);
                if(SUCCEEDED(hr))
                {
                    memcpy(pChlKey->pwszKey, pwsz, iKeySize);
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

HRESULT _CopyKeyOut(_In_ CHL_key *pChlKey, _In_ CHL_KEYTYPE keyType, _Inout_ PVOID pvKeyOut, _In_ BOOL fGetPointerOnly)
{
    HRESULT hr = S_OK;

    ASSERT(pChlKey && pvKeyOut);
    ASSERT((keyType > CHL_KT_START) && (keyType < CHL_KT_END));

    DBG_UNREFERENCED_PARAMETER(fGetPointerOnly);

    switch(keyType)
    {
        case CHL_KT_INT32:
        {
            *((int*)pvKeyOut) = pChlKey->iKey;
            break;
        }

        case CHL_KT_UINT32:
        {
            *((PUINT)pvKeyOut) = pChlKey->uiKey;
            break;
        }

        case CHL_KT_POINTER:
        {
            *((PVOID*)pvKeyOut) = pChlKey->pvKey;
            break;
        }

        case CHL_KT_STRING:
        {
            *((char**)pvKeyOut) = pChlKey->pszKey;  // TODO: Use fGetPointerOnly
            break;
        }

        case CHL_KT_WSTRING:
        {
            *((WCHAR**)pvKeyOut) = pChlKey->pwszKey;    // TODO: Use fGetPointerOnly
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

BOOL _IsDuplicateKey(_In_ CHL_key *pChlLeftKey, _In_ PCVOID pvRightKey, _In_ CHL_KEYTYPE keyType, _In_ int iKeySize)
{
    BOOL fMatch = FALSE;

    ASSERT(pChlLeftKey);
    ASSERT((keyType > CHL_KT_START) && (keyType < CHL_KT_END));

    switch(keyType)
    {
        case CHL_KT_INT32:
        {
            fMatch = (pChlLeftKey->iKey == (int)pvRightKey);
            break;
        }

        case CHL_KT_UINT32:
        {
            fMatch = (pChlLeftKey->uiKey == (UINT)pvRightKey);
            break;
        }

        case CHL_KT_POINTER:
        {
            fMatch = (pChlLeftKey->pvKey == pvRightKey);
            break;
        }

        case CHL_KT_STRING:
        {
            if((iKeySize > 0) && (pvRightKey != NULL))
            {
                fMatch = strncmp(pChlLeftKey->pszKey, (PCSTR)pvRightKey, iKeySize) == 0;
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
                fMatch = wcsncmp(pChlLeftKey->pwszKey, (PCWSTR)pvRightKey, iKeySize) == 0;   
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

void _DeleteKey(_In_ CHL_key *pChlKey, _In_ CHL_KEYTYPE keyType)
{
    switch(keyType)
    {
        case CHL_KT_STRING:
        {
            CHL_MmFree((PVOID*)&pChlKey->pszKey);
            break;
        }

        case CHL_KT_WSTRING:
        {
            CHL_MmFree((PVOID*)&pChlKey->pwszKey);
            break;
        }

        default:
        {
            break;
        }
    }
}

HRESULT _GetKeySize(_In_ PVOID pvKey, _In_ CHL_KEYTYPE keyType, _In_ PINT piKeySize)
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
            *piKeySize = strlen((PCSTR)pvKey) + sizeof(char);
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

HRESULT _CopyValIn(_In_ CHL_val *pChlVal, _In_ CHL_VALTYPE valType, _In_ PCVOID pvVal, _In_opt_ int iValSize)
{
    HRESULT hr = S_OK;

    ASSERT(pChlVal && (iValSize > 0));
    ASSERT((valType > CHL_VT_START) && (valType < CHL_VT_END));

    switch(valType)
    {
        case CHL_VT_INT32:
        {
            pChlVal->iVal = (int)pvVal;
            break;
        }

        case CHL_VT_UINT32:
        {
            pChlVal->uiVal = (UINT)pvVal;
            break;
        }

        case CHL_VT_POINTER:
        {
            hr = (pvVal != NULL) ? S_OK : E_INVALIDARG;
            if(SUCCEEDED(hr))
            {
                pChlVal->pvPtr = pvVal;
            }
            break;
        }

        case CHL_VT_USEROBJECT:
        {
            hr = ((pvVal != NULL) && (iValSize > 0)) ? S_OK : E_INVALIDARG;
            if(SUCCEEDED(hr))
            {
                hr = CHL_MmAlloc((PVOID*)&pChlVal->pvUserObj, iValSize, NULL);
                if(SUCCEEDED(hr))
                {
                    memcpy(pChlVal->pvUserObj, pvVal, iValSize);
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
                hr = CHL_MmAlloc((PVOID*)&pChlVal->pszVal, iValSize, NULL);
                if(SUCCEEDED(hr))
                {
                    memcpy(pChlVal->pszVal, psz, iValSize);
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
                hr = CHL_MmAlloc((PVOID*)&pChlVal->pwszVal, iValSize, NULL);
                if(SUCCEEDED(hr))
                {
                    memcpy(pChlVal->pwszVal, pwsz, iValSize);
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
    return hr;
}

HRESULT _CopyValOut(_In_ CHL_val *pChlVal, _In_ CHL_VALTYPE valType, _Inout_ PVOID pvValOut, _In_ BOOL fGetPointerOnly)
{
    HRESULT hr = S_OK;

    ASSERT(pChlVal && pvValOut);
    ASSERT((valType > CHL_VT_START) && (valType < CHL_VT_END));

    DBG_UNREFERENCED_PARAMETER(fGetPointerOnly);

    switch(valType)
    {
        case CHL_VT_INT32:
        {
            *((int*)pvValOut) = pChlVal->iVal;
            break;
        }

        case CHL_VT_UINT32:
        {
            *((PUINT)pvValOut) = pChlVal->uiVal;
            break;
        }

        case CHL_VT_POINTER:
        {
            *((PVOID*)pvValOut) = pChlVal->pvPtr;
            break;
        }

        case CHL_VT_USEROBJECT:
        {
            //memcpy(ppValOut, pnode->chlVal.pvUserObj, pnode->dwValSize);
            *((PVOID*)pvValOut) = pChlVal->pvUserObj;   // TODO: Return object itself based on fGetPointerOnly
            break;
        }

        case CHL_VT_STRING:
        {
            //memcpy(ppValOut, pnode->chlVal.pszVal, pnode->dwValSize);
            *((PVOID*)pvValOut) = pChlVal->pszVal;   // TODO: Return object itself based on fGetPointerOnly
            break;
        }

        case CHL_VT_WSTRING:
        {
            //memcpy(ppValOut, pnode->chlVal.pwszVal, pnode->dwValSize);
            *((PVOID*)pvValOut) = pChlVal->pwszVal;   // TODO: Return object itself based on fGetPointerOnly
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

BOOL _IsDuplicateVal(_In_ CHL_val *pChlLeftVal, _In_ PCVOID pvRightVal, _In_ CHL_VALTYPE valType, _In_ int iValSize)
{
    BOOL fMatch = FALSE;

    ASSERT(pChlLeftVal);
    ASSERT((valType > CHL_VT_START) && (valType < CHL_VT_END));

    switch(valType)
    {
        case CHL_VT_INT32:
        {
            fMatch = (pChlLeftVal->iVal == (int)pvRightVal);
            break;
        }

        case CHL_VT_UINT32:
        {
            fMatch = (pChlLeftVal->uiVal == (UINT)pvRightVal);
            break;
        }

        case CHL_VT_POINTER:
        {
            fMatch = (pChlLeftVal->pvPtr == pvRightVal);
            break;
        }

        case CHL_VT_STRING:
        {
            if((iValSize > 0) && (pvRightVal != NULL))
            {
                fMatch = strncmp(pChlLeftVal->pszVal, (PCSTR)pvRightVal, iValSize) == 0;   
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
                fMatch = wcsncmp(pChlLeftVal->pwszVal, (PCWSTR)pvRightVal, iValSize) == 0;   
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

void _DeleteVal(_In_ CHL_val *pChlVal, _In_ CHL_VALTYPE valType)
{
    switch(valType)
    {
        case CHL_VT_USEROBJECT:
        {
            CHL_MmFree((PVOID*)&pChlVal->pvUserObj);
            break;
        }

        case CHL_VT_STRING:
        {
            CHL_MmFree((PVOID*)&pChlVal->pszVal);
            break;
        }

        case CHL_VT_WSTRING:
        {
            CHL_MmFree((PVOID*)&pChlVal->pwszVal);
            break;
        }

        default:
        {
            break;
        }
    }
}

HRESULT _GetValSize(_In_ PVOID pvVal, _In_ CHL_VALTYPE valType, _In_ PINT piValSize)
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
            *piValSize = strlen((PCSTR)pvVal) + sizeof(char);
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
