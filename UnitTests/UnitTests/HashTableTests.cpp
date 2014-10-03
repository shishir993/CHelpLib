
#include "HashTableTests.h"
#include "TestUtil.h"

#include "Hashtable.h"

extern WCHAR* g_pszPassed;
extern WCHAR* g_pszFailed;

#define MAX_RAND_COUNT      9999

struct _keyvalNumbers {
    DWORD dwkey;
    DWORD dwval;
};

struct _keyvalWstrNum {
    PWSTR pszKey;
    DWORD dwval;
};

char testKeys[][8] = {
    "ab", 
    "a", 
    "you", 
    "fourth", 
    "now", 
    "sixth", 
    "7th one"};

char testStrings[][64] = {
    "dfaldfja", 
    "fdslfjoijfaldsfj", 
    "hello there", 
    "I am VS2012", 
    "The C Compiler",
    "From Microsoft", 
    "where my boss works"};

DWORD testKeysNum[] = { 0x080011211, 0x0041106E, 0x004118A0, 
                        0,           1,          -1,
                        15123};

#pragma region Helpers

BOOL _CompareRetrievedKeyVals(struct _keyvalNumbers *pOriginal, int nOrigCount, struct _keyvalNumbers *pRetrieved, int nRetCount)
{
    BOOL fAllFound = TRUE;
    int numNotFound = 0;

    int i, j;

    wprintf(L"Comparing two keyvals.... **************************\n");
    wprintf(L"Orig = %d :: Ret = %d\n\n", nOrigCount, nRetCount);

    // n^2 compare algorithm for now
    for(i = 0; i < nOrigCount; ++i)
    {
        for(j = 0; j < nRetCount; ++j)
        {
            if(pOriginal[i].dwkey == pRetrieved[j].dwkey && 
                pOriginal[i].dwval == pRetrieved[j].dwval)
            {
                break;
            }
        }

        if(j >= nRetCount)
        {
            ++numNotFound;
            fAllFound = FALSE;
            wprintf(L"NOT FOUND %u : %u\n", pOriginal[i].dwkey, pOriginal[i].dwval);
        }
    }

    wprintf(L"\nDONE Comparing two keyvals.... *********************\n");
    wprintf(L"Number of non-matches = %d\n", numNotFound);

    return fAllFound;
}

BOOL _CompareRetrievedKeyVals(struct _keyvalWstrNum *pOriginal, int nOrigCount, struct _keyvalWstrNum *pRetrieved, int nRetCount)
{
    BOOL fAllFound = TRUE;
    int numNotFound = 0;

    int i, j;

    wprintf(L"Comparing two keyvals.... **************************\n");
    wprintf(L"Orig = %d :: Ret = %d\n\n", nOrigCount, nRetCount);
    if(nOrigCount != nRetCount)
    {
        wprintf(L"Counts DO NOT MATCH\n");
        return FALSE;
    }

    // n^2 compare algorithm for now
    for(i = 0; i < nOrigCount; ++i)
    {
        for(j = 0; j < nRetCount; ++j)
        {
            if((pOriginal[i].pszKey != NULL) && (pRetrieved[j].pszKey))
            {
                if((wcscmp(pOriginal[i].pszKey, pRetrieved[j].pszKey) == 0) &&
                    pOriginal[i].dwval == pRetrieved[j].dwval)
                {
                    break;
                }
            }
        }

        if(j >= nRetCount)
        {
            ++numNotFound;
            fAllFound = FALSE;
            wprintf(L"NOT FOUND %s : %u\n", pOriginal[i].pszKey, pOriginal[i].dwval);
        }
    }

    wprintf(L"\nDONE Comparing two keyvals.... *********************\n");
    wprintf(L"Number of non-matches = %d\n", numNotFound);

    return fAllFound;
}

#pragma endregion Helpers

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

#pragma region FuncTests

BOOL _TableSizes()
{
    int tableSize = 0;

    int entriesToCheck[] = { 50000, 100000, 700000, 1<<23, 1<<24 };

    for(int i = 0; i < _countof(entriesToCheck); ++i)
        wprintf(L"%10d %d\n", entriesToCheck[i], CHL_DsGetNearestSizeIndexHT(entriesToCheck[i]));

    return TRUE;
}


