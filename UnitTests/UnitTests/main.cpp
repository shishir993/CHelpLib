
#include "Common.h"
#include "PerfTests.h"
#include "TestLinkedList.h"
#include "QueueTests.h"

#include "StringFunctions.h"
#include "IOFunctions.h"

#include "HashTableTests.h"

#define MAX_RAND_COUNT      9999
#define MAX_TIME_TESTS      10
#define MAX_RUNS_SYS_CALL   3

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

BOOL fTestHT_TableSizes();
BOOL fTestHT_StrStr();
BOOL fTestHT_NumStr();
BOOL fTestStrings();
BOOL fTestHT_NumStrRand();
BOOL fCompareRetrievedKeyVals(struct _keyval *pOriginal, int nOrigCount, struct _keyval *pRetrieved, int nRetCount);
void rdtscBusiness();
void doSysCallTimingTests();
void rdtsc_Query();

BOOL fTestCreateFileWithSize();
void vTestFileMapping();

int main()
{
    BOOL success = TRUE;

    OutputDebugString(L"Starting unit tests on CHelpLib");

    // Test string functions
    success = fTestStrings() & success;

    // Hashtable unit tests
    success &= ExecUnitTestsHT();

    // Test hash table
    success = fTestHT_StrStr() & success;
    success = fTestHT_NumStr() & success;
    success = fTestHT_NumStrRand() & success;
    success = fTestHT_TableSizes();

    //OutputDebugString(L"Starting perf tests on CHelpLib.dll");
    doPerfTests();

    //doSysCallTimingTests();
    //rdtsc_Query();
    //rdtscBusiness();
    
    // Test linked list
    fTestLinkedList();

    fTestCreateFileWithSize();
    vTestFileMapping();
    
    success = success & SUCCEEDED(QueueRunTests());

    OutputDebugString(L"\nTests done. Exiting...");
    return !success;
}

BOOL fTestHT_TableSizes()
{
    int tableSize = 0;

    int entriesToCheck[] = { 50000, 100000, 700000, 1<<23, 1<<24 };

    for(int i = 0; i < _countof(entriesToCheck); ++i)
        wprintf(L"%10d %d\n", entriesToCheck[i], CHL_DsGetNearestSizeIndexHT(entriesToCheck[i]));

    return TRUE;
}


BOOL fTestHT_StrStr()
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


BOOL fTestHT_NumStr()
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


