
#include "stdafx.h"
#include "Hashtable.h"

#include "CppUnitTest.h"
#include "Helpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
TEST_CLASS(HashtableUnitTests)
{
public:
    TEST_METHOD(CreateAndDestroy);
    TEST_METHOD(FunctionPointers);
    TEST_METHOD(InsertFindRemove_IntUint);
    TEST_METHOD(InsertFindRemove_StrInt);
    TEST_METHOD(InsertFindRemove_WStrInt);
    TEST_METHOD(InsertFindRemove_WStrWStr);
    TEST_METHOD(InsertFindRemove_IntObj);

    TEST_METHOD(FindAfterRemove_WStrWStr);

    TEST_METHOD(Iteration_WStrInt);

private:
    struct TestStruct
    {
        char _c;
        int _i;
        PCWSTR _psz;
        ULONGLONG _ull;
        LARGE_INTEGER _li;

        TestStruct() = default;

        TestStruct(char c, int i, PCWSTR psz, ULONGLONG ull)
        {
            _c = c;
            _i = i;
            _psz = psz;
            _ull = ull;
            _li.LowPart = (DWORD)i;
            _li.HighPart = (LONG)ull;
        }

        bool operator==(const TestStruct& rhs)
        {
            return (_c == rhs._c && _i == rhs._i && _psz == rhs._psz && _ull == rhs._ull);
        }
    };

private:
    static WCHAR s_randomStrSource_AlphaNum[];
};

WCHAR HashtableUnitTests::s_randomStrSource_AlphaNum[] = L"qwertyuiopasdfghjklzxcvbnmABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";


void HashtableUnitTests::CreateAndDestroy()
{
    static int s_tableSizes[] = { 1, 4, 16, 33, 78, 9999, 354972 };

    int itrTableSize = 0;
    for (int kt = CHL_KT_START + 1; kt < CHL_KT_END; ++kt)
    {
        for (int vt = CHL_VT_START + 1; vt < CHL_VT_END; ++vt)
        {
            PCHL_HTABLE pht;
            auto nEstEntries = s_tableSizes[itrTableSize++ % ARRAYSIZE(s_tableSizes)];
            BOOL isValOnHeap = ((kt % 2) == 0);

            logInfo(L"Create hashtable: KT = %d, VT = %d, #entries = %d, isOnHeap? %d", kt, vt, nEstEntries, isValOnHeap);
            Assert::IsTrue(SUCCEEDED(CHL_DsCreateHT(&pht, nEstEntries, (CHL_KEYTYPE)kt, (CHL_VALTYPE)vt, isValOnHeap)));
            Assert::IsTrue(SUCCEEDED(pht->Destroy(pht)), L"Destroying empty hashtable succeeds");
        }
    }
}

void HashtableUnitTests::FunctionPointers()
{
    PCHL_HTABLE pht;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateHT(&pht, 100, CHL_KT_WSTRING, CHL_VT_UINT32, FALSE)));
    Assert::IsNotNull((PVOID)pht->Destroy);
    Assert::IsNotNull((PVOID)pht->Dump);
    Assert::IsNotNull((PVOID)pht->Find);
    Assert::IsNotNull((PVOID)pht->GetNext);
    Assert::IsNotNull((PVOID)pht->InitIterator);
    Assert::IsNotNull((PVOID)pht->Insert);
    Assert::IsNotNull((PVOID)pht->Remove);
    Assert::IsTrue(SUCCEEDED(CHL_DsDestroyHT(pht)));
}

void HashtableUnitTests::InsertFindRemove_IntUint()
{
    static int s_keys[] = { 0, 1, -1, MAXINT32 - 1, MAXINT32 };
    static UINT s_values[] = { 0, 1, 2, MAXUINT32 - 1, MAXUINT32 };
    static_assert(ARRAYSIZE(s_keys) == ARRAYSIZE(s_values), "#keys is equal to #values");

    // Hashtable with KT = Int, VT = UInt
    PCHL_HTABLE pht;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateHT(&pht, 10, CHL_KT_INT32, CHL_VT_UINT32, FALSE)));

    // Insert all key-value pairs
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logInfo(L"Inserting: %d = %u", s_keys[idx], s_values[idx]);
        Assert::IsTrue(SUCCEEDED(pht->Insert(pht, (PCVOID)s_keys[idx], sizeof(s_keys[0]), 
            (PVOID)s_values[idx], sizeof(s_values[0]))));
    }

    // Find all of them
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logInfo(L"Finding: %d", s_keys[idx]);
        UINT val;
        Assert::IsTrue(SUCCEEDED(pht->Find(pht, (PCVOID)s_keys[idx], sizeof(s_keys[0]), &val, nullptr, FALSE)));
        Assert::AreEqual(s_values[idx], val, L"Retrieved value must match expected value");
    }

    // Remove all of them
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logInfo(L"Removing: %d", s_keys[idx]);
        Assert::IsTrue(SUCCEEDED(pht->Remove(pht, (PCVOID)s_keys[idx], sizeof(s_keys[0]))));
    }

    Assert::IsTrue(SUCCEEDED(pht->Destroy(pht)));
}

