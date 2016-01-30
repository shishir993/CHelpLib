
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
    TEST_METHOD(SimpleInsertFind_StrInt);

private:
    static int CompareFn_Int32(PCVOID pvLeft, PCVOID pvRight);
    static int CompareFn_WString(PCVOID pvLeft, PCVOID pvRight);
};

void BSTUnitTests::CreateAndDestroy()
{
    LOG_FUNC_ENTRY;

    CHL_BSTREE bst;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateBST(&bst, CHL_KT_INT32, CHL_VT_INT32, CompareFn_Int32, FALSE)));

    PVOID pv = bst.Create; Assert::IsNotNull(pv);
    pv = bst.Destroy; Assert::IsNotNull(pv);
    pv = bst.Insert; Assert::IsNotNull(pv);
    pv = bst.InitIterator; Assert::IsNotNull(pv);
    pv = bst.GetNext; Assert::IsNotNull(pv);

    Assert::IsTrue(SUCCEEDED(bst.Destroy(&bst)));

    LOG_FUNC_EXIT;
}

void BSTUnitTests::SimpleInserts_Ints()
{
    LOG_FUNC_ENTRY;

    CHL_BSTREE bst;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateBST(&bst, CHL_KT_INT32, CHL_VT_INT32, CompareFn_Int32, FALSE)));

    for (int i = 0; i < 5; ++i)
    {
        Assert::IsTrue(SUCCEEDED(bst.Insert(&bst, (PCVOID)i, sizeof(i), (PCVOID)i, sizeof(i))));
    }

    Assert::IsTrue(SUCCEEDED(bst.Destroy(&bst)));

    LOG_FUNC_EXIT;
}

void BSTUnitTests::SimpleInsertFind_StrInt()
{
    LOG_FUNC_ENTRY;

    CHL_BSTREE bst;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateBST(&bst, CHL_KT_WSTRING, CHL_VT_INT32, CompareFn_WString, FALSE)));

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

int BSTUnitTests::CompareFn_Int32(PCVOID pvLeft, PCVOID pvRight)
{
    int left = (int)pvLeft;
    int right = (int)pvRight;

    if (left < right)
    {
        return -1;
    }

    if (left > right)
    {
        return 1;
    }

    return 0;
}

int BSTUnitTests::CompareFn_WString(PCVOID pvLeft, PCVOID pvRight)
{
    PCWSTR pszLeft = (PCWSTR)pvLeft;
    PCWSTR pszRight = (PCWSTR)pvRight;
    return wcscmp(pszLeft, pszRight);
}

}
