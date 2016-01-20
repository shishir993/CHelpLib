
#include "stdafx.h"
#include "RArray.h"

#include "CppUnitTest.h"
#include "Helpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
TEST_CLASS(ResizableArrayUnitTests)
{
public:
    TEST_METHOD(CreateAndDestroy);
    TEST_METHOD(InsertRetrieve_Int);
    TEST_METHOD(InsertRetrieve_Obj);
    TEST_METHOD(GrowOnce_Int);
    TEST_METHOD(GrowManyTimes_Int);
    TEST_METHOD(GrowManually_Obj);
    TEST_METHOD(ShrinkManually_Int);
    TEST_METHOD(ShrinkManually_Obj);
    TEST_METHOD(ShrinkManuallyNoWrites_Obj);

private:
    static WCHAR s_randomStrSource_AlphaNum[];
};

WCHAR ResizableArrayUnitTests::s_randomStrSource_AlphaNum[] = L"qwertyuiopasdfghjklzxcvbnmABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";


void ResizableArrayUnitTests::CreateAndDestroy()
{
    LOG_FUNC_ENTRY;

    CHL_RARRAY ra;
    UINT size = 10;
    UINT maxSize = 0;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateRA(&ra, CHL_VT_INT32, size, maxSize)));
    Assert::AreEqual(ra.Size(&ra), size);
    Assert::AreEqual(ra.MaxSize(&ra), maxSize);
    Assert::IsTrue(SUCCEEDED(ra.Destroy(&ra)));

    // With specified max size
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateRA(&ra, CHL_VT_INT32, size, maxSize)));
    Assert::AreEqual(ra.Size(&ra), size);
    Assert::AreEqual(ra.MaxSize(&ra), maxSize);
    Assert::IsTrue(SUCCEEDED(ra.Destroy(&ra)));

    LOG_FUNC_EXIT;
}

void ResizableArrayUnitTests::InsertRetrieve_Int()
{
    LOG_FUNC_ENTRY;

    // Create 48 random integers
    const int c_nItems = 48;
    auto spNumbers = Helpers::GenerateRandomNumbers(c_nItems);
    const auto& inputVector = *spNumbers;

    // Create the resizable array
    CHL_RARRAY ra;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateRA(&ra, CHL_VT_INT32, c_nItems, c_nItems)));

    // Insert all items
    for (int idx = 0; idx < c_nItems; ++idx)
    {
        Assert::IsTrue(SUCCEEDED(ra.Write(&ra, idx, (PCVOID)inputVector[idx], sizeof(int))));
    }

    // Read all items
    for (int idx = 0; idx < c_nItems; ++idx)
    {
        int val;
        Assert::IsTrue(SUCCEEDED(ra.Read(&ra, idx, &val, nullptr, FALSE)));
        Assert::AreEqual(inputVector[idx], val, L"Retrieved val must match inputVector value");
    }

    Assert::IsTrue(SUCCEEDED(CHL_DsDestroyRA(&ra)));

    LOG_FUNC_EXIT;
}

void ResizableArrayUnitTests::InsertRetrieve_Obj()
{
    LOG_FUNC_ENTRY;

    Helpers::TestStruct objs[] =
        {
            { 'a', 1, L"first one", MAXINT32 },
            { 'b', ~(MAXINT16), L"2nd one", MAXINT64 },
            { 'z', 384732, L"III", 0 }
        };

    const int c_nItems = ARRAYSIZE(objs);

    // Create the resizable array
    CHL_RARRAY ra;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateRA(&ra, CHL_VT_USEROBJECT, c_nItems, c_nItems)));

    // Insert all items
    for (int idx = 0; idx < c_nItems; ++idx)
    {
        Assert::IsTrue(SUCCEEDED(ra.Write(&ra, idx, &(objs[idx]), sizeof(objs[0]))));
    }

    // Read all items, get pointer only
    for (int idx = 0; idx < c_nItems; ++idx)
    {
        Helpers::TestStruct *pst;
        Assert::IsTrue(SUCCEEDED(ra.Read(&ra, idx, &pst, nullptr, TRUE)));
        Assert::IsTrue((objs[idx] == *pst), L"Retrieved obj via ptr must match input obj");
    }

    // Read all items, get object itself
    for (int idx = 0; idx < c_nItems; ++idx)
    {
        Helpers::TestStruct st;
        int bufSize = sizeof(st);
        Assert::IsTrue(SUCCEEDED(ra.Read(&ra, idx, &st, &bufSize, FALSE)));
        Assert::IsTrue((objs[idx] == st), L"Retrieved obj must match input obj");
    }

    Assert::IsTrue(SUCCEEDED(CHL_DsDestroyRA(&ra)));

    LOG_FUNC_EXIT;
}

