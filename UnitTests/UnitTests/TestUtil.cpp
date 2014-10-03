
#include "TestUtil.h"

HRESULT CreateRandString(_Out_ PSTR pszBuffer, _In_ int nChars)
{
    static char szSource[] = "qwertyuiopasdfghjklzxcvbnm1234567890!@#$%^&*()";
    static int nElems = sizeof szSource / sizeof szSource[0];

    pszBuffer[0] = 0;

    UINT uiSelector;
    for(int i = 0; i < nChars - 1; ++i)
    {
        if(rand_s(&uiSelector))
        {
            return E_FAIL;
        }

        uiSelector = uiSelector % (nElems-1);
        pszBuffer[i] = szSource[uiSelector];
    }
    
    pszBuffer[nChars - 1] = 0;
    return S_OK;
}

// Wide string version
HRESULT CreateRandString(_Out_ PWSTR pszBuffer, _In_ int nChars)
{
    static WCHAR szSource[] = L"qwertyuiopasdfghjklzxcvbnm1234567890!@#$%^&*()";
    static int nElems = sizeof szSource / sizeof szSource[0];

    pszBuffer[0] = 0;

    UINT uiSelector;
    for(int i = 0; i < nChars - 1; ++i)
    {
        if(rand_s(&uiSelector))
        {
            return E_FAIL;
        }

        uiSelector = uiSelector % (nElems-1);
        pszBuffer[i] = szSource[uiSelector];
    }
    
    pszBuffer[nChars - 1] = 0;
    return S_OK;
}