void HashtableUnitTests::InsertFindRemove_StrInt()
{
    static char s_keys[][MAX_PATH] = { "one", "two", "third 3rd", "4th one" };
    static UINT s_values[] = { 0, 1, 2, MAXUINT32 };
    static_assert(ARRAYSIZE(s_keys) == ARRAYSIZE(s_values), "#keys is equal to #values");

    // Hashtable with KT = Str, VT = UInt
    PCHL_HTABLE pht;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateHT(&pht, 10, CHL_KT_STRING, CHL_VT_UINT32, FALSE)));

    // Insert all key-value pairs
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logInfo(L"Inserting: %S = %u", s_keys[idx], s_values[idx]);
        Assert::IsTrue(SUCCEEDED(pht->Insert(pht, (PCVOID)s_keys[idx], 0,
            (PVOID)s_values[idx], sizeof(s_values[0]))));
    }

    // Find all of them
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logInfo(L"Finding: %S", s_keys[idx]);
        UINT val;
        Assert::IsTrue(SUCCEEDED(pht->Find(pht, (PCVOID)s_keys[idx], 0, &val, nullptr, FALSE)));
        Assert::AreEqual(s_values[idx], val, L"Retrieved value must match expected value");
    }

    // Remove all of them
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logInfo(L"Removing: %S", s_keys[idx]);
        Assert::IsTrue(SUCCEEDED(pht->Remove(pht, (PCVOID)s_keys[idx], 0)));
    }

    Assert::IsTrue(SUCCEEDED(pht->Destroy(pht)));
}

void HashtableUnitTests::InsertFindRemove_WStrInt()
{
    static WCHAR s_keys[][MAX_PATH] = { L"one", L"two", L"third 3rd", L"4th one" };
    static UINT s_values[] = { 0, 1, 2, MAXUINT32 };
    static_assert(ARRAYSIZE(s_keys) == ARRAYSIZE(s_values), "#keys is equal to #values");

    // Hashtable with KT = WStr, VT = UInt
    PCHL_HTABLE pht;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateHT(&pht, 10, CHL_KT_WSTRING, CHL_VT_UINT32, FALSE)));

    // Insert all key-value pairs
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logDebug(L"Inserting: %s = %u", s_keys[idx], s_values[idx]);
        Assert::IsTrue(SUCCEEDED(pht->Insert(pht, (PCVOID)s_keys[idx], 0,
            (PVOID)s_values[idx], sizeof(s_values[0]))));
    }

    // Find all of them
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logDebug(L"Finding: %s", s_keys[idx]);
        UINT val;
        Assert::IsTrue(SUCCEEDED(pht->Find(pht, (PCVOID)s_keys[idx], 0, &val, nullptr, FALSE)));
        Assert::AreEqual(s_values[idx], val, L"Retrieved value must match expected value");
    }

    // Remove all of them
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logDebug(L"Removing: %s", s_keys[idx]);
        Assert::IsTrue(SUCCEEDED(pht->Remove(pht, (PCVOID)s_keys[idx], 0)));
    }

    Assert::IsTrue(SUCCEEDED(pht->Destroy(pht)));
}

