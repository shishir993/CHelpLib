
#define _CRT_RAND_S
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include "CHelpLibDll.h"

#define MAX_RAND_COUNT  9999

struct _keyval {
    DWORD dwkey;
    DWORD dwval;
};

char testKeys[][8] = {"ab", "a", "you", "fourth", "now", "sixth", "7th one"};
char testStrings[][64] = {"dfaldfja", "fdslfjoijfaldsfj", "hello there", "I am VS2012", "The C Compiler",
                            "From Microsoft", "where my boss works"};

DWORD testKeysNum[] = { 0x080011211, 0x0041106E, 0x004118A0, 
                        0,           1,          -1,
                        15123};

BOOL fTestHT_StrStr();
BOOL fTestHT_NumStr();
BOOL fTestStrings();
BOOL fTestHT_NumStrRand();
BOOL fCompareRetrievedKeyVals(struct _keyval *pOriginal, int nOrigCount, struct _keyval *pRetrieved, int nRetCount);


int main()
{
    BOOL success = TRUE;

    // hashtable tests
    OutputDebugString(L"Starting unit tests on CHelpLib.dll");
    success = fTestHT_StrStr() & success;
    success = fTestHT_NumStr() & success;
    success = fTestStrings() & success;

    success = fTestHT_NumStrRand() & success;

    OutputDebugString(L"Tests done. Exiting...");
    return !success;
}