void ResizableArrayUnitTests::GrowOnce_Int()
{
    LOG_FUNC_ENTRY;

    // Create 48 random integers
    const int c_nItems = 48;
    auto spNumbers = Helpers::GenerateRandomNumbers(c_nItems);
    const auto& inputVector = *spNumbers;

    int initSize = c_nItems / 2;

    // Create the resizable array with half of input elements initially (no max size)
    CHL_RARRAY ra;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateRA(&ra, CHL_VT_INT32, initSize, 0)));

    // Insert all items up to initial size
    logInfo(L"Inserting up to index %d", (initSize - 1));
    for (int idx = 0; idx < initSize; ++idx)
    {
        Assert::IsTrue(SUCCEEDED(ra.Write(&ra, idx, (PCVOID)inputVector[idx], sizeof(int))));
    }

    logInfo(L"Inserting at index %d, which is greater than initial size", initSize);
    Assert::IsTrue(SUCCEEDED(ra.Write(&ra, initSize, (PCVOID)inputVector[initSize], sizeof(int))));
    Assert::IsTrue((ra.Size(&ra) > (UINT)initSize), L"Array size must've increased after adding extra item");

    // Now, insert rest of values
    logInfo(L"Now, inserting from index %d", (initSize + 1));
    for (int idx = initSize + 1; idx < c_nItems; ++idx)
    {
        Assert::IsTrue(SUCCEEDED(ra.Write(&ra, idx, (PCVOID)inputVector[idx], sizeof(int))));
    }

    // Read all items
    for (int idx = 0; idx < c_nItems; ++idx)
    {
        int val;
        Assert::IsTrue(SUCCEEDED(ra.Read(&ra, idx, &val, nullptr, FALSE)));
        Assert::AreEqual(inputVector[idx], val, L"Retrieved val must match inputVector value");
    }

    Assert::IsTrue(SUCCEEDED(CHL_DsDestroyRA(&ra)));

    LOG_FUNC_EXIT;
}

void ResizableArrayUnitTests::GrowManyTimes_Int()
{
    LOG_FUNC_ENTRY;

    // Create 48 random integers
    const int c_nItems = 48;
    auto spNumbers = Helpers::GenerateRandomNumbers(c_nItems);
    const auto& inputVector = *spNumbers;

    // Create the resizable array without specifying an initial size
    CHL_RARRAY ra;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateRA(&ra, CHL_VT_INT32, 0, 0)));

    // Insert all items and verify size of array at the end
    for (int idx = 0; idx < c_nItems; ++idx)
    {
        Assert::IsTrue(SUCCEEDED(ra.Write(&ra, idx, (PCVOID)inputVector[idx], sizeof(int))));
        logInfo(L"Inserted idx %d, ra size = %u", idx, ra.Size(&ra));
    }

    // Read all items
    for (int idx = 0; idx < c_nItems; ++idx)
    {
        int val;
        Assert::IsTrue(SUCCEEDED(ra.Read(&ra, idx, &val, nullptr, FALSE)));
        Assert::AreEqual(inputVector[idx], val, L"Retrieved val must match inputVector value");
    }

    Assert::IsTrue(SUCCEEDED(CHL_DsDestroyRA(&ra)));

    LOG_FUNC_EXIT;
}

