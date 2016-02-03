
#include "stdafx.h"
#include "BinarySearchTree.h"

#include "CppUnitTest.h"
#include "Helpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
TEST_CLASS(BSTUnitTests)
{
public:
    TEST_METHOD(CreateAndDestroy);
    TEST_METHOD(SimpleInserts_Ints);
    TEST_METHOD(FindMinMax_Ints);
    TEST_METHOD(FindFloorCeil_Ints);
    TEST_METHOD(SimpleInsertFind_StrInt);

    // TODO: Change HRESULT verification from IsTrue to AreEqual
};

void BSTUnitTests::CreateAndDestroy()
{
    LOG_FUNC_ENTRY;

    CHL_BSTREE bst;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateBST(&bst, CHL_KT_INT32, CHL_VT_INT32, Helpers::CompareFn_Int32, FALSE)));

    PVOID pv = bst.Create; Assert::IsNotNull(pv);
    pv = bst.Destroy; Assert::IsNotNull(pv);
    pv = bst.Insert; Assert::IsNotNull(pv);
    pv = bst.Find; Assert::IsNotNull(pv);
    pv = bst.FindMax; Assert::IsNotNull(pv);
    pv = bst.FindMin; Assert::IsNotNull(pv);
    pv = bst.FindFloor; Assert::IsNotNull(pv);
    pv = bst.FindCeil; Assert::IsNotNull(pv);
    pv = bst.InitIterator; Assert::IsNotNull(pv);
    pv = bst.GetNext; Assert::IsNotNull(pv);

    Assert::IsTrue(SUCCEEDED(bst.Destroy(&bst)));

    LOG_FUNC_EXIT;
}

void BSTUnitTests::SimpleInserts_Ints()
{
    LOG_FUNC_ENTRY;

    CHL_BSTREE bst;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateBST(&bst, CHL_KT_INT32, CHL_VT_INT32, Helpers::CompareFn_Int32, FALSE)));

    const int c_nItems = 5;
    for (int i = 0; i < c_nItems; ++i)
    {
        Assert::IsTrue(SUCCEEDED(bst.Insert(&bst, (PCVOID)i, sizeof(i), (PCVOID)i, sizeof(i))));
    }

    Assert::IsTrue(SUCCEEDED(bst.Destroy(&bst)));

    LOG_FUNC_EXIT;
}

void BSTUnitTests::FindMinMax_Ints()
{
    LOG_FUNC_ENTRY;

    {
        // First, min and max with keys inserted in increasing order
        const int c_nItems = 5;

        CHL_BSTREE bst;
        Assert::IsTrue(SUCCEEDED(CHL_DsCreateBST(&bst, CHL_KT_INT32, CHL_VT_INT32, Helpers::CompareFn_Int32, FALSE)));

        logInfo(L"Inserting integer keys in increasing order");
        for (int i = 0; i < c_nItems; ++i)
        {
            Assert::IsTrue(SUCCEEDED(bst.Insert(&bst, (PCVOID)i, sizeof(i), (PCVOID)i, sizeof(i))));
        }

        int actualMin, actualMax;
        Assert::AreEqual(S_OK, bst.FindMin(&bst, &actualMin, nullptr, FALSE));
        Assert::AreEqual(0, actualMin, L"Expected min key");
        Assert::AreEqual(S_OK, bst.FindMax(&bst, &actualMax, nullptr, FALSE));
        Assert::AreEqual((c_nItems - 1), actualMax, L"Expected max key");

        Assert::IsTrue(SUCCEEDED(bst.Destroy(&bst)));
    }

    {
        // Now, insert in decreasing order and find min and max
        const int c_nItems = 5;

        CHL_BSTREE bst;
        Assert::IsTrue(SUCCEEDED(CHL_DsCreateBST(&bst, CHL_KT_INT32, CHL_VT_INT32, Helpers::CompareFn_Int32, FALSE)));

        logInfo(L"Inserting integer keys in decreasing order");
        for (int i = c_nItems - 1; i >= 0; --i)
        {
            Assert::IsTrue(SUCCEEDED(bst.Insert(&bst, (PCVOID)i, sizeof(i), (PCVOID)i, sizeof(i))));
        }

        int actualMin, actualMax;
        Assert::AreEqual(S_OK, bst.FindMin(&bst, &actualMin, nullptr, FALSE));
        Assert::AreEqual(0, actualMin, L"Expected min key");
        Assert::AreEqual(S_OK, bst.FindMax(&bst, &actualMax, nullptr, FALSE));
        Assert::AreEqual((c_nItems - 1), actualMax, L"Expected max key");

        Assert::IsTrue(SUCCEEDED(bst.Destroy(&bst)));
    }

    {
        // Now, generate random numbers and insert in random order, find min and max
        const int c_nRandomItems = 20;
        auto spRandomKeys = Helpers::GenerateRandomNumbers(c_nRandomItems);
        auto& keysVec = *spRandomKeys;

        CHL_BSTREE bst;
        Assert::IsTrue(SUCCEEDED(CHL_DsCreateBST(&bst, CHL_KT_INT32, CHL_VT_INT32, Helpers::CompareFn_Int32, FALSE)));

        logInfo(L"Inserting integer keys in random order");
        for (int i = 0; i < c_nRandomItems; ++i)
        {
            logDebug(L"Inserting key : %d", keysVec[i]);
            Assert::IsTrue(SUCCEEDED(bst.Insert(&bst, (PCVOID)keysVec[i], sizeof(keysVec[i]), (PCVOID)i, sizeof(i))));
        }

        // Calculate expected min and max keys
        auto expectedMin = keysVec[0];
        auto expectedMax = keysVec[0];
        for (auto& curKey : keysVec)
        {
            if (curKey < expectedMin)
            {
                expectedMin = curKey;
            }
            else if (curKey > expectedMax)
            {
                expectedMax = curKey;
            }
        }

        logDebug(L"Expected min : %d, max : %d", expectedMin, expectedMax);

        int actualMin, actualMax;
        Assert::AreEqual(S_OK, bst.FindMin(&bst, &actualMin, nullptr, FALSE));
        Assert::AreEqual(expectedMin, actualMin, L"Expected min key");
        Assert::AreEqual(S_OK, bst.FindMax(&bst, &actualMax, nullptr, FALSE));
        Assert::AreEqual(expectedMax, actualMax, L"Expected max key");

        Assert::IsTrue(SUCCEEDED(bst.Destroy(&bst)));
    }

    LOG_FUNC_EXIT;
}

