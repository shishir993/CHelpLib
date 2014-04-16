
#include "Common.h"
#include "TestLinkedList.h"

#define NUM_TESTS   2

typedef struct _testStruct {
    UINT ui;
    DWORD dw;
    void *pv;
    WCHAR sz[16];
}TESTSTRUCT, *PTESTSTRUCT;

// Tests
static BOOL fInsertAndFind();
static BOOL fInsertRemoveFind();
static BOOL fInsertAndFind_Internal(PTESTSTRUCT *ppTestStructs, int nElems);

////////////////////////////////////////////////////////////////////// 
// ************************ Test Helpers ************************
//
BOOL fCreatRandString(__out WCHAR *pszBuffer, __in int nBufSize)
{
    static WCHAR szSource[] = L"qwertyuiopasdfghjklzxcvbnm1234567890!@#$%^&*()";
    static int nElems = _countof(szSource);
    int iError;

    UINT uiSelector;
    
    for(int i = 0; i < nBufSize - 1; ++i)
    {
        if((iError = rand_s(&uiSelector)))
        {
            SetLastError(iError);
            return FALSE;
        }

        uiSelector = uiSelector % (nElems-1);
        pszBuffer[i] = szSource[uiSelector];
    }
    
    pszBuffer[nBufSize - 1] = 0;

    return TRUE;
}

BOOL fCreateTestStruct(__out PTESTSTRUCT *ppStruct)
{
    PTESTSTRUCT pStruct = NULL;
    DWORD dwError = ERROR_SUCCESS;
    int iError = 0;

    if(!ppStruct)
    {
        __asm int 3
    }

    if(!fChlMmAlloc((void**)&pStruct, sizeof(TESTSTRUCT), &dwError))
    {
        goto error_return;
    }

    iError = rand_s(&pStruct->ui);
    if(iError)
    {
        SetLastError(iError);
        goto error_return;
    }

    pStruct->dw = pStruct->ui >> 2;
    pStruct->pv = (void*)(pStruct->ui | 0x00401000);
    if(!fCreatRandString(pStruct->sz, _countof(pStruct->sz)))
    {
        goto error_return;
    }

    *ppStruct = pStruct;
    return TRUE;

error_return:

    if(pStruct)
    {
        free(pStruct);
    }
    return FALSE;
}

BOOL fCreateTestStructs(__in int nReqItems, __out PTESTSTRUCT **ppTestItems)
{
    PTESTSTRUCT *pplocal = NULL;
    DWORD dwError = ERROR_SUCCESS;

    PTESTSTRUCT ptemp = NULL;

    if(nReqItems <= 0)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        return FALSE;
    }

    if(!fChlMmAlloc((void**)&pplocal, sizeof(PTESTSTRUCT) * nReqItems, &dwError))
    {
        return FALSE;
    }

    for(int i = 0; i < nReqItems; ++i)
    {
        if(!fCreateTestStruct(&ptemp))
        {
            goto error_return;
        }

        pplocal[i] = ptemp;
    }

    *ppTestItems = pplocal;
    return TRUE;

error_return:
    // todo: free memory
    return FALSE;

}

BOOL fCompareTestStructs(void* pleft, void* pright)
{
    if(!pleft || !pright)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        return FALSE;
    }

    PTESTSTRUCT pstLeft = (PTESTSTRUCT)pleft;
    PTESTSTRUCT pstRight = (PTESTSTRUCT)pright;

#ifdef _DEBUG

    BOOL fMatch = (
        pstLeft->ui == pstRight->ui &&
        pstLeft->dw == pstRight->dw &&
        pstLeft->pv == pstRight->pv &&
        wcscmp(pstLeft->sz, pstRight->sz) == 0);

    return fMatch;

#else

    return (
        pstLeft->ui == pstRight->ui &&
        pstLeft->dw == pstRight->dw &&
        pstLeft->pv == pstRight->pv &&
        wcscmp(pstLeft->sz, pstRight->sz) == 0);