BOOL _StrStr()
{
    CHL_HTABLE *phtable;

    int outValSize = 0;
    int nItemsInserted = 0;

    // Create a str:str htable
    if(FAILED(CHL_DsCreateHT(&phtable, 3, CHL_KT_STRING, CHL_VT_STRING, FALSE)))
        return FALSE;

    // Insert abcd:"hello,there"
    char *pch = "hello, there";
    if(FAILED(CHL_DsInsertHT(phtable, "key", 4, pch, strlen(pch)+1)))
        printf("Insert key:%s failed\n", "key", pch);
    ++nItemsInserted;

    // find it
    if(FAILED(CHL_DsFindHT(phtable, "key", 4, &pch, &outValSize, TRUE)))
        printf("Find key failed\n");
    else
    {
        if(outValSize != strlen("hello, there")+1)
            printf("fFind returned %d valsize\n", outValSize);
        if(strcmp(pch, "hello, there") != 0)
            printf("fFind returned incorrect value for key\n");
        else
            printf("fFind returned [%s]\n", pch);
    }
    
    // insert the same thing again
    pch = "hello, there";
    if(FAILED(CHL_DsInsertHT(phtable, "key", 4, pch, strlen(pch)+1)))
        printf("Insert key:%s failed\n", "key", pch);

    // insert another dgadsf:"did the first test pass?"
    pch = "did the first test pass?";
    if(FAILED(CHL_DsInsertHT(phtable, "dgadsf", 7, pch, strlen(pch)+1)))
        printf("Insert key:%s failed\n", "dgadsf", pch);
    ++nItemsInserted;

    // find the first item
    if(FAILED(CHL_DsFindHT(phtable, "key", 4, &pch, &outValSize, TRUE)))
        printf("Find key failed\n");
    else
    {
        if(outValSize <= 0)
            printf("fFind returned %d valsize\n", outValSize);
        if(strcmp(pch, "hello, there") != 0)
            printf("fFind returned incorrect value for key\n");
        else
            printf("fFind returned [%s]\n", pch);
    }

    // remove the first item
    if(FAILED(CHL_DsRemoveHT(phtable, "key", 4)))
        printf("Remove key failed\n");
    --nItemsInserted;

    // find it: fail to find
    if(FAILED(CHL_DsFindHT(phtable, "key", 4, &pch, &outValSize, TRUE)))
        printf("Find key PASSED\n");
    else
    {
        printf("Find key after removal failed\n");
    }

    // insert everything using test keys and strings
    for(int keys = 0, vals = 0; vals < _countof(testStrings); ++vals, ++keys)
    {
        if(FAILED(CHL_DsInsertHT(phtable, testKeys[keys], strlen(testKeys[keys])+1, 
            testStrings[vals], strlen(testStrings[vals])+1)))
        {
            printf("Failed to insert %s:%s\n", testKeys[keys], testStrings[vals]);
        }
        else
        {
            ++nItemsInserted;
            printf("Inserted %s:%s\n", testKeys[keys], testStrings[vals]);
        }
    }

    // find everything
    for(int keys = 0; keys < _countof(testKeys); ++keys)
    {
        if(FAILED(CHL_DsFindHT(phtable, testKeys[keys], strlen(testKeys[keys])+1, &pch, &outValSize, TRUE)))
            printf("Failed to find val for key %s\n", testKeys[keys]);
        else
        {
            printf("FOUND: %s:[%s]\n", testKeys[keys], pch);
        }
    }

    // iterate through and print everything
    CHL_HT_ITERATOR itr;
    char *pskey, *psval;
    int keysize, valsize, nfound = 0;

    printf("Iterating through hashtable items... *******************\n");
    CHL_DsInitIteratorHT(&itr);
    while(SUCCEEDED(CHL_DsGetNextHT(phtable, &itr, &pskey, &keysize, &psval, &valsize, TRUE)))
    {
        int i = 0;
        printf("KEY:%d:", keysize);
        while(i < keysize) putchar(pskey[i++]);
        putchar('\n');

        i = 0;
        printf("VAL:%d:", valsize);
        while(i < valsize) putchar(psval[i++]);
        putchar('\n');
        ++nfound;
    }
    printf("DONE Iterating through hashtable items... *******************\n");

    if(nfound != nItemsInserted)
        printf("FAILED interation test...");

    if(SUCCEEDED(CHL_DsFindHT(phtable, "notthere", 9, &pch, &outValSize, TRUE)))
        printf("FAILED: Finding a non-inserted key\n");

    // remove some and dump table
    if(FAILED(CHL_DsRemoveHT(phtable, "dgadsf", 7)))
        printf("Failed to remove dgadsf\n");

    if(FAILED(CHL_DsRemoveHT(phtable, testKeys[2], strlen(testKeys[2])+1)))
        printf("Failed to remove testKeys[2]\n");

    if(FAILED(CHL_DsRemoveHT(phtable, testKeys[4], strlen(testKeys[4])+1)))
        printf("Failed to remove testKeys[2]\n");

    CHL_DsDumpHT(phtable);
    CHL_DsDestroyHT(phtable);

    OutputDebugString(L"Finished first test");
    return TRUE;    
}

