#include "stdafx.h"
#include "LinkedList.h"

#include "CppUnitTest.h"
#include "Helpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
TEST_CLASS(LinkedListUnitTests)
{
public:
    TEST_METHOD(CreateAndDestroy);
    TEST_METHOD(FunctionPointers);
    TEST_METHOD(Iteration_Find);
};

void LinkedListUnitTests::CreateAndDestroy()
{
    static int s_listSizes[] = { 1, 4, 16, 33, 78, 9999, 354972 };

    int itrListSize = 0;
    for (int vt = CHL_VT_START + 1; vt < CHL_VT_END; ++vt)
    {
        PCHL_LLIST pList;
        auto nEstEntries = s_listSizes[itrListSize++ % ARRAYSIZE(s_listSizes)];
        logInfo(L"Create linkedlist: VT = %d, #entries = %d", vt, s_listSizes);
        Assert::IsTrue(SUCCEEDED(CHL_DsCreateLL(&pList, (CHL_VALTYPE)vt, nEstEntries)));
        Assert::IsTrue(SUCCEEDED(pList->Destroy(pList)), L"Destroying empty linkedlist succeeds");
    }
}

void LinkedListUnitTests::FunctionPointers()
{
    PCHL_LLIST pList;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateLL(&pList, CHL_VT_UINT32, 100)));
    Assert::IsNotNull((PVOID)pList->Destroy);
    Assert::IsNotNull((PVOID)pList->Find);
    Assert::IsNotNull((PVOID)pList->FindItr);
    Assert::IsNotNull((PVOID)pList->InitIterator);
    Assert::IsNotNull((PVOID)pList->Insert);
    Assert::IsNotNull((PVOID)pList->IsEmpty);
    Assert::IsNotNull((PVOID)pList->Peek);
    Assert::IsNotNull((PVOID)pList->Remove);
    Assert::IsNotNull((PVOID)pList->RemoveAt);
    Assert::IsNotNull((PVOID)pList->RemoveAtItr);
    Assert::IsTrue(SUCCEEDED(CHL_DsDestroyLL(pList)));
}

void LinkedListUnitTests::Iteration_Find()
{
    static int s_values[] = { 0, 10, 167, 490854, -1, -2, -554324 };

    PCHL_LLIST pList;
    Assert::IsTrue(SUCCEEDED(CHL_DsCreateLL(&pList, CHL_VT_INT32, 10)));

    for (int i = 0; i < ARRAYSIZE(s_values); ++i)
    {
        logDebug(L"Inserting = %d", s_values[i]);
        Assert::IsTrue(SUCCEEDED(pList->Insert(pList, (PCVOID)s_values[i], sizeof(s_values[0]))));
    }

    for (int i = ARRAYSIZE(s_values) - 1; i >= 0; --i)
    {
        logDebug(L"Finding = %d", s_values[i]);

        CHL_ITERATOR_LL itr;
        Assert::IsTrue(SUCCEEDED(pList->FindItr(pList, (PCVOID)s_values[i], NULL, &itr)));

        int val;
        int valSize = sizeof(val);
        Assert::IsTrue(SUCCEEDED(itr.GetCurrent(&itr, &val, &valSize, FALSE)));
        Assert::AreEqual(s_values[i], val);
    }

    Assert::IsTrue(SUCCEEDED(pList->Destroy(pList)));
}

}