#endif
}

void vDumpTestStruct(PTESTSTRUCT pst)
{
    ASSERT(pst);

    wprintf(L"%11u %11u 0x%08x %s", pst->ui, pst->dw, pst->pv, pst->sz);
}

void vFreePointerList(PTESTSTRUCT *ppTestStructs, int nElems)
{
    ASSERT(ppTestStructs);
    ASSERT(nElems > 0);

    for(int i = 0; i < nElems; ++i)
    {
        if(ppTestStructs[i])
        {
            free(ppTestStructs[i]);
        }
    }

    free(ppTestStructs);

}

////////////////////////////////////////////////////////////////////// 
// ************************ Test Functions ************************

//
BOOL fTestLinkedList()
{
    int nTotalTests = NUM_TESTS;
    int nPassed = 0;

    if(fInsertAndFind())
    {
        ++nPassed;
    }

    
    if(fInsertRemoveFind())
    {
        ++nPassed;
    }

    wprintf(L"TEST Linked List: Total = %d, Passed = %d\n", nTotalTests, nPassed);

    return nPassed == nTotalTests;
}

// Basic insert and find test.
// - Insert 1, 2, 8 elements and find all.
// : All inserted elements must be found
static BOOL fInsertAndFind()
{
    PTESTSTRUCT *ppTestStructs = NULL;
    BOOL fResult = TRUE;

    int nCurrentElems;

    wprintf(L"\nSTART %s\n", __FUNCTIONW__);

    // First, one element
    nCurrentElems = 1;
    if(!fCreateTestStructs(nCurrentElems, &ppTestStructs))
    {
        wprintf(L"!!!! Cannot create test data %u\n", GetLastError());
        goto error_return;
    }

    fResult &= fInsertAndFind_Internal(ppTestStructs, nCurrentElems);
    vFreePointerList(ppTestStructs, nCurrentElems);

    // Two elements
    nCurrentElems = 2;
    if(!fCreateTestStructs(nCurrentElems, &ppTestStructs))
    {
        wprintf(L"!!!! Cannot create test data %u\n", GetLastError());
        goto error_return;
    }

    fResult &= fInsertAndFind_Internal(ppTestStructs, nCurrentElems);
    vFreePointerList(ppTestStructs, nCurrentElems);

    // Eight elements
    nCurrentElems = 8;
    if(!fCreateTestStructs(nCurrentElems, &ppTestStructs))
    {
        wprintf(L"!!!! Cannot create test data %u\n", GetLastError());
        goto error_return;
    }

    fResult &= fInsertAndFind_Internal(ppTestStructs, nCurrentElems);
    vFreePointerList(ppTestStructs, nCurrentElems);

    wprintf(L"\nEND %s\n", __FUNCTIONW__);
    return fResult;

error_return:
    wprintf(L"\nEND %s\n", __FUNCTIONW__);
    return FALSE;
}