BOOL _NumStr()
{
    CHL_HTABLE *phtable;

    int outValSize = 0;
    int nItemsInserted = 0;

    // Create a str:str htable
    if(FAILED(CHL_DsCreateHT(&phtable, 3, CHL_KT_UINT32, CHL_VT_STRING, FALSE)))
        return FALSE;

    // Insert abcd:"hello,there"
    DWORD key = 2131;
    char *pch = "hello, there";
    if(FAILED(CHL_DsInsertHT(phtable, (PVOID)key, sizeof(DWORD), pch, strlen(pch)+1)))
        printf("Insert %u:%s failed\n", key, pch);
    ++nItemsInserted;

    // find it
    pch = NULL;
    if(FAILED(CHL_DsFindHT(phtable, (PVOID)key, sizeof(DWORD), &pch, &outValSize, TRUE)))
        printf("Find key failed\n");
    else
    {
        if(outValSize != sizeof(char*))
            printf("fFind returned %d valsize\n", outValSize);
        if(strcmp(pch, "hello, there") != 0)
            printf("fFind returned incorrect value for %u\n", key);
        else
            printf("fFind returned [%s]\n", pch);
    }
    
    // insert the same thing again
    key = 2131;
    pch = "hello, there";
    if(FAILED(CHL_DsInsertHT(phtable, (PVOID)key, sizeof(DWORD), pch, strlen(pch)+1)))
        printf("Insert %u:%s failed\n", key, pch);

    // insert another dgadsf:"did the first test pass?"
    key = 22231;
    pch = "did the first test pass?";
    if(FAILED(CHL_DsInsertHT(phtable, (PVOID)key, sizeof(DWORD), pch, strlen(pch)+1)))
        printf("Insert %u:%s failed\n", key, pch);
    ++nItemsInserted;

    // find the first item
    key = 2131;
    pch = NULL;
    if(FAILED(CHL_DsFindHT(phtable, (PVOID)key, sizeof(DWORD), &pch, &outValSize, TRUE)))
        printf("Find key failed\n");
    else
    {
        if(outValSize <= 0)
            printf("fFind returned %d valsize\n", outValSize);
        if(strcmp(pch, "hello, there") != 0)
            printf("fFind returned incorrect value for key\n");
        else
            printf("fFind returned [%s]\n", pch);
    }

    // remove the first item
    key = 2131;
    if(FAILED(CHL_DsRemoveHT(phtable, (PVOID)key, sizeof(DWORD))))
        printf("Remove key failed\n");
    --nItemsInserted;

    // find it: fail to find
    if(FAILED(CHL_DsFindHT(phtable, (PVOID)key, sizeof(DWORD), &pch, &outValSize, TRUE)))
        printf("Find key PASSED\n");
    else
    {
        printf("Find key after removal failed\n");
    }

    // insert everything using test keys and strings
    for(int keys = 0, vals = 0; vals < _countof(testStrings); ++keys, ++vals)
    {
        if(FAILED(CHL_DsInsertHT(phtable, &(testKeysNum[keys]), sizeof(DWORD), 
            testStrings[vals], strlen(testStrings[vals])+1)))
        {
            printf("Failed to insert %u:%s\n", testKeysNum[keys], testStrings[vals]);
        }
        else
            ++nItemsInserted;
    }

    // find everything
    for(int keys = 0; keys < _countof(testKeys); ++keys)
    {
        pch = NULL;
        if(FAILED(CHL_DsFindHT(phtable, &(testKeysNum[keys]), sizeof(DWORD), &pch, &outValSize, TRUE)))
            printf("Failed to find val for key %u\n", testKeysNum[keys]);
        else
        {
            printf("FOUND: %u:[%s]\n", testKeysNum[keys], pch);
        }
    }

    // iterate through and print everything
    CHL_HT_ITERATOR itr;
    DWORD dwkey;
    char *psval;
    int keysize, valsize, nfound = 0;

    printf("Iterating through hashtable items... *******************\n");
    CHL_DsInitIteratorHT(&itr);
    while(SUCCEEDED(CHL_DsGetNextHT(phtable, &itr, &dwkey, &keysize, &psval, &valsize, TRUE)))
    {
        int i = 0;
        printf("KEY:%d:%d\n", keysize, dwkey);
        dwkey = 0;

        printf("VAL:%d:", valsize);
        while(i < valsize) putchar(psval[i++]);
        putchar('\n');
        ++nfound;
    }
    printf("DONE Iterating through hashtable items... *******************\n");

    if(nfound != nItemsInserted)
        printf("FAILED interation test...");

    key = 0xdead;
    if(SUCCEEDED(CHL_DsFindHT(phtable, (PVOID)key, sizeof(DWORD), &pch, &outValSize, TRUE)))
        printf("FAILED: Finding a non-inserted key\n");

    // remove some and dump table
    key = 22231;
    if(FAILED(CHL_DsRemoveHT(phtable, (PVOID)key, sizeof(DWORD))))
        printf("Failed to remove dgadsf\n");

    if(FAILED(CHL_DsRemoveHT(phtable, &testKeysNum[2], sizeof(DWORD))))
        printf("Failed to remove testKeysNum[2]\n");

    if(FAILED(CHL_DsRemoveHT(phtable, &testKeysNum[4], sizeof(DWORD))))
        printf("Failed to remove testKeysNum[4]\n");

    CHL_DsDumpHT(phtable);
    CHL_DsDestroyHT(phtable);

    OutputDebugString(L"Finished second test");
    return TRUE;    
}