BOOL fTestHT_StrStr()
{
    CHL_HTABLE *phtable;

    int outValSize = 0;
    int nItemsInserted = 0;

    // Create a str:str htable
    if(!HT_fCreate(&phtable, 3, HT_KEY_STR, HT_VAL_STR))
        return FALSE;

    // Insert abcd:"hello,there"
    char *pch = "hello, there";
    if(!HT_fInsert(phtable, "key", 3, pch, strlen(pch)+1))
        printf("Insert key:%s failed\n", "key", pch);
    ++nItemsInserted;

    // find it
    if(!HT_fFind(phtable, "key", 3, &pch, &outValSize))
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
    if(!HT_fInsert(phtable, "key", 3, pch, strlen(pch)+1))
        printf("Insert key:%s failed\n", "key", pch);

    // insert another dgadsf:"did the first test pass?"
    pch = "did the first test pass?";
    if(!HT_fInsert(phtable, "dgadsf", 6, pch, strlen(pch)+1))
        printf("Insert key:%s failed\n", "dgadsf", pch);
    ++nItemsInserted;

    // find the first item
    if(!HT_fFind(phtable, "key", 3, &pch, &outValSize))
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
    if(!HT_fRemove(phtable, "key", 3))
        printf("Remove key failed\n");
    --nItemsInserted;

    // find it: fail to find
    if(!HT_fFind(phtable, "key", 3, &pch, &outValSize))
        printf("Find key PASSED\n");
    else
    {
        printf("Find key after removal failed\n");
    }

    // insert everything using test keys and strings
    for(int keys = 0, vals = 0; vals < _countof(testStrings); ++vals)
    {
        if(!HT_fInsert(phtable, testKeys[keys], strlen(testKeys[keys]), 
            testStrings[vals], strlen(testStrings[vals])+1))
        {
            printf("Failed to insert %s:%s\n", testKeys[keys], testStrings[vals]);
        }
        else
            ++nItemsInserted;

        keys = (keys + 1) % (_countof(testKeys));
    }

    // find everything
    for(int keys = 0; keys < _countof(testKeys); ++keys)
    {
        if(!HT_fFind(phtable, testKeys[keys], strlen(testKeys[keys]), &pch, &outValSize))
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
    HT_fInitIterator(&itr);
    while(HT_fGetNext(phtable, &itr, &pskey, &keysize, &psval, &valsize))
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

    if(HT_fFind(phtable, "notthere", 8, &pch, &outValSize))
        printf("FAILED: Finding a non-inserted key\n");

    // remove some and dump table
    if(!HT_fRemove(phtable, "dgadsf", 6))
        printf("Failed to remove dgadsf\n");

    if(!HT_fRemove(phtable, testKeys[2], strlen(testKeys[2])))
        printf("Failed to remove testKeys[2]\n");

    if(!HT_fRemove(phtable, testKeys[4], strlen(testKeys[4])))
        printf("Failed to remove testKeys[2]\n");

    HT_vDumpTable(phtable);

    HT_fDestroy(phtable);

    OutputDebugString(L"Finished first test");

    return TRUE;    
}


BOOL fTestHT_NumStr()
{
    CHL_HTABLE *phtable;

    int outValSize = 0;
    int nItemsInserted = 0;

    // Create a str:str htable
    if(!HT_fCreate(&phtable, 3, HT_KEY_DWORD, HT_VAL_STR))
        return FALSE;

    // Insert abcd:"hello,there"
    DWORD key = 2131;
    char *pch = "hello, there";
    if(!HT_fInsert(phtable, &key, sizeof(DWORD), pch, strlen(pch)+1))
        printf("Insert %u:%s failed\n", key, pch);
    ++nItemsInserted;

    // find it
    pch = NULL;
    if(!HT_fFind(phtable, &key, sizeof(DWORD), &pch, &outValSize))
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
    if(!HT_fInsert(phtable, &key, sizeof(DWORD), pch, strlen(pch)+1))
        printf("Insert %u:%s failed\n", key, pch);

    // insert another dgadsf:"did the first test pass?"
    key = 22231;
    pch = "did the first test pass?";
    if(!HT_fInsert(phtable, &key, sizeof(DWORD), pch, strlen(pch)+1))
        printf("Insert %u:%s failed\n", key, pch);
    ++nItemsInserted;

    // find the first item
    key = 2131;
    pch = NULL;
    if(!HT_fFind(phtable, &key, sizeof(DWORD), &pch, &outValSize))
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
    if(!HT_fRemove(phtable, &key, sizeof(DWORD)))
        printf("Remove key failed\n");
    --nItemsInserted;

    // find it: fail to find
    if(!HT_fFind(phtable, &key, sizeof(DWORD), &pch, &outValSize))
        printf("Find key PASSED\n");
    else
    {
        printf("Find key after removal failed\n");
    }

    // insert everything using test keys and strings
    for(int keys = 0, vals = 0; vals < _countof(testStrings); ++keys, ++vals)
    {
        if(!HT_fInsert(phtable, &(testKeysNum[keys]), sizeof(DWORD), 
            testStrings[vals], strlen(testStrings[vals])+1))
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
        if(!HT_fFind(phtable, &(testKeysNum[keys]), sizeof(DWORD), &pch, &outValSize))
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
    HT_fInitIterator(&itr);
    while(HT_fGetNext(phtable, &itr, &dwkey, &keysize, &psval, &valsize))
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
    if(HT_fFind(phtable, &key, sizeof(DWORD), &pch, &outValSize))
        printf("FAILED: Finding a non-inserted key\n");

    // remove some and dump table
    key = 22231;
    if(!HT_fRemove(phtable, &key, sizeof(DWORD)))
        printf("Failed to remove dgadsf\n");

    if(!HT_fRemove(phtable, &testKeysNum[2], sizeof(DWORD)))
        printf("Failed to remove testKeysNum[2]\n");

    if(!HT_fRemove(phtable, &testKeysNum[4], sizeof(DWORD)))
        printf("Failed to remove testKeysNum[4]\n");

    HT_vDumpTable(phtable);

    HT_fDestroy(phtable);
    OutputDebugString(L"Finished second test");
    return TRUE;    
}


BOOL fTestStrings()
{
    BOOL fError = FALSE;

    WCHAR awsTestStrings[][MAX_PATH+1] = { 
        L"", L"z", L"zy", L"dbg.dll", 
        L"C:\\Users\\Shishir_Limited\\Downloads\\Matrix_wallpaper_.jpg",
        L"S:\\MyBriefcase\\Coding\\C\\Remote Monitoring - Internship at RFL\\RM\\Code\\Remote Monitoring - To RFL\\Source Code\\ServerRM\\ServerRM\\ScanIPAddresses.cpp",
        L"ServerRM.vcproj.SHISHIR-0C6645C.Shishir.user" };

    WCHAR *pws = NULL;

    if(Chl_GetFilenameFromPath(NULL, 412))
    {
        fError = TRUE;
        wprintf(L"FAILED: Returned something for NULL!\n");
    }

    // STRING 0
    pws = Chl_GetFilenameFromPath(awsTestStrings[0], wcsnlen_s(awsTestStrings[0], _countof(awsTestStrings[0]))+1);
    if(pws)
    {
        fError = TRUE;
        wprintf(L"FAILED: Path:[%s] -- Filename:[%s]\n", awsTestStrings[0], pws);
    }

    // STRING 1
    pws = Chl_GetFilenameFromPath(awsTestStrings[1], wcsnlen_s(awsTestStrings[1], _countof(awsTestStrings[1]))+1);
    if(!pws)
    {
        fError = TRUE;
        wprintf(L"FAILED: Path:[%s] -- Filename:[(null)]\n", awsTestStrings[1]);
    }
    else if(wcscmp(pws, L"z") != 0)
    {
        wprintf(L"FAILED: Path:[%s] -- Filename:[%s]\n", awsTestStrings[1], pws);
    }

    // STRING 2
    pws = Chl_GetFilenameFromPath(awsTestStrings[2], wcsnlen_s(awsTestStrings[2], _countof(awsTestStrings[2]))+1);
    if(!pws)
    {
        fError = TRUE;
        wprintf(L"FAILED: Path:[%s] -- Filename:[(null)]\n", awsTestStrings[2]);
    }
    else if(wcscmp(pws, L"zy") != 0)
    {
        wprintf(L"FAILED: Path:[%s] -- Filename:[%s]\n", awsTestStrings[2], pws);
    }

    // STRING 3
    pws = Chl_GetFilenameFromPath(awsTestStrings[3], wcsnlen_s(awsTestStrings[3], _countof(awsTestStrings[3]))+1);
    if(!pws)
    {
        fError = TRUE;
        wprintf(L"FAILED: Path:[%s] -- Filename:[(null)]\n", awsTestStrings[3]);
    }
    else if(wcscmp(pws, L"dbg.dll") != 0)
    {
        wprintf(L"FAILED: Path:[%s] -- Filename:[%s]\n", awsTestStrings[3], pws);
    }

    // STRING 4
    pws = Chl_GetFilenameFromPath(awsTestStrings[4], wcsnlen_s(awsTestStrings[4], _countof(awsTestStrings[4]))+1);
    if(!pws)
    {
        fError = TRUE;
        wprintf(L"FAILED: Path:[%s] -- Filename:[(null)]\n", awsTestStrings[4]);
    }
    else if(wcscmp(pws, L"Matrix_wallpaper_.jpg") != 0)
    {
        wprintf(L"FAILED: Path:[%s] -- Filename:[%s]\n", awsTestStrings[4], pws);
    }

    // STRING 5
    pws = Chl_GetFilenameFromPath(awsTestStrings[5], wcsnlen_s(awsTestStrings[5], _countof(awsTestStrings[5]))+1);
    if(!pws)
    {
        fError = TRUE;
        wprintf(L"FAILED: Path:[%s] -- Filename:[(null)]\n", awsTestStrings[5]);
    }
    else if(wcscmp(pws, L"ScanIPAddresses.cpp") != 0)
    {
        wprintf(L"FAILED: Path:[%s] -- Filename:[%s]\n", awsTestStrings[5], pws);
    }

    // STRING 6
    pws = Chl_GetFilenameFromPath(awsTestStrings[6], wcsnlen_s(awsTestStrings[6], _countof(awsTestStrings[6]))+1);
    if(!pws)
    {
        fError = TRUE;
        wprintf(L"FAILED: Path:[%s] -- Filename:[(null)]\n", awsTestStrings[6]);
    }
    else if(wcscmp(pws, L"ServerRM.vcproj.SHISHIR-0C6645C.Shishir.user") != 0)
    {
        wprintf(L"FAILED: Path:[%s] -- Filename:[%s]\n", awsTestStrings[6], pws);
    }

    if(!fError)
        wprintf(L"***** String Functions: All PASSED\n");

    return !fError;
}

BOOL fTestHT_NumStrRand()
{
    CHL_HTABLE *phtable = NULL;
    CHL_HT_ITERATOR itr;

    BOOL fSuccess = TRUE;

    struct _keyval stRandKeyVals[MAX_RAND_COUNT];
    unsigned int ui;

    int tableSizeKey = 2;

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
        if(!HT_fCreate(&phtable, 2, HT_KEY_DWORD, HT_VAL_DWORD))
        {
            fSuccess = FALSE;
            wprintf(L"Failed to create hash table\n");
            __leave;    
        }

        // insert all keys
        for(int i = 0; i < MAX_RAND_COUNT; ++i)
        {
            if(!HT_fInsert(phtable, &stRandKeyVals[i].dwkey, sizeof(DWORD), &stRandKeyVals[i].dwval, sizeof(DWORD)))
            {
                wprintf(L"Unable to insert %u:%d\n", stRandKeyVals[i], i);
                fSuccess = FALSE;
                stRandKeyVals[i].dwkey = -1;
                stRandKeyVals[i].dwval = -1;
            }
        }

        // find all keys
        HT_fInitIterator(&itr);
        DWORD dwkey;
        DWORD dwval;
        int keysize, valsize;
        struct _keyval retKeyVal[MAX_RAND_COUNT];

        wprintf(L"Contents of hashtable now... *********************\n\n");

        int numfound = 0;
        while(HT_fGetNext(phtable, &itr, &dwkey, &keysize, &dwval, &valsize))
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

        if(!fCompareRetrievedKeyVals(&stRandKeyVals[0], MAX_RAND_COUNT, &retKeyVal[0], numfound))
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
            if(!HT_fFind(phtable, &stRandKeyVals[i].dwkey, sizeof(DWORD), &foundval, &valsize))
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
        if(phtable && !HT_fDestroy(phtable))
            wprintf(L"Failed to destroy hash table\n");
        phtable = NULL;
    }

    return fSuccess;

}


BOOL fCompareRetrievedKeyVals(struct _keyval *pOriginal, int nOrigCount, struct _keyval *pRetrieved, int nRetCount)
{
    BOOL fAllFound = TRUE;

    BOOL fInnerFound = FALSE;
    int numNotFound = 0;

    int i, j;

    wprintf(L"Comparing two keyvals.... **************************\n");
    wprintf(L"Orig = %d :: Ret = %d\n\n", nOrigCount, nRetCount);

    // n^2 compare algorithm for now
    for(i = 0; i < nOrigCount; ++i)
    {
        fInnerFound = FALSE;
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