// Insert, remove and try to find the removed and other elements.
// 
static BOOL fInsertRemoveFind()
{
    int index;
    PTESTSTRUCT *ppTestStructs = NULL;
    PTESTSTRUCT pRetrievedTestData = NULL;

    PCHL_LLIST pLList = NULL;

    int numTestData = 4;

    wprintf(L"\nSTART %s\n", __FUNCTIONW__);

    // Create 4 test structs
    wprintf(L"Creating test structs\n");
    if(!fCreateTestStructs(numTestData, &ppTestStructs))
    {
        wprintf(L"!!!! Cannot create test data %u\n", GetLastError());
        goto test_failed;
    }

    // Create the linked list
    wprintf(L"Creating linked list\n");
    if(!fChlDsCreateLL(&pLList, LL_VAL_PTR, numTestData))
    {
        wprintf(L"!!!! Cannot create linked list(size = %d): Error %u\n", numTestData, GetLastError());
        goto test_failed;
    }

    // Insert all four
    for(index = 0; index < numTestData; ++index)
    {
        wprintf(L"Inserting element %d\n", index);
        if(!fChlDsInsertLL(pLList, ppTestStructs[index], sizeof(TESTSTRUCT)))
        {
            wprintf(L"!!!! Could not insert element %d\n", index);
            goto test_failed;
        }
    }

    // Remove the last inserted, i.e., numTestData - 1 index
    wprintf(L"Removing at %d\n", numTestData-1);
    if(!fChlDsRemoveAtLL(pLList, numTestData-1, (void**)&pRetrievedTestData))
    {
        wprintf(L"!!!! Could not remove at %d\n", numTestData-1);
        goto test_failed;
    }

    // Compare retrieved data
    if(!fCompareTestStructs(pRetrievedTestData, ppTestStructs[numTestData-1]))
    {
        wprintf(L"!!!! Removed test data(atLL) NO MATCH: \n");
        wprintf(L"Retrieved: ");
        vDumpTestStruct(pRetrievedTestData);
        wprintf(L"\nExpected: ");
        vDumpTestStruct(ppTestStructs[numTestData-1]);
        wprintf(L"\n");
    }

    wprintf(L"MATCH retrieved struct\n");
    vChlMmFree((void**)&pRetrievedTestData);

    // Find the first three
    wprintf(L"Finding after removing at end...\n");
    for(index = 0; index < numTestData - 1; ++index)
    {
        if(!fChlDsFindLL(pLList, ppTestStructs[index], fCompareTestStructs, (void**)&pRetrievedTestData))
        {
            wprintf(L"!!!! Could not find element at %d after removing at %d\n", index, numTestData - 1);
            goto test_failed;
        }

        if(!fCompareTestStructs(ppTestStructs[index], pRetrievedTestData))
        {
            wprintf(L"!!!! Found NO MATCH %d: ", index);
            vDumpTestStruct(ppTestStructs[index]);
            wprintf(L"\n");
        }
    }

    // Add the fourth one again
    wprintf(L"Adding the last one again...\n");
    if(!fChlDsInsertLL(pLList, ppTestStructs[numTestData-1], sizeof(TESTSTRUCT)))
    {
        wprintf(L"!!!! Could not re-insert element %d\n", numTestData-1);
        goto test_failed;
    }

    // Remove using the other remove function
    wprintf(L"Remove the last one using *RemoveLL() function\n");
    if(!fChlDsRemoveLL(pLList, ppTestStructs[numTestData-1], TRUE, fCompareTestStructs))
    {
        wprintf(L"!!!! Could not remove at %d using RemoveLL()\n", numTestData-1);
        goto test_failed;
    }

    // Find first three, again
    for(index = 0; index < numTestData - 1; ++index)
    {
        wprintf(L"Finding element %d\n", index);
        if(!fChlDsFindLL(pLList, ppTestStructs[index], fCompareTestStructs, (void**)&pRetrievedTestData))
        {
            wprintf(L"!!!! Could not find element at %d after removing at %d\n", index, numTestData - 1);
            goto test_failed;
        }

        if(!fCompareTestStructs(ppTestStructs[index], pRetrievedTestData))
        {
            wprintf(L"!!!! Found NO MATCH %d: ", index);
            vDumpTestStruct(ppTestStructs[index]);
            wprintf(L"\n");
        }
    }

    // Remove first one inserted
    wprintf(L"Removing first one\n");
    if(!fChlDsRemoveAtLL(pLList, 0, (void**)&pRetrievedTestData))
    {
        wprintf(L"!!!! Could not remove at 0\n");
        goto test_failed;
    }

    // Compare retrieved data
    if(!fCompareTestStructs(pRetrievedTestData, ppTestStructs[0]))
    {
        wprintf(L"!!!! Removed test data NO MATCH: \n");
        wprintf(L"Retrieved: ");
        vDumpTestStruct(pRetrievedTestData);
        wprintf(L"\nExpected: ");
        vDumpTestStruct(ppTestStructs[numTestData-1]);
        wprintf(L"\n");
    }

    wprintf(L"MATCH first retrieved\n");
    vChlMmFree((void**)&pRetrievedTestData);

    // Find, everything except first and last
    for(index = 1; index < numTestData - 1; ++index)
    {
        wprintf(L"Finding element %d\n", index);
        if(!fChlDsFindLL(pLList, ppTestStructs[index], fCompareTestStructs, (void**)&pRetrievedTestData))
        {
            wprintf(L"!!!! Could not find element at %d after removing first and last\n", index);
            goto test_failed;
        }

        if(!fCompareTestStructs(ppTestStructs[index], pRetrievedTestData))
        {
            wprintf(L"!!!! Found NO MATCH %d: ", index);
            vDumpTestStruct(ppTestStructs[index]);
            wprintf(L"\n");
        }
    }

    wprintf(L"-- PASSED --\n");

    vFreePointerList(ppTestStructs, numTestData);
    fChlDsDestroyLL(pLList);

    wprintf(L"\nEND %s\n", __FUNCTIONW__);
    return TRUE;

test_failed:
    if(ppTestStructs)
    {
        vFreePointerList(ppTestStructs, numTestData);
    }

    if(pLList)
    {
        fChlDsDestroyLL(pLList);
    }
    wprintf(L"-- FAILED --\n");
    wprintf(L"\nEND %s\n", __FUNCTIONW__);
    return FALSE;
}