BOOL _NumStrRand()
{
    CHL_HTABLE *phtable = NULL;
    CHL_HT_ITERATOR itr;

    BOOL fSuccess = TRUE;

    struct _keyvalNumbers stRandKeyVals[MAX_RAND_COUNT];

    // generate the random keys
    wprintf(L"Generating random numbers... ");
    for(int i = 0; i < MAX_RAND_COUNT; ++i)
    {
        //if( rand_s(&ui) != 0 )
        //{
        //    wprintf(L"rand_s() failed %d\n", GetLastError());
        //    --i;    // retry same index
        //    Sleep(250);
        //    continue;
        //}
        //ui = ui % (MAX_RAND_COUNT+1);
        stRandKeyVals[i].dwkey = i;
        stRandKeyVals[i].dwval = MAX_RAND_COUNT - i;
        //Sleep(100);
    }

    wprintf(L" done\n");

    __try
    {
        if(FAILED(CHL_DsCreateHT(&phtable, MAX_RAND_COUNT, CHL_KT_UINT32, CHL_VT_UINT32, FALSE)))
        {
            fSuccess = FALSE;
            wprintf(L"Failed to create hash table\n");
            __leave;    
        }

        // insert all keys
        for(int i = 0; i < MAX_RAND_COUNT; ++i)
        {
            if(FAILED(CHL_DsInsertHT(phtable, (PVOID)stRandKeyVals[i].dwkey, sizeof(DWORD), (PVOID)stRandKeyVals[i].dwval, sizeof(DWORD))))
            {
                wprintf(L"Unable to insert %u:%d\n", stRandKeyVals[i], i);
                fSuccess = FALSE;
                stRandKeyVals[i].dwkey = -1;
                stRandKeyVals[i].dwval = -1;
            }
        }

        // find all keys
        CHL_DsInitIteratorHT(&itr);
        DWORD dwkey;
        DWORD dwval;
        int keysize, valsize;
        struct _keyvalNumbers retKeyVal[MAX_RAND_COUNT];

        wprintf(L"Contents of hashtable now... *********************\n\n");

        int numfound = 0;
        while(SUCCEEDED(CHL_DsGetNextHT(phtable, &itr, &dwkey, &keysize, &dwval, &valsize, FALSE)))
        {
            if(numfound >= MAX_RAND_COUNT)
            {
                wprintf(L"WTF? Hashtable is returning more than what was put in!!\n");
                fSuccess = FALSE;
                break;
            }

            if(keysize != sizeof(DWORD) || valsize != sizeof(DWORD))
            {
                wprintf(L"%d: keysize = %d :: valsize = %d\n", numfound, keysize, valsize);
                fSuccess = FALSE;
            }
            retKeyVal[numfound].dwkey = dwkey;
            retKeyVal[numfound].dwval = dwval;
            ++numfound;

            wprintf(L"%u : %d\n", dwkey, dwval);
        }

        wprintf(L"END OF Contents of hashtable now... ***************\n\n");

        if(!_CompareRetrievedKeyVals(&stRandKeyVals[0], MAX_RAND_COUNT, &retKeyVal[0], numfound))
        {
            wprintf(L"Original and Returned keyvals DO NOT MATCH\n");
            fSuccess = FALSE;
        }

        // find all the values
        DWORD foundval = 0;
        valsize = 0;
        int numnotfound = 0;
        int numWrong = 0;
        int numValSizeWrong = 0;
        wprintf(L"Finding all values in reverse now... ***************\n\n");
        for(int i = MAX_RAND_COUNT-1; i >= 0; --i)
        {
            if(FAILED(CHL_DsFindHT(phtable, (PVOID)stRandKeyVals[i].dwkey, sizeof(DWORD), &foundval, &valsize, FALSE)))
            {
                ++numnotfound;
                wprintf(L"Not found: %u : %u\n", stRandKeyVals[i].dwkey, stRandKeyVals[i].dwval);
            }
            else
            {
                if(valsize != sizeof(DWORD))
                {
                    ++numValSizeWrong;
                    wprintf(L"Invalid valsize %d\n", valsize);
                }
                if(foundval != stRandKeyVals[i].dwval)
                {
                    ++numWrong;
                    wprintf(L"Found but WRONG val: %u != %u\n", stRandKeyVals[i].dwval, foundval);
                }
            }
        }
        wprintf(L"DONE Finding all values in reverse now... ************\n\n");
        wprintf(L"NOT FOUND = %d :: WRONG = %d :: VALSIZE WRONG = %d\n", numnotfound, numWrong, numValSizeWrong);

        if(numnotfound || numWrong || numValSizeWrong)
            fSuccess = FALSE;

    }
    __finally
    {
        if(phtable && FAILED(CHL_DsDestroyHT(phtable)))
        {
            wprintf(L"Failed to destroy hash table\n");
        }
        phtable = NULL;
    }

    return fSuccess;

}