void ResizableArrayUnitTests::GrowManually_Obj()
{
    LOG_FUNC_ENTRY;

    const int c_nItems = 64;
    auto spInputStrings = Helpers::GenerateRandomStrings(c_nItems, s_randomStrSource_AlphaNum);
    Assert::AreEqual(c_nItems, (int)spInputStrings->size());

    // Create resizable array specifying an initial size of c_nItems / 4
    const int initSize = c_nItems / 4;

    auto fnReadVerifyByPtr = [](PCHL_RARRAY pra, const auto& vecStrings, int verifyUptoIdx)
        {
            logInfo(L"[] verifying upto index %d. RArray size = %u.", verifyUptoIdx, pra->Size(pra));
            for (int i = 0; i <= verifyUptoIdx; ++i)
            {
                std::wstring* pstr;
                Assert::IsTrue(SUCCEEDED(pra->Read(pra, i, &pstr, nullptr, TRUE)));
                logDebug(L"[] verifying %s == %s", vecStrings[i].c_str(), pstr->c_str());
                Assert::AreEqual(vecStrings[i], *pstr);
            }
        };

    CHL_RARRAY ra;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateRA(&ra, CHL_VT_USEROBJECT, initSize, 0)));
    Assert::AreEqual(initSize, (int)ra.Size(&ra), L"Init size of rarray matches expected size");

    // Fill up rarray up to initial size
    int idx = 0;
    logInfo(L"Inserting strings from index %d", idx);
    for (; idx < initSize; ++idx)
    {
        Assert::IsTrue(SUCCEEDED(ra.Write(&ra, idx, (PCVOID)(&(*spInputStrings)[idx]), sizeof(std::wstring))));
    }

    Assert::AreEqual(initSize, (int)ra.Size(&ra), L"Size of rarray hasn't increased");
    fnReadVerifyByPtr(&ra, *spInputStrings, (initSize - 1));

    // Resize rarray to initial size * 2
    Assert::IsTrue(SUCCEEDED(ra.Resize(&ra, (initSize * 2))));
    Assert::AreEqual((initSize * 2), (int)ra.Size(&ra), L"Size of rarray shows as increased");
    fnReadVerifyByPtr(&ra, *spInputStrings, (initSize - 1));

    // Insert up to intital size * 2
    logInfo(L"Inserting strings from index %d", idx);
    for (; idx < (initSize * 2); ++idx)
    {
        Assert::IsTrue(SUCCEEDED(ra.Write(&ra, idx, (PCVOID)(&(*spInputStrings)[idx]), sizeof(std::wstring))));
    }

    Assert::AreEqual((initSize * 2), (int)ra.Size(&ra), L"Size of rarray hasn't increased");
    fnReadVerifyByPtr(&ra, *spInputStrings, (initSize * 2 - 1));

    // Resize array to c_nItems and insert rest of elements
    Assert::IsTrue(SUCCEEDED(ra.Resize(&ra, c_nItems)));
    Assert::AreEqual(c_nItems, (int)ra.Size(&ra), L"Size of rarray shows as increased");
    fnReadVerifyByPtr(&ra, *spInputStrings, (initSize * 2 - 1));

    logInfo(L"Inserting strings from index %d", idx);
    for (; idx < c_nItems; ++idx)
    {
        Assert::IsTrue(SUCCEEDED(ra.Write(&ra, idx, (PCVOID)(&(*spInputStrings)[idx]), sizeof(std::wstring))));
    }

    Assert::AreEqual(c_nItems, (int)ra.Size(&ra), L"Size of rarray hasn't increased finally");
    
    // Intentionally leak the vector so that it doesn't get destroyed during func exit.
    // Because the string objects that it holds get destroyed each time in loop below ;-)
    const auto& vecStrings = *spInputStrings;
    spInputStrings.release();

    // Finally, verify all items by retrieving the object itself
    for (int i = 0; i < c_nItems; ++i)
    {
        std::wstring str;
        int bufSize = sizeof(std::wstring);
        Assert::IsTrue(SUCCEEDED(ra.Read(&ra, i, &str, &bufSize, FALSE)));
        Assert::AreEqual((int)sizeof(std::wstring), bufSize, L"Buffer size argument shouldn't be modified if size was sufficient");
        Assert::AreEqual(vecStrings[i], str);
    }

    Assert::IsTrue(SUCCEEDED(CHL_DsDestroyRA(&ra)));

    LOG_FUNC_EXIT;
}

