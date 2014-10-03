
#include "HashTableTests.h"
#include "TestUtil.h"

#include "Hashtable.h"

#pragma region UnitTests

BOOL _HashFuncW()
{
    BOOL fRetVal = TRUE;

    // Always store in increasing order. Includes null-terminator.
    int aiKeyLengths[] = { 2, 4, 8, 24, 16, 32, 48, 56, 64, 96, 112, 128 };
    int tableSize = 197;

    wprintf(L"Testing hash functions for wide strings\n");

    // Allocate memory to hold the key
    int maxChars = aiKeyLengths[ARRAYSIZE(aiKeyLengths) - 1];
    PWSTR psz = (PWSTR)malloc(maxChars * sizeof WCHAR);
    if(psz != NULL)
    {
        for(int i = 0; i < ARRAYSIZE(aiKeyLengths); ++i)
        {
            ZeroMemory(psz, maxChars * sizeof WCHAR);
            if(SUCCEEDED(CreateRandString(psz, aiKeyLengths[i])))
            {
                wprintf(L"%u = %s\n", _GetKeyHash(psz, CHL_KT_WSTRING, aiKeyLengths[i], tableSize), psz);
            }
            else
            {
                wprintf(L"%s(): CreateRandString failed\n");
                fRetVal = FALSE;
                break;
            }
        }
    }
    else
    {
        wprintf(L"%s(): Out of memory.\n", __FUNCTIONW__);
        fRetVal = FALSE;
    }
    return fRetVal;
}

#pragma endregion UnitTests

BOOL ExecUnitTestsHT()
{
    int nFailed = 0;

    if(!_HashFuncW())
    {
        ++nFailed;
    }

    return (nFailed == 0);
}
