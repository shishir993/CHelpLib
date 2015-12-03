
#include "stdafx.h"
#include "IOFunctions.h"

#include "CppUnitTest.h"
#include "Helpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
TEST_CLASS(IOFunctionsTests)
{
public:
    TEST_METHOD(File_SimpleCreateTest);
    TEST_METHOD(File_MemoryMappingTest);
};

void IOFunctionsTests::File_SimpleCreateTest()
{
    static WCHAR aszFilenames[][32] = { L"file1.tmp", L"file2.tmp", L"file3.tmp" };
    static int aiFileSizes[] = { 0, 512, 1024 * 1024 /*1MB*/ };

    static_assert(ARRAYSIZE(aiFileSizes) == ARRAYSIZE(aszFilenames), "File size and name array counts match");

    HANDLE hFile = 0;
    LARGE_INTEGER li{ 0 };

    for (int index = 0; index < ARRAYSIZE(aiFileSizes); ++index)
    {
        hFile = NULL;
        ZeroMemory(&li, sizeof(li));

        Assert::IsTrue(SUCCEEDED(CHL_IoCreateFileWithSize(&hFile, aszFilenames[index], aiFileSizes[index])),
            L"Simple file create succeeds");
        Assert::IsTrue(ISVALID_HANDLE(hFile), L"File creation returns a valid handle");

        // ** Verify file size **
        
        // Close handle to make sure file contents are flushed to disk
        CloseHandle(hFile);
        hFile = NULL;

        // Open it again and get file size
        hFile = CreateFile(aszFilenames[index], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        Assert::IsTrue(ISVALID_HANDLE(hFile), L"Opening the created file returns a valid handle");
        Assert::IsTrue(GetFileSizeEx(hFile, &li), L"GetFileSizeEx for created file succeeds");
        
        // Unfortunately, Assert doesn't provide specialization of CppUnitTestFramework::ToString() for INT64 type,
        // so we cast both expected and actual values to to UINT64.
        Assert::AreEqual((UINT64)aiFileSizes[index], (UINT64)li.QuadPart, L"Actual file size matches expected size");

        CloseHandle(hFile);
        DeleteFile(aszFilenames[index]);
    }
}

void IOFunctionsTests::File_MemoryMappingTest()
{
    HANDLE hFile = NULL;
    HANDLE hMapObj = NULL;
    HANDLE hView = NULL;

    int iSize = 512;
    int index;
    const int bufSize = 4097;

    BYTE abData[bufSize];

    WCHAR szFile[] = L"file1.tmp";

    // Delete file if exists, then create it with a fixed size
    if (PathFileExists(szFile))
    {
        Assert::IsTrue(DeleteFile(szFile));
    }
    Assert::IsTrue(SUCCEEDED(CHL_IoCreateFileWithSize(&hFile, szFile, iSize)));

    // Map into our address space
    hMapObj = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, L"dfajdsfk");
    Assert::IsTrue(ISVALID_HANDLE(hMapObj), L"CreateFileMapping should succeed");

    // check for ERROR_ALREADY_EXISTS ??
    
    hView = MapViewOfFile(hMapObj, FILE_MAP_WRITE, 0, 0, 0);
    Assert::IsTrue(ISVALID_HANDLE(hView), L"MapViewOfFile should succeed");

    // test write
    PBYTE pb = (PBYTE)hView;

    logDebug(L"Base address: 0x%p", pb);

    __try
    {
        for (index = 0; index < _countof(abData); ++index)
        {
            *pb++ = abData[index];
            logInfo(L"Successfully wrote at %d, %p", index, pb);
        }
    }
    __except (0, EXCEPTION_EXECUTE_HANDLER)
    {
        logWarn(L"Failed to write at %d, %p", index, pb);
    }

    Assert::AreEqual(bufSize - 1, index, L"Must've written expected amount of bytes to file");

    MEMORY_BASIC_INFORMATION memBasic = { 0 };
    Assert::IsTrue((VirtualQuery(hView, &memBasic, sizeof(MEMORY_BASIC_INFORMATION)) != 0), L"VirtualQuery succeeds");
    logInfo(L"Mapped mem area:\n"
        L"BaseAddr = 0x%p\n"
        L"Size     = %u bytes\n"
        L"State    = 0x%08x\n"
        L"Protect  = 0x%08x\n"
        L"Type     = 0x%08x",
        memBasic.BaseAddress, memBasic.RegionSize, memBasic.State, memBasic.Protect, memBasic.Type);

    Assert::IsTrue(UnmapViewOfFile(hView), L"Unmap succeeds");
    Assert::IsTrue(CloseHandle(hMapObj), L"CloseHandle for mapping object succeeds");
    Assert::IsTrue(CloseHandle(hFile), L"CloseHandle for file object succeeds");
}

}