void ResizableArrayUnitTests::ShrinkManually_Int()
{
    LOG_FUNC_ENTRY;

    const int c_nItems = 24;
    auto spNumVector = Helpers::GenerateRandomNumbers(c_nItems);
    Assert::AreEqual(c_nItems, (int)spNumVector->size());

    // Initial size is c_nItems
    CHL_RARRAY ra;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateRA(&ra, CHL_VT_INT32, c_nItems, 0)));
    Assert::AreEqual(c_nItems, (int)ra.Size(&ra));

    for (int idx = 0; idx < c_nItems; ++idx)
    {
        Assert::IsTrue(SUCCEEDED(ra.Write(&ra, idx, (PCVOID)(*spNumVector)[idx], sizeof(int))));
    }
    Assert::AreEqual(c_nItems, (int)ra.Size(&ra)); // size hasn't changed

    // Shrink to half size
    Assert::IsTrue(SUCCEEDED(ra.Resize(&ra, (c_nItems / 2))));
    Assert::AreEqual((c_nItems / 2), (int)ra.Size(&ra)); // size has changed

    // Verify remaining items are intact
    int readVal;
    for (int idx = 0; idx < (c_nItems / 2); ++idx)
    {
        Assert::IsTrue(SUCCEEDED(ra.Read(&ra, idx, &readVal, nullptr, FALSE)));
        Assert::AreEqual((*spNumVector)[idx], readVal, L"Retrieved val must match inputVector value");
    }

    for (int idx = (c_nItems / 2); idx < ((c_nItems / 2) + 10); ++idx)
    {
        HRESULT hr = ra.Read(&ra, idx, &readVal, nullptr, FALSE);
        auto spStr = Helpers::BuildString(128, L"Reading invalid index(%d) produces correct error", idx);
        Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_INDEX), hr, spStr.get());
    }

    Assert::IsTrue(SUCCEEDED(CHL_DsDestroyRA(&ra)));

    LOG_FUNC_EXIT;
}

void ResizableArrayUnitTests::ShrinkManually_Obj()
{
    LOG_FUNC_ENTRY;

    Helpers::TestStruct objs[] =
    {
        { 'a', 1, L"first one", MAXINT32 },
        { 'b', ~(MAXINT16), L"2nd one", MAXINT64 },
        { 'z', 384732, L"III", 0 },
        { 'e', 1, L"IV", MAXINT32 },
        { 'y', MAXBYTE, L"V", MAXINT64 },
        { 'i', 45245, L"VI", 85345346ULL }
    };

    const int c_nItems = ARRAYSIZE(objs);
    Assert::IsTrue((c_nItems % 2) == 0, L"#items is even");

    Helpers::TestStruct* objPointers[c_nItems];

    // Create the resizable array
    CHL_RARRAY ra;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateRA(&ra, CHL_VT_USEROBJECT, c_nItems, c_nItems)));

    // Insert all items
    for (int idx = 0; idx < c_nItems; ++idx)
    {
        Assert::IsTrue(SUCCEEDED(ra.Write(&ra, idx, &(objs[idx]), sizeof(objs[0]))));
    }

    // Read all items, get pointer only and save the pointers
    for (int idx = 0; idx < c_nItems; ++idx)
    {
        Helpers::TestStruct *pst;
        Assert::IsTrue(SUCCEEDED(ra.Read(&ra, idx, &pst, nullptr, TRUE)));
        Assert::IsTrue((objs[idx] == *pst), L"Retrieved obj via ptr must match input obj");
        objPointers[idx] = pst;
        logInfo(L"Read idx %d with ptr = 0x%p", idx, pst);
    }

    // Resize array to half of original size
    Assert::IsTrue(SUCCEEDED(ra.Resize(&ra, (c_nItems / 2))));

    // Now accessing the first half of array should be fine
    for (int idx = 0; idx < (c_nItems / 2); ++idx)
    {
        logInfo(L"Reading idx %d with ptr = 0x%p", idx, objPointers[idx]);
        Assert::IsTrue((objs[idx] == *(objPointers[idx])), L"Stored ptr must match input obj");
    }

    // Accessing second half must result in exception
    bool fGotException = false;
    for (int idx = (c_nItems / 2); idx < c_nItems; ++idx)
    {
        logInfo(L"Freeing idx %d with ptr = 0x%p, expecting exception", idx, objPointers[idx]);

        __try
        {
            free(objPointers[idx]);
            break;
        }
        __except(Helpers::ExceptionFilter_ExecAll(GetExceptionCode(), GetExceptionInformation()))
        {
            // expected exception, move forward
            fGotException = true;
        }
    }

    if (!fGotException)
    {
        Assert::Fail(L"Memory not freed after array resizing. Was expecting an exception.");
    }

    Assert::IsTrue(SUCCEEDED(ra.Destroy(&ra)));

    LOG_FUNC_EXIT;
}