BOOL fTestStrings()
{
    BOOL fError = FALSE;

    WCHAR awsTestStrings[][MAX_PATH+1] = { 
        L"", L"z", L"zy", L"dbg.dll", 
        L"C:\\Users\\Shishir_Limited\\Downloads\\Matrix_wallpaper_.jpg",
        L"S:\\MyBriefcase\\Coding\\C\\Remote Monitoring - Internship at RFL\\RM\\Code\\Remote Monitoring - To RFL\\Source Code\\ServerRM\\ServerRM\\ScanIPAddresses.cpp",
        L"ServerRM.vcproj.SHISHIR-0C6645C.Shishir.user" };

    PCWSTR pws = NULL;

    if(CHL_SzGetFilenameFromPath(NULL, 412))
    {
        fError = TRUE;
        wprintf(L"FAILED: Returned something for NULL!\n");
    }

    // STRING 0
    pws = CHL_SzGetFilenameFromPath(awsTestStrings[0], wcsnlen_s(awsTestStrings[0], _countof(awsTestStrings[0]))+1);
    if(pws)
    {
        fError = TRUE;
        wprintf(L"FAILED: Path:[%s] -- Filename:[%s]\n", awsTestStrings[0], pws);
    }

    // STRING 1
    pws = CHL_SzGetFilenameFromPath(awsTestStrings[1], wcsnlen_s(awsTestStrings[1], _countof(awsTestStrings[1]))+1);
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
    pws = CHL_SzGetFilenameFromPath(awsTestStrings[2], wcsnlen_s(awsTestStrings[2], _countof(awsTestStrings[2]))+1);
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
    pws = CHL_SzGetFilenameFromPath(awsTestStrings[3], wcsnlen_s(awsTestStrings[3], _countof(awsTestStrings[3]))+1);
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
    pws = CHL_SzGetFilenameFromPath(awsTestStrings[4], wcsnlen_s(awsTestStrings[4], _countof(awsTestStrings[4]))+1);
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
    pws = CHL_SzGetFilenameFromPath(awsTestStrings[5], wcsnlen_s(awsTestStrings[5], _countof(awsTestStrings[5]))+1);
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
    pws = CHL_SzGetFilenameFromPath(awsTestStrings[6], wcsnlen_s(awsTestStrings[6], _countof(awsTestStrings[6]))+1);
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
        struct _keyval retKeyVal[MAX_RAND_COUNT];

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


void rdtscBusiness()
{
    LONGLONG ts, te, tdiff[MAX_TIME_TESTS];
    LONGLONG tavg = 0;

    int nTests = 0;
    int nOuterTests  = 0;
    register int i = 0;

    CONTEXT thContext;

    // reset some variables
    __asm
    {
        mov nTests, 0
        mov ecx, MAX_TIME_TESTS
        xor eax, eax
        lea edi, tdiff
        rep stos
    }

    while(nTests < MAX_TIME_TESTS)
    {
        ts = te = 0;
    
        // ** playing with rdtsc **
        __asm
        {
            rdtsc
            mov dword ptr [ts+4], edx
            mov dword ptr [ts], eax
        }

        // perform a system call
        GetThreadContext(GetCurrentThread(), &thContext);

        __asm
        {
            rdtsc
            mov dword ptr [te+4], edx
            mov dword ptr [te], eax
        }

        tdiff[nTests++] = te - ts;
    }

    nTests = 0;
    while(nTests < MAX_TIME_TESTS)
    {
        tavg += tdiff[nTests];
        wprintf(L"Elapsed counts: %I64u\n", tdiff[nTests]);
        ++nTests;
    }

    tavg = tavg/MAX_TIME_TESTS;
    wprintf(L"Elapsed Average: %I64u\n", tavg);

    return;
}


void doSysCallTimingTests()
{
    LARGE_INTEGER freq = {0};
    LARGE_INTEGER counterStart = {0};
    LARGE_INTEGER counterEnd = {0};

    LONGLONG elapsedCounts = 0;
    double elapsedTime[10] = {0};

    int numRuns = 0;
    int sizes[MAX_RUNS_SYS_CALL] = { 500, 50000, 100000 };

    //CONTEXT thContext;
    HANDLE hCurThread;

    DWORD dwError = 0;
    BOOL testSuccess[MAX_RUNS_SYS_CALL] = {TRUE};

    // This retrieves the counts per second
    if(!QueryPerformanceFrequency(&freq))
    {
        dwError = GetLastError();
        wprintf(L"testHashtable(): QueryPerformanceFrequency() failed %d\n", dwError);
        return;
    }

    while(numRuns < MAX_RUNS_SYS_CALL)
    {
        wprintf(L"Beginning run %d\n", numRuns);

        // Begin counter
        if(!QueryPerformanceCounter(&counterStart))
        {
            dwError = GetLastError();
            wprintf(L"testHashtable(): QueryPerformanceCounter() failed %d\n", dwError);
            return;
        }

        //
        // Code to test
        //
        for(int i = 0; i < sizes[numRuns]; ++i)
            hCurThread = GetCurrentThread();

        // End counter
        if(!QueryPerformanceCounter(&counterEnd))
        {
            dwError = GetLastError();
            wprintf(L"testHashtable(): QueryPerformanceCounter() failed %d\n", dwError);
            return;
        }

        // Get the elapsed time
        elapsedCounts = counterEnd.QuadPart - counterStart.QuadPart;
        elapsedTime[numRuns] = (double)(elapsedCounts / (double)freq.QuadPart);
        ++numRuns;
    }

    wprintf(L"Performance counter ticks %I64u times per second\n", freq.QuadPart);
    wprintf(L"Resolution is %lf nanoseconds\n", (1.0/(double)freq.QuadPart)*1e9);

    wprintf(L"%16s %13s %s\n---------------------------------------------------------\n",
        L"RunSize", L"TimeSeconds", L"TimeMicro");
    for(numRuns = 0; numRuns < MAX_RUNS_SYS_CALL; ++numRuns)
        wprintf(L"%16d %5.8lf %16.3lf %s\n", sizes[numRuns], elapsedTime[numRuns], elapsedTime[numRuns] * 1e6);

    return;
}

void rdtsc_Query()
{
    LONGLONG rdtsc1;
    LARGE_INTEGER query1;
    LONGLONG diff;

    LONG temp;

    __asm
    {
        rdtsc
        mov dword ptr [rdtsc1], eax
        mov dword ptr [rdtsc1+4], edx
    }

    if(!QueryPerformanceCounter(&query1))
    {
        wprintf(L"QueryPerformanceCounter() failed: %d\n", GetLastError());
        return;
    }

    __asm
    {
        mov eax, dword ptr [rdtsc1]
        mov edx, dword ptr [query1]
        sub eax, edx
        mov dword ptr [temp], eax
    }

    abs(temp);

    __asm mov dword ptr [diff], eax

    __asm
    {
        mov eax, dword ptr [rdtsc1+4]
        mov edx, dword ptr [query1+4]
        sub eax, edx
        mov dword ptr [temp], eax
    }

    abs(temp);

    __asm mov dword ptr [diff+4], eax

    return;
}

BOOL fTestCreateFileWithSize()
{
    static WCHAR aszFilenames[][32] = { L"file1.tmp", L"file2.tmp", L"file3.tmp" };
    static int aiFileSizes[] = { 0, 
                                 512, 
                                 1024 * 1024 // 1 MB
                               };

    HANDLE hFile = 0;
    DWORD dwFileSizeLow, dwFileSizeHigh;

    int nSampleCounts = _countof(aiFileSizes);

    ASSERT(_countof(aiFileSizes) == _countof(aszFilenames));

    int index;

    BOOL fRetVal = TRUE;

    wprintf(L"*************\nBegin TEST fChlIoCreateFileWithSize()\n");

    for(index = 0; index < nSampleCounts; ++index)
    {
        if(FAILED(CHL_IoCreateFileWithSize(&hFile, aszFilenames[index], aiFileSizes[index])))
        {
            fRetVal = FALSE;
            wprintf(L"FAILED: Create %s %d. Error = %u\n", aszFilenames[index], aiFileSizes[index], GetLastError());
            continue;
        }

        // Verify file handle
        if(!ISVALID_HANDLE(hFile))
        {
            fRetVal = FALSE;
            wprintf(L"FAILED: Invalid handle %s %d. Error = %u\n", aszFilenames[index], aiFileSizes[index], GetLastError());
            continue;
        }

        // Verify file size
        // Close handle to make sure file contents are flushed to disk
        CloseHandle(hFile);

        hFile = NULL;

        // Open it again and get file size
        hFile = CreateFile(aszFilenames[index], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile == INVALID_HANDLE_VALUE)
        {
            fRetVal = FALSE;
            wprintf(L"FAILED: CreateFile() failed %u\n", GetLastError());
            goto end_of_for;
        }

        dwFileSizeLow = GetFileSize(hFile, &dwFileSizeHigh);

        if(dwFileSizeHigh != 0)
        {
            fRetVal = FALSE;
            wprintf(L"FAILED: HighPart = %u, LowPart = %u. %u\n", dwFileSizeHigh, dwFileSizeLow, GetLastError());
        }
        else if(dwFileSizeLow != aiFileSizes[index])
        {
            fRetVal = FALSE;
            wprintf(L"FAILED: HighPart = %u, LowPart = %u. Expected %d. %u\n", dwFileSizeHigh, dwFileSizeLow, aiFileSizes[index], GetLastError());
        }

        wprintf(L"PASSED: %d: %s %d\n", index, aszFilenames[index], aiFileSizes[index]);

end_of_for:
        if(hFile) 
        {
            CloseHandle(hFile);
            DeleteFile(aszFilenames[index]);
        }

        hFile = NULL;

        dwFileSizeHigh = dwFileSizeLow = 0;
    }

    return fRetVal;
}

void vTestFileMapping()
{
    HANDLE hFile = NULL;
    HANDLE hMapObj = NULL;
    HANDLE hView = NULL;

    int iSize = 512;
    int index;

    BYTE abData[4097];

    if(FAILED(CHL_IoCreateFileWithSize(&hFile, L"file1.tmp", iSize)))
    {
        wprintf(L"FAILED: Create. Error = %u\n", GetLastError());
        return;
    }

    // Map it
    if( (hMapObj = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, L"dfajdsfk")) == NULL )
	{
        wprintf(L"FAILED: CreateFileMapping. Error = %u\n", GetLastError());
		goto end_of_func;
	}

	// check for ERROR_ALREADY_EXISTS ??

	// map this file mapping object into our address space
	if( (hView = MapViewOfFile(hMapObj, FILE_MAP_WRITE, 0, 0, 0)) == NULL )
	{
        wprintf(L"FAILED: MapViewOfFile. Error = %u\n", GetLastError());
		goto end_of_func;
	}

    // test write
    PBYTE pb = (PBYTE)hView;

    wprintf(L"Base address: %p\n", pb);

    __try
    {
        for(index = 0; index < _countof(abData); ++index)
        {
            *pb++ = abData[index];
            wprintf(L"Succes at %d %p\n", index, pb);
        }
    }
    __except(0, EXCEPTION_EXECUTE_HANDLER)
    {
        wprintf(L"FAILED at %d %p\n", index, pb);
    }

    MEMORY_BASIC_INFORMATION memBasic = {0};
    if(VirtualQuery(hView, &memBasic, sizeof(MEMORY_BASIC_INFORMATION)) == 0)
    {
        wprintf(L"FAILED: VirtualQuery %u\n", GetLastError());
    }
    else
    {
        wprintf(L"0x%p %u 0x%08x 0x%08x 0x%08x\n",
            memBasic.BaseAddress,
            memBasic.RegionSize,
            memBasic.State,
            memBasic.Protect,
            memBasic.Type
            );
    }

end_of_func:
    if(hView)
    {
        UnmapViewOfFile(hView);
    }

    if(hMapObj)
    {
        CloseHandle(hMapObj);
    }

    if(hFile)
    {
        CloseHandle(hFile);
    }

    return;
}