void HashtableUnitTests::InsertFindRemove_WStrWStr()
{
    static WCHAR s_keys[][MAX_PATH] = 
        {
            L"ab",
            L"a",
            L"you",
            L"fourth",
            L"now",
            L"sixth",
            L"7th one"
        };

    static WCHAR s_values[][MAX_PATH] =
        {   L"dfaldfja",
            L"fdslfjoijfaldsfj",
            L"hello there",
            L"I am VS2012",
            L"The C Compiler",
            L"From Microsoft",
            L"where my boss works"
        };

    static_assert(ARRAYSIZE(s_keys) == ARRAYSIZE(s_values), "#keys is equal to #values");

    // Hashtable with KT = WStr, VT = WStr
    PCHL_HTABLE pht;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateHT(&pht, 10, CHL_KT_WSTRING, CHL_VT_WSTRING, FALSE)));

    // Insert all key-value pairs
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logDebug(L"Inserting: %s = %s", s_keys[idx], s_values[idx]);
        Assert::IsTrue(SUCCEEDED(pht->Insert(pht, (PCVOID)s_keys[idx], 0, (PVOID)s_values[idx], 0)));
    }

    // Find all of them (get pointer only)
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logDebug(L"Finding: %s", s_keys[idx]);
        WCHAR* psz;
        Assert::IsTrue(SUCCEEDED(pht->Find(pht, (PCVOID)s_keys[idx], 0, &psz, nullptr, TRUE)));
        Assert::AreEqual(s_values[idx], psz, L"Retrieved value must match expected value");
    }

    // Find all of them (get string value itself)
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logDebug(L"Finding: %s", s_keys[idx]);
        WCHAR sz[MAX_PATH];
        int bufSize = ARRAYSIZE(sz) * sizeof(WCHAR);
        Assert::IsTrue(SUCCEEDED(pht->Find(pht, (PCVOID)s_keys[idx], 0, sz, &bufSize, FALSE)));
        Assert::AreEqual(s_values[idx], sz, L"Retrieved value must match expected value");
    }

    // Remove all of them
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logDebug(L"Removing: %s", s_keys[idx]);
        Assert::IsTrue(SUCCEEDED(pht->Remove(pht, (PCVOID)s_keys[idx], 0)));
    }

    Assert::IsTrue(SUCCEEDED(pht->Destroy(pht)));
}

void HashtableUnitTests::InsertFindRemove_IntObj()
{
    TestStruct objs[] =
        {
            { 'a', 1, L"first one", MAXINT32 },
            { 'b', ~(MAXINT16), L"2nd one", MAXINT64 },
            { 'z', 384732, L"III", 0 }
        };

    const int c_numValues = ARRAYSIZE(objs);
    std::vector<int> keysVector((size_t)c_numValues);
    Assert::AreEqual((size_t)c_numValues, keysVector.size());

    // Generate random integers for use as keys
    srand(GetTickCount());

    for (int idx = 0; idx < c_numValues; ++idx)
    {
        keysVector[idx] = rand();
        logDebug(L"keys[%u] = %d", idx, keysVector[idx]);
    }

    // Hashtable with KT = Int, VT = UObj
    PCHL_HTABLE pht;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateHT(&pht, 10, CHL_KT_INT32, CHL_VT_USEROBJECT, FALSE)));

    // Insert the key-value pairs
    for (int idx = 0; idx < c_numValues; ++idx)
    {
        auto key = keysVector[idx];
        Assert::IsTrue(SUCCEEDED(pht->Insert(pht, (PVOID)key, sizeof(int), &objs[idx], sizeof(objs[0]))));
    }

    // Find all the keys, via pointer only
    for (int idx = 0; idx < c_numValues; ++idx)
    {
        auto key = keysVector[idx];

        TestStruct* pst;
        Assert::IsTrue(SUCCEEDED(pht->Find(pht, (PVOID)key, sizeof(int), &pst, nullptr, TRUE)));
        Assert::IsTrue((objs[idx] == *pst), L"Retrieved struct ptr matches expected struct");
    }

    // Find all the keys, via getting actual struct
    for (int idx = 0; idx < c_numValues; ++idx)
    {
        auto key = keysVector[idx];

        TestStruct st;
        int valSize = sizeof(st);
        Assert::IsTrue(SUCCEEDED(pht->Find(pht, (PVOID)key, sizeof(int), &st, &valSize, FALSE)));
        Assert::IsTrue((objs[idx] == st), L"Retrieved struct ptr matches expected struct");
    }

    // Remove all keys
    for (int idx = 0; idx < c_numValues; ++idx)
    {
        auto key = keysVector[idx];
        Assert::IsTrue(SUCCEEDED(pht->Remove(pht, (PCVOID)key, sizeof(int))));
    }

    Assert::IsTrue(SUCCEEDED(pht->Destroy(pht)));
}