void ResizableArrayUnitTests::ShrinkManuallyNoWrites_Obj()
{
    LOG_FUNC_ENTRY;

    Helpers::TestStruct objs[] =
    {
        { 'a', 1, L"first one", MAXINT32 },
        { 'b', ~(MAXINT16), L"2nd one", MAXINT64 },
        { 'z', 384732, L"III", 0 },
        { 'e', 1, L"IV", MAXINT32 },
        { 'y', MAXBYTE, L"V", MAXINT64 },
        { 'i', 45245, L"VI", 85345346ULL }
    };

    const int c_nItems = ARRAYSIZE(objs);
    Assert::IsTrue((c_nItems % 2) == 0, L"#items is even");

    // Create the resizable array
    CHL_RARRAY ra;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateRA(&ra, CHL_VT_USEROBJECT, c_nItems, c_nItems)));

    // Resize to half, no exceptions expected
    __try
    {
        Assert::IsTrue(SUCCEEDED(ra.Resize(&ra, (c_nItems / 2))));
    }
    __except (Helpers::ExceptionFilter_ExecAll(GetExceptionCode(), GetExceptionInformation()))
    {
        Assert::Fail(L"Exception while resizing array before any writes");
    }

    // Bring it back to original size and fill up to (c_nItems - 1)
    Assert::IsTrue(SUCCEEDED(ra.Resize(&ra, c_nItems)));
    for (int idx = 0; idx < (c_nItems - 1); ++idx)
    {
        Assert::IsTrue(SUCCEEDED(ra.Write(&ra, idx, &(objs[idx]), sizeof(objs[0]))));
    }

    // Resize to half, no exceptions expected
    __try
    {
        Assert::IsTrue(SUCCEEDED(ra.Resize(&ra, (c_nItems / 2))));
    }
    __except (Helpers::ExceptionFilter_ExecAll(GetExceptionCode(), GetExceptionInformation()))
    {
        Assert::Fail(L"Exception while resizing array after n-1 writes");
    }
    
    // Clear all remaining indices and resize to 1 (which is minimum currently)
    for (int idx = 0; idx < (c_nItems / 2); ++idx)
    {
        Assert::IsTrue(SUCCEEDED(ra.ClearAt(&ra, idx)));
    }

    __try
    {
        Assert::IsTrue(SUCCEEDED(ra.Resize(&ra, 1)));
    }
    __except (Helpers::ExceptionFilter_ExecAll(GetExceptionCode(), GetExceptionInformation()))
    {
        Assert::Fail(L"Exception while resizing to min size after clearing");
    }

    Assert::IsTrue(SUCCEEDED(ra.Destroy(&ra)));

    LOG_FUNC_EXIT;
}

} // namespace Tests
