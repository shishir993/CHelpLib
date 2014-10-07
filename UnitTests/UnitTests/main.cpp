
#include "Common.h"
#include "PerfTests.h"
#include "TestLinkedList.h"
#include "QueueTests.h"

#include "StringFunctions.h"
#include "IOFunctions.h"

#include "HashTableTests.h"

#define MAX_TIME_TESTS      10
#define MAX_RUNS_SYS_CALL   3

BOOL fTestStrings();
void rdtscBusiness();
void doSysCallTimingTests();
void rdtsc_Query();

BOOL fTestCreateFileWithSize();
void vTestFileMapping();

WCHAR* g_pszPassed = L"PASSED";
WCHAR* g_pszFailed = L"FAILED";


int main()
{
    BOOL success = TRUE;

    OutputDebugString(L"Starting unit tests on CHelpLib");

    // Test string functions
    success = fTestStrings() & success;

    // Hashtable unit tests
    success &= ExecUnitTestsHT();

    // Hashtable functional tests
    success &= ExecFuncTestsHT();

    //OutputDebugString(L"Starting perf tests on CHelpLib.dll");
    //doPerfTests();

    //doSysCallTimingTests();
    //rdtsc_Query();
    //rdtscBusiness();
    
    // Test linked list
    ExecFuncTestsLL();

    fTestCreateFileWithSize();
    vTestFileMapping();
    
    success = success & SUCCEEDED(QueueRunTests());

    wprintf(L"\nCumulative result: %s\n", CHOOSE_TEST_OUTCOME(success));
    OutputDebugString(L"\nTests done. Exiting...");
    return !success;
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

    wprintf(L"\n-----------------------------------------------------\n");
    wprintf(L"\n------------------ String Functions -----------------\n");

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

    wprintf(L"\nEND TEST SUITE: String Functions: %s\n", CHOOSE_TEST_OUTCOME(!fError));

    return !fError;
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