void HashtableUnitTests::FindAfterRemove_WStrWStr()
{
    static WCHAR s_keys[][MAX_PATH] =
        {
            L"key1",
            L"key2"
        };

    static WCHAR s_values[][MAX_PATH] =
        { 
            L"I am VS2012",
            L"The C Compiler"
        };

    // Hashtable with KT = WStr, VT = WStr
    PCHL_HTABLE pht;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateHT(&pht, 10, CHL_KT_WSTRING, CHL_VT_WSTRING, FALSE)));

    // Insert all key-value pairs
    for (int idx = 0; idx < ARRAYSIZE(s_keys); ++idx)
    {
        logDebug(L"Inserting: %s = %s", s_keys[idx], s_values[idx]);
        Assert::IsTrue(SUCCEEDED(pht->Insert(pht, (PCVOID)s_keys[idx], 0, (PVOID)s_values[idx], 0)));
    }

    // Remove the first one
    Assert::IsTrue(SUCCEEDED(pht->Remove(pht, s_keys[0], 0)));

    // Fail to find it
    PCWSTR psz;
    HRESULT hr = pht->Find(pht, s_keys[0], 0, &psz, nullptr, TRUE);
    Assert::AreEqual(E_NOT_SET, hr, L"Finding a removed items returns E_NOT_SET");

    // Find the second item
    Assert::IsTrue(SUCCEEDED(pht->Find(pht, s_keys[1], 0, &psz, nullptr, TRUE)));
    Assert::AreEqual(s_values[1], psz, L"Retrieved value must match expected");

    Assert::IsTrue(SUCCEEDED(pht->Destroy(pht)));
}

void HashtableUnitTests::Iteration_WStrInt()
{
    const int c_nItems = 10;
    auto spKeys = Helpers::GenerateRandomStrings(c_nItems, s_randomStrSource_AlphaNum);
    auto spValues = Helpers::GenerateRandomNumbers(c_nItems);

    // Hashtable with KT = WStr, VT = Int
    PCHL_HTABLE pht;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateHT(&pht, 10, CHL_KT_WSTRING, CHL_VT_INT32, FALSE)));

    // Insert all key-value pairs
    for (int idx = 0; idx < c_nItems; ++idx)
    {
        logDebug(L"Inserting: %s = %d", (*spKeys)[idx].c_str(), (*spValues)[idx]);
        Assert::IsTrue(SUCCEEDED(pht->Insert(pht, (PCVOID)(*spKeys)[idx].c_str(), 0,
            (PVOID)(*spValues)[idx], sizeof(int))));
    }

    CHL_HT_ITERATOR htItr;
    Assert::IsTrue(SUCCEEDED(pht->InitIterator(pht, &htItr)));

    // Now iterate through the hashtable and ensure all inserted elements are found

    using KVPair = std::pair<PCWSTR, int>;
    using KVList = std::list<KVPair>;
    auto spFoundKVs(std::make_unique<KVList>());

    PCWSTR pszKey;
    int iVal;
    while (SUCCEEDED(pht->GetNext(&htItr, &pszKey, nullptr, &iVal, nullptr, TRUE)))
    {
        logDebug(L"Found: %s = %d", pszKey, iVal);
        (*spFoundKVs).push_back(std::make_pair(pszKey, iVal));
    }

    Assert::AreEqual((size_t)c_nItems, spFoundKVs->size(), L"Must've found expected #items via iteration");

    // Verify all items are present in retrieved KV pairs
    for (int idx = 0; idx < c_nItems; ++idx)
    {
        auto curPair = std::make_pair((*spKeys)[idx].c_str(), (*spValues)[idx]);
        auto cItr = Helpers::FindInList(*spFoundKVs, curPair, [](const KVPair& lhs, const KVPair& rhs) -> bool
            {
                return (wcscmp(lhs.first, rhs.first) == 0) && (rhs.second == rhs.second);
            });

        auto spStr = Helpers::BuildString(512, L"Finding: %s = %d", curPair.first, curPair.second);
        Assert::IsFalse(cItr == spFoundKVs->cend(), spStr.get());
    }

    Assert::IsTrue(SUCCEEDED(pht->Destroy(pht)));
}

}