void _WstrNumRand_FreeUntil(struct _keyvalWstrNum* pKeyVals, int freeUntilIndex)
{
    for(int i = 0; i <= freeUntilIndex; ++i)
    {
        if(pKeyVals[i].pszKey)
        {
            free(pKeyVals[i].pszKey);
        }
    }
}

BOOL _WstrNumRand()
{
    CHL_HTABLE *phtable = NULL;
    CHL_HT_ITERATOR itr;

    BOOL fSuccess = TRUE;

    int aiKeyLengths[] = { 8, 16, 24, 32, 48, 64 };
    struct _keyvalWstrNum stRandKeyVals[MAX_RAND_COUNT];

    int numKeyLengths = ARRAYSIZE(aiKeyLengths);

    ZeroMemory(stRandKeyVals, sizeof(stRandKeyVals));

    // generate the random data
    wprintf(L"Generating random test key,value pairs ... ");
    for(int i = 0; i < MAX_RAND_COUNT; ++i)
    {
        register int sizeBytes = aiKeyLengths[i % numKeyLengths] * sizeof WCHAR;
        register PWSTR psz = (PWSTR)malloc(sizeBytes);
        if(psz)
        {
            if(SUCCEEDED(CreateRandString(psz, sizeBytes/sizeof WCHAR)))
            {
                stRandKeyVals[i].pszKey = psz;
                stRandKeyVals[i].dwval = i;
            }
            else
            {
                wprintf(L"CreateRandString() failed when generating %d element\n", i);
                _WstrNumRand_FreeUntil(stRandKeyVals, i-1);
                fSuccess = FALSE;
                break;
            }
        }
        else
        {
            wprintf(L"!!!! Out of memory when generating %d element !!!!\n", i);
            _WstrNumRand_FreeUntil(stRandKeyVals, i-1);
            fSuccess = FALSE;
            break;
        }
    }

    __try
    {
        if(!fSuccess)
        {
            __leave;
        }

        wprintf(L"Generated random test data\n");

        if(FAILED(CHL_DsCreateHT(&phtable, MAX_RAND_COUNT, CHL_KT_WSTRING, CHL_VT_UINT32, FALSE)))
        {
            fSuccess = FALSE;
            wprintf(L"Failed to create hash table\n");
            __leave;
        }

        // insert all keys
        for(int i = 0; i < MAX_RAND_COUNT; ++i)
        {
            if(FAILED(CHL_DsInsertHT(phtable, (PVOID)stRandKeyVals[i].pszKey, 0, (PVOID)stRandKeyVals[i].dwval, sizeof(DWORD))))
            {
                wprintf(L"Unable to insert %s:%d\n", stRandKeyVals[i].pszKey, stRandKeyVals[i].dwval);
                fSuccess = FALSE;
                __leave;
            }
        }

        // find all keys
        CHL_DsInitIteratorHT(&itr);
        PWSTR pszRetKey;
        DWORD dwRetVal;
        int keysize, valsize;
        struct _keyvalWstrNum retKeyVal[MAX_RAND_COUNT];

        wprintf(L"Retrieve full content of hashtable now... *********************\n\n");

        int numfound = 0;
        while(SUCCEEDED(CHL_DsGetNextHT(phtable, &itr, &pszRetKey, &keysize, &dwRetVal, &valsize, TRUE)))
        {
            if(numfound >= MAX_RAND_COUNT)
            {
                wprintf(L"WTF? Hashtable is returning more than what was put in!!\n");
                fSuccess = FALSE;
                break;
            }

            if(valsize != sizeof(DWORD))
            {
                wprintf(L"%d: keysize = %d :: valsize = %d\n", numfound, keysize, valsize);
                fSuccess = FALSE;
            }
            retKeyVal[numfound].pszKey= pszRetKey;
            retKeyVal[numfound].dwval = dwRetVal;
            ++numfound;

            wprintf(L"%s : %d\n", pszRetKey, dwRetVal);
        }

        wprintf(L"END OF Retrieving full content of hashtable... ***************\n\n");

        if(!_CompareRetrievedKeyVals(&stRandKeyVals[0], MAX_RAND_COUNT, &retKeyVal[0], numfound))
        {
            wprintf(L"Original and Returned keyvals DO NOT MATCH\n");
            fSuccess = FALSE;
        }

        // find all the values in reverse now
        DWORD foundval = 0;
        valsize = 0;
        int numnotfound = 0;
        int numWrong = 0;
        int numValSizeWrong = 0;
        wprintf(L"Finding all values in reverse now... ***************\n\n");
        for(int i = MAX_RAND_COUNT-1; i >= 0; --i)
        {
            if(FAILED(CHL_DsFindHT(phtable, (PVOID)stRandKeyVals[i].pszKey, 0, &foundval, &valsize, FALSE)))
            {
                ++numnotfound;
                wprintf(L"Not found: %u : %u\n", stRandKeyVals[i].pszKey, stRandKeyVals[i].dwval);
            }
            else
            {
                if(valsize != sizeof(DWORD))
                {
                    ++numValSizeWrong;
                    wprintf(L"Invalid valsize %d\n", valsize);
                }

                if(foundval != stRandKeyVals[i].dwval)
                {
                    ++numWrong;
                    wprintf(L"Found but WRONG val: %u != %u\n", stRandKeyVals[i].dwval, foundval);
                }
            }
        }
        wprintf(L"DONE Finding all values in reverse now... ************\n\n");
        wprintf(L"NOT FOUND = %d :: WRONG = %d :: VALSIZE WRONG = %d\n", numnotfound, numWrong, numValSizeWrong);

        if(numnotfound || numWrong || numValSizeWrong)
        {
            fSuccess = FALSE;
        }

    }
    __finally
    {
        if(phtable && FAILED(CHL_DsDestroyHT(phtable)))
        {
            wprintf(L"Failed to destroy hash table\n");
        }
        phtable = NULL;

        _WstrNumRand_FreeUntil(stRandKeyVals, ARRAYSIZE(stRandKeyVals) - 1);
    }

    return fSuccess;

}