void BSTUnitTests::FindFloorCeil_Ints()
{
    LOG_FUNC_ENTRY;

    std::vector<int> keysVec({ 10, 5, 1, 7, 21, 31, 16, 11, 9 });
    const int c_nItems = keysVec.size();

    std::vector<int> findFloorCeilOfKeys(   { 12, 8, 20, 1, 7 });
    std::vector<int> expectedFloors(        { 11, 7, 16, 1, 7 });
    std::vector<int> expectedCeils(         { 16, 9, 21, 1, 7 });
    const int c_nFloorsCeils = static_cast<int>(findFloorCeilOfKeys.size());

    Assert::AreEqual(findFloorCeilOfKeys.size(), expectedFloors.size(), L"Equal # of floors and expected answers");

    CHL_BSTREE bst;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateBST(&bst, CHL_KT_INT32, CHL_VT_INT32, Helpers::CompareFn_Int32, FALSE)));

    for (const auto& key : keysVec)
    {
        logDebug(L"Inserting %d : %d", key, key);
        Assert::AreEqual(S_OK, bst.Insert(&bst, (PCVOID)key, sizeof(key), (PCVOID)key, sizeof(key)));
    }

    for (int i = 0; i < c_nFloorsCeils; ++i)
    {
        logInfo(L"Expect floor(%d) = %d", findFloorCeilOfKeys[i], expectedFloors[i]);

        int actualFloor;
        Assert::AreEqual(S_OK, bst.FindFloor(&bst, (PCVOID)findFloorCeilOfKeys[i], sizeof(int),
            &actualFloor, nullptr, FALSE));
        Assert::AreEqual(expectedFloors[i], actualFloor);
    }

    for (int i = 0; i < c_nFloorsCeils; ++i)
    {
        logInfo(L"Expect ceil(%d) = %d", findFloorCeilOfKeys[i], expectedCeils[i]);

        int actualCeil;
        Assert::AreEqual(S_OK, bst.FindCeil(&bst, (PCVOID)findFloorCeilOfKeys[i], sizeof(int),
                                             &actualCeil, nullptr, FALSE));
        Assert::AreEqual(expectedCeils[i], actualCeil);
    }

    // Finding a non-existent floor
    Assert::AreEqual(E_NOT_SET, bst.FindFloor(&bst, (PCVOID)0, sizeof(0), nullptr, nullptr, FALSE));

    // Finding a non-existent ceil
    Assert::AreEqual(E_NOT_SET, bst.FindCeil(&bst, (PCVOID)99, sizeof(99), nullptr, nullptr, FALSE));

    Assert::IsTrue(SUCCEEDED(bst.Destroy(&bst)));

    LOG_FUNC_EXIT;
}

void BSTUnitTests::SimpleInsertFind_StrInt()
{
    LOG_FUNC_ENTRY;

    CHL_BSTREE bst;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateBST(&bst, CHL_KT_WSTRING, CHL_VT_INT32, Helpers::CompareFn_WString, FALSE)));

    const int c_nItems = 5;
    auto spKeysVec = Helpers::GenerateRandomStrings(c_nItems, Helpers::s_randomStrSource_AlphaNum);
    const auto& keysVec = *spKeysVec;
    
    for (int i = 0; i < c_nItems; ++i)
    {
        Assert::IsTrue(SUCCEEDED(bst.Insert(&bst, (PCVOID)keysVec[i].c_str(), 0, (PCVOID)i, sizeof(i))));
    }

    // Find all keys in reverse
    for (int i = c_nItems - 1; i >= 0; --i)
    {
        logInfo(L"Finding key %s", keysVec[i].c_str());

        int val;
        Assert::IsTrue(SUCCEEDED(bst.Find(&bst, (PCVOID)keysVec[i].c_str(), 0, &val, NULL, FALSE)));
        Assert::AreEqual(i, val, L"Expect found value to match expected value");
    }

    Assert::IsTrue(SUCCEEDED(bst.Destroy(&bst)));

    LOG_FUNC_EXIT;
}

}
