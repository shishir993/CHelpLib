
#include "PerfTests.h"

#define MAX_RUNS 5

void doPerfTests()
{
    LARGE_INTEGER freq = {0};
    LARGE_INTEGER counterStart = {0};
    LARGE_INTEGER counterEnd = {0};

    LONGLONG elapsedCounts = 0;
    double elapsedTime[10] = {0};

    int numRuns = 0;
    int sizes[MAX_RUNS] = { 50000, 100000, 700000, 1<<23, 1<<24 };

    DWORD dwError = 0;
    BOOL testSuccess[MAX_RUNS] = {TRUE};

    // This retrieves the counts per second
    if(!QueryPerformanceFrequency(&freq))
    {
        dwError = GetLastError();
        wprintf(L"testHashtable(): QueryPerformanceFrequency() failed %d\n", dwError);
        return;
    }

    while(numRuns < MAX_RUNS)
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
        testSuccess[numRuns] = testHashtable(CHL_KT_UINT32, CHL_VT_UINT32, sizes[numRuns], FALSE);

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

    wprintf(L"%16s %13s %19s %s\n---------------------------------------------------------------\n",
        L"RunSize", L"TimeSeconds", L"TimeMicro", L"Result");
    for(numRuns = 0; numRuns < MAX_RUNS; ++numRuns)
        wprintf(L"%16d %5.8lf %16.3lf %s\n", sizes[numRuns], elapsedTime[numRuns], elapsedTime[numRuns] * 1e6, 
                                                                                testSuccess[numRuns]?L"PASSED":L"FAILED");

    return;
}

BOOL testHashtable(CHL_KEYTYPE keytype, CHL_VALTYPE valType, int nEntries, BOOL fRandomize)
{
    switch(keytype)
    {
        case CHL_KT_STRING:
            break;

        case CHL_KT_UINT32:
            if(valType == CHL_VT_UINT32)
                return testHastable_NumNum(nEntries, fRandomize);
            break;
    }

    return FALSE;
}// testHashtable()

BOOL testHastable_NumNum(int nEntries, BOOL fRandomize)
{
    DWORD *keys = NULL;
    DWORD *vals = NULL;

    CHL_HTABLE *phtable = NULL;
    int counter;

    if(FAILED(CHL_DsCreateHT(&phtable, 10, CHL_KT_UINT32, CHL_VT_UINT32, FALSE)))
        goto error_return;

    if(fRandomize)
    {
        if((keys = (DWORD*)malloc(sizeof(DWORD) * nEntries)) == NULL)
            goto error_return;

        if((vals = (DWORD*)malloc(sizeof(DWORD) * nEntries)) == NULL)
            goto error_return;
    }
    else
    {
        for(counter = 0; counter < nEntries; ++counter)
        {
            if(FAILED(CHL_DsInsertHT(phtable, (PVOID)counter, sizeof(DWORD), (PVOID)counter, sizeof(DWORD))))
                goto error_return;
        }
    }

    CHL_DsDestroyHT(phtable);
    if(keys) free(keys);
    if(vals) free(vals);
    return TRUE;

    error_return:
    if(keys) free(keys);
    if(vals) free(vals);
    if(phtable) CHL_DsDestroyHT(phtable);
    return FALSE;
}