#pragma endregion FuncTests

BOOL ExecUnitTestsHT()
{
    int nFailed = 0;

    wprintf(L"\n-----------------------------------------------------\n");
    wprintf(L"\n---------------- Hashtable Unit Tests ---------------\n");

    wchar_t *pszTestName = L"Wide char hash function";
    wprintf(L"\n-----------------------------------------------\nStarting Test: %s\n", pszTestName);
    if(!_HashFuncW())
    {
        ++nFailed;
    }
    else
    {
        wprintf(L"\nPASSED: %s\n-----------------------------------------------\n", pszTestName);
    }

    wprintf(L"\nEND TEST SUITE: Hashtable Unit Tests: %s\n", CHOOSE_TEST_OUTCOME(nFailed == 0));

    return (nFailed == 0);
}

BOOL ExecFuncTestsHT()
{
    int nFailed = 0;

    wprintf(L"\n-----------------------------------------------------\n");
    wprintf(L"\n---------------- Hashtable Functional ---------------\n");

    wchar_t *pszTestName = L"Hashtable: String, String";
    wprintf(L"\n-----------------------------------------------\nStarting Test: %s\n", pszTestName);
    if(!_StrStr())
    {
        wprintf(L"\nFAILED: %s\n-----------------------------------------------\n", pszTestName);
        ++nFailed;
    }
    else
    {
        wprintf(L"\nPASSED: %s\n-----------------------------------------------\n", pszTestName);
    }

    pszTestName = L"Hashtable: Number, String";
    wprintf(L"\n-----------------------------------------------\nStarting Test: %s\n", pszTestName);
    if(!_NumStr())
    {
        wprintf(L"\nFAILED: %s\n-----------------------------------------------\n", pszTestName);
        ++nFailed;
    }
    else
    {
        wprintf(L"\nPASSED: %s\n-----------------------------------------------\n", pszTestName);
    }

    pszTestName = L"Hashtable: Number, String. Random.";
    wprintf(L"\n-----------------------------------------------\nStarting Test: %s\n", pszTestName);
    if(!_NumStrRand())
    {
        wprintf(L"\nFAILED: %s\n-----------------------------------------------\n", pszTestName);
        ++nFailed;
    }
    else
    {
        wprintf(L"\nPASSED: %s\n-----------------------------------------------\n", pszTestName);
    }

    pszTestName = L"Hashtable: Wide String, Number. Random.";
    wprintf(L"\n-----------------------------------------------\nStarting Test: %s\n", pszTestName);
    if(!_WstrNumRand())
    {
        wprintf(L"\nFAILED: %s\n-----------------------------------------------\n", pszTestName);
        ++nFailed;
    }
    else
    {
        wprintf(L"\nPASSED: %s\n-----------------------------------------------\n", pszTestName);
    }

    pszTestName = L"Hashtable: Table sizes.";
    wprintf(L"\n-----------------------------------------------\nStarting Test: %s\n", pszTestName);
    if(!_TableSizes())
    {
        wprintf(L"\nFAILED: %s\n-----------------------------------------------\n", pszTestName);
        ++nFailed;
    }
    else
    {
        wprintf(L"\nPASSED: %s\n-----------------------------------------------\n", pszTestName);
    }

    wprintf(L"\nEND TEST SUITE: Hashtable Functional: %s\n", CHOOSE_TEST_OUTCOME(nFailed == 0));

    return (nFailed == 0);
}