static BOOL fInsertAndFind_Internal(PTESTSTRUCT *ppTestStructs, int nElems)
{
    ASSERT(nElems > 0);
    ASSERT(ppTestStructs);

    int index;
    PCHL_LLIST pLList = NULL;

    int nElemsNotFound = 0;

    wprintf(L"***************************\nSTART: %d elements\n", nElems);

    // First, create the linked list
    wprintf(L"Creating linked list of %d elements\n", nElems);
    if(!fChlDsCreateLL(&pLList, LL_VAL_PTR, nElems))
    {
        wprintf(L"!!!! Cannot create linked list(size = %d): Error %u\n", nElems, GetLastError());
        return FALSE;
    }

    // Insert all test elements
    wprintf(L"Inserting total of %d elements\n", nElems);
    for(index = 0; index < nElems; ++index)
    {
        if(!fChlDsInsertLL(pLList, ppTestStructs[index], sizeof(TESTSTRUCT)))
        {
            wprintf(L"!!!! Could not insert element %d\n", index);
            goto test_failed;
        }
    }

    // Find all elements and compare their internal data for integrity
    PTESTSTRUCT pStructFound = NULL;
    for(index = nElems - 1; index >= 0; --index)
    {
        wprintf(L"Finding element %d\n", index);
        if(!fChlDsFindLL(pLList, ppTestStructs[index], fCompareTestStructs, (void**)&pStructFound))
        {
            wprintf(L"!!!! Could not find element %d\n", index);
            ++nElemsNotFound;
        }
        else
        {
            if(!fCompareTestStructs(ppTestStructs[index], pStructFound))
            {
                wprintf(L"!!!! Found NO MATCH %d: ", index);
                vDumpTestStruct(ppTestStructs[index]);
                wprintf(L"\n");
            }
        }
    }

    wprintf(L"%d elements not found out of %d elements\n", nElemsNotFound, nElems);

    if(pLList)
    {
        fChlDsDestroyLL(pLList);
        pLList = NULL;
    }

    wprintf(L"\nEND: %d elements\n***************************\n", nElems);
    return nElemsNotFound == 0;

test_failed:
    if(pLList)
    {
        fChlDsDestroyLL(pLList);
        pLList = NULL;
    }

    wprintf(L"\nEND: %d elements\n***************************\n", nElems);
    return FALSE;
}
