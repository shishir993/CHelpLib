
#include "CommonInclude.h"
#include "TestLinkedList.h"

#define NUM_TESTS   1

typedef struct _testStruct {
    UINT ui;
    DWORD dw;
    void *pv;
    WCHAR sz[16];
}TESTSTRUCT, *PTESTSTRUCT;

// Tests
static BOOL fInsertAndFind();
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

void vDumpTestData()
{
    int nElems = 10;

    PTESTSTRUCT *ppTestData = NULL;

    if(!fCreateTestStructs(nElems, &ppTestData))
    {
        return;
    }

    for(int i = 0; i < nElems; ++i)
    {
        wprintf(L"%d: ", i);
        vDumpTestStruct(ppTestData[i]);
        wprintf(L"\n");
    }

    wprintf(L"\n");

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
// ************************ Test Helpers ************************
// END

//
BOOL fTestLinkedList()
{
    int nTotalTests = NUM_TESTS;
    int nPassed = 0;
    int nFailed = 0;

    if(fInsertAndFind())
    {
        ++nPassed;
    }

    wprintf(L"TEST Linked List: Total = %d, Passed = %d\n", nTotalTests, nPassed);

    return nFailed == 0;
}

// Basic insert and find test.
// - Insert 1, 2, 8 elements and find all.
// : All inserted elements must be found
static BOOL fInsertAndFind()
{
    PTESTSTRUCT *ppTestStructs = NULL;
    BOOL fResult = TRUE;

    int nCurrentElems;

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

    return fResult;

error_return:
    return FALSE;
}

static BOOL fInsertAndFind_Internal(PTESTSTRUCT *ppTestStructs, int nElems)
{
    ASSERT(nElems > 0);
    ASSERT(ppTestStructs);

    int index;
    PCHL_LLIST pLList = NULL;

    int nElemsNotFound = 0;

    wprintf(L"***************************\nSTART: %d elements\n***************************\n", nElems);

    // First, create the linked list
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
        if(!fChlDsFindLL(pLList, ppTestStructs[index], fCompareTestStructs, (void**)&pStructFound))
        {
            wprintf(L"!!!! Could not find element %d\n", index);
            ++nElemsNotFound;
        }
        else
        {
            if(!fCompareTestStructs(ppTestStructs[index], pStructFound))
            {
                wprintf(L"!!!! Found NO MATCH: ");
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

    wprintf(L"***************************\nEND: %d elements\n***************************\n", nElems);
    return nElemsNotFound == 0;

test_failed:
    if(pLList)
    {
        fChlDsDestroyLL(pLList);
        pLList = NULL;
    }

    wprintf(L"***************************\nEND: %d elements\n***************************\n", nElems);
    return FALSE;
}
