

#include "InternalDefines.h"
#include "Hashtable.h"

#define HT_ITR_FIRST    0xdeedbeed
#define HT_ITR_NEXT     0xabcddcba


/* Prime numbers from
 * https://en.wikipedia.org/wiki/List_of_prime_numbers#Centered_heptagonal_primes
 * and Carol primes for 16769023
 * and Circular primes for 319993, 919393
 */
int s_hashSizes[] = { 43,     197,    547,    1471,
                    4663,   8233,   11173,  14561,
                    20483,  93563,  319993, 919393,
                    16769023 };

// File-local Functions
static DWORD _hashi(_In_ int tablesize, _In_ size_t key);
static DWORD _hashs(_In_ int tablesize, _In_bytecount_c_(iKeySize) const BYTE *key, _In_ size_t cchKey);
static DWORD _hashsW(_In_ int tablesize, _In_bytecount_c_(iKeySize) const PUSHORT key, _In_ size_t cchKey);

static void _ClearNode(CHL_KEYTYPE ktype, CHL_VALTYPE vtype, HT_NODE *pnode, BOOL fFreeVal);
static HRESULT _CopyNodeOut(
    _In_ PCHL_HTABLE phtable,
    _In_ HT_NODE* phtNode,
    _Inout_opt_ PCVOID pvKey,
    _Inout_opt_ PINT piKeySize,
    _Inout_opt_ PVOID pvVal,
    _Inout_opt_ PINT piValSize,
    _In_opt_ BOOL fGetPointerOnly);

static BOOL _FindKeyInList(
    _In_ HT_NODE *pFirstHTNode,
    _In_ PVOID pvkey,
    _In_ int iKeySize,
    _In_ CHL_KEYTYPE keyType,
    _Out_ HT_NODE **phtFoundNode,
    _Out_opt_ HT_NODE **phtPrevFound);

static BOOL _FindKnownKeyInList(
    _In_ HT_NODE *pFirstHTNode,
    _In_ HT_NODE *pTarget,
    _Out_ HT_NODE **phtFoundNode,
    _Out_ HT_NODE **phtPrevFound);

static HRESULT _IncrementIterator(_In_ CHL_HT_ITERATOR *pItr);

DWORD _hashi(_In_ int tablesize, _In_ size_t key)
{
    ASSERT(tablesize > 0);

    return key % tablesize;
}

DWORD _hashs(_In_ int tablesize, _In_bytecount_c_(iKeySize) const BYTE *key, _In_ size_t cchKey)
{
    DWORD hash = 5381;
    BYTE c;
    size_t i = 0;

    ASSERT(tablesize > 0);

    //
    // hash function from
    // http://www.cse.yorku.ca/~oz/hash.html
    //

    c = key[0];
    while ((c != 0) && (i < cchKey))
    {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
        c = key[++i];
    }

    return (DWORD)(hash % tablesize);
}

// Wide-char version of the hash function
DWORD _hashsW(_In_ int tablesize, _In_bytecount_c_(iKeySize) const PUSHORT key, _In_ size_t cchKey)
{
    DWORD hash = 5381;
    USHORT us;
    size_t i = 0;

    ASSERT(tablesize > 0);

    //
    // hash function from
    // http://www.cse.yorku.ca/~oz/hash.html
    //

    us = key[0];
    while ((us != 0) && (i < cchKey))
    {
        hash = ((hash << 5) + hash) + us; // hash * 33 + us
        us = key[++i];
    }

    return (DWORD)(hash % tablesize);
}

DWORD _GetKeyHash(_In_ PVOID pvKey, _In_ CHL_KEYTYPE keyType, _In_ int iKeySize, _In_ int iTableNodes)
{
    DWORD dwKeyHash;

    ASSERT(iTableNodes > 0);
    ASSERT((keyType > CHL_KT_START) && (keyType < CHL_KT_END));

    dwKeyHash = 0;
    switch (keyType)
    {
    case CHL_KT_INT32:
    case CHL_KT_UINT32:
    case CHL_KT_POINTER:
        {
            dwKeyHash = _hashi(iTableNodes, (size_t)pvKey);
            break;
        }

    case CHL_KT_STRING:
        {
            size_t nChars = iKeySize / sizeof(char);
            if (iKeySize <= 0)
            {
                nChars = (int)strlen((PCSTR)pvKey);
            }
            dwKeyHash = _hashs(iTableNodes, (const PBYTE)pvKey, nChars);
            break;
        }

    case CHL_KT_WSTRING:
        {
            size_t nChars = iKeySize / sizeof(WCHAR);
            if (iKeySize <= 0)
            {
                nChars = wcslen((PCWSTR)pvKey);
            }
            dwKeyHash = _hashsW(iTableNodes, (const PUSHORT)pvKey, nChars);
            break;
        }

    default:
        {
            logerr("%s(): Invalid keyType %d", __FUNCTION__, keyType);
            ASSERT(!L"Invalid keytype");
            break;
        }
    }
    return dwKeyHash;
}

// TODO: Think about using the CHL_LLIST object here (for hash collisions)

// Creates a hashtable and returns a pointer which can be used for later operations
// on the table.
// params:
//      pHTableOut: Address of pointer where to copy the pointer to the hashtable
//      nEstEntries: Estimated number of entries that would be in the table at any given time.
//                   This is used to determine the initial size of the hashtable.
//      keyType: Type of variable that is used as key - a string or a number
//      valType: Type of value that is stored - number, string or void(can be anything)
//      fValInHeapMem: Set this to true if the value(void type) is allocated memory on the heap
//                     so that it is freed whenever an table entry is removed or when table is destroyed.
// 
HRESULT CHL_DsCreateHT(
    _Inout_ PCHL_HTABLE *pHTableOut,
    _In_ int nEstEntries,
    _In_ CHL_KEYTYPE keyType,
    _In_ CHL_VALTYPE valType,
    _In_opt_ BOOL fValInHeapMem)
{
    int newTableSize = 0;
    PCHL_HTABLE pnewtable = NULL;

    HRESULT hr = S_OK;

    // validate parameters
    if ((nEstEntries < 0) ||
        (keyType < CHL_KT_START) || (keyType > CHL_KT_END) ||
        (valType < CHL_VT_START) || (valType > CHL_VT_END))
    {
        hr = E_INVALIDARG;
        goto error_return;
    }

    // 
    newTableSize = s_hashSizes[CHL_DsGetNearestSizeIndexHT(nEstEntries)];
    if ((pnewtable = (CHL_HTABLE*)malloc(sizeof(CHL_HTABLE))) == NULL)
    {
        logerr("%s(): malloc() ", __FUNCTION__);
        hr = E_OUTOFMEMORY;
        goto error_return;
    }

    pnewtable->fValIsInHeap = fValInHeapMem;
    pnewtable->nTableSize = newTableSize;
    pnewtable->keyType = keyType;
    pnewtable->valType = valType;

    pnewtable->phtNodes = (HT_NODE*)calloc(newTableSize, sizeof(HT_NODE));
    if (pnewtable->phtNodes == NULL)
    {
        logerr("%s(): calloc() ", __FUNCTION__);
        hr = E_OUTOFMEMORY;
        goto error_return;
    }

    pnewtable->Destroy = CHL_DsDestroyHT;
    pnewtable->Insert = CHL_DsInsertHT;
    pnewtable->Find = CHL_DsFindHT;
    pnewtable->Remove = CHL_DsRemoveHT;
    pnewtable->RemoveAt = CHL_DsRemoveAtHT;
    pnewtable->InitIterator = CHL_DsInitIteratorHT;
    pnewtable->Dump = CHL_DsDumpHT;

    *pHTableOut = pnewtable;
    return hr;

error_return:
    if (pnewtable->phtNodes)
    {
        free(pnewtable->phtNodes);
    }
    if (pnewtable)
    {
        free(pnewtable);
    }
    *pHTableOut = NULL;
    return hr;

}

HRESULT CHL_DsDestroyHT(_In_ PCHL_HTABLE phtable)
{
    int i;
    int maxNodes;
    CHL_KEYTYPE ktype;
    CHL_VALTYPE vtype;
    BOOL fInHeap;
    HT_NODE *phtnodes = NULL;
    HT_NODE *pcurnode = NULL;
    HT_NODE *pnextnode = NULL;

    phtnodes = phtable->phtNodes;
    if (phtnodes != NULL)
    {
        ktype = phtable->keyType;
        vtype = phtable->valType;
        maxNodes = phtable->nTableSize;
        fInHeap = phtable->fValIsInHeap;
        for (i = 0; i < maxNodes; ++i)
        {
            pcurnode = phtnodes[i].pnext;
            if (phtnodes[i].fOccupied)
            {
                _ClearNode(ktype, vtype, &phtnodes[i], fInHeap);
            }

            while (pcurnode)
            {
                pnextnode = pcurnode->pnext;

                ASSERT(pcurnode->fOccupied);
                _ClearNode(ktype, vtype, pcurnode, fInHeap);
                CHL_MmFree((PVOID*)&pcurnode);

                pcurnode = pnextnode;
            }
        }
    }

    phtable->nTableSize = 0;
    phtable->phtNodes = NULL;

    DBG_MEMSET(phtable, sizeof(CHL_HTABLE));
    free(phtable);

    return S_OK;
}

HRESULT CHL_DsInsertHT(
    _In_ PCHL_HTABLE phtable,
    _In_ PCVOID pvkey,
    _In_ int iKeySize,
    _In_ PCVOID pvVal,
    _In_ int iValSize)
{
    DWORD index;
    HT_NODE *pNodeAtHashedIndex = NULL;
    HT_NODE *pNodeToInsertTo = NULL;

    CHL_KEYTYPE keyType;

    HRESULT hr = S_OK;

    ASSERT(phtable);

    keyType = phtable->keyType;
    if (iKeySize <= 0 && FAILED(_GetKeySize(pvkey, keyType, &iKeySize)))
    {
        logerr("%s(): Keysize unspecified or unable to determine.", __FUNCTION__);
        hr = E_INVALIDARG;
        goto done;
    }

    if (iValSize <= 0 && FAILED(_GetValSize(pvVal, phtable->valType, &iValSize)))
    {
        logerr("%s(): Valsize unspecified or unable to determine.", __FUNCTION__);
        hr = E_INVALIDARG;
        goto done;
    }

    ASSERT(phtable->nTableSize > 0);
    index = _GetKeyHash(pvkey, keyType, iKeySize, phtable->nTableSize);
    pNodeAtHashedIndex = &phtable->phtNodes[index];

    // Verify that duplicate values are not inserted (same key and value)
    if (pNodeAtHashedIndex->fOccupied == FALSE)
    {
        // Nothing at this index, just copy key:value into this
        pNodeToInsertTo = &phtable->phtNodes[index];
    }
    // We have the key hashing onto a index that is already populated, check if key is duplicate
    else if (!_IsDuplicateKey(&pNodeAtHashedIndex->chlKey, pvkey, keyType, pNodeAtHashedIndex->chlKey.iKeySize))
    {
        // Not a duplicate, so this is just a key collision. Attach to linked list.
        // create a new hashtable node
        hr = CHL_MmAlloc((PVOID*)&pNodeToInsertTo, sizeof(HT_NODE), NULL);
        if (FAILED(hr))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
    // It is a duplicate key
    else
    {
        if (!_IsDuplicateVal(&pNodeAtHashedIndex->chlVal, pvVal, phtable->valType, pNodeAtHashedIndex->chlVal.iValSize))
        {
            // Same key but different value, just update node with new value
            // NOTE: Old value will be lost!!
            _DeleteVal(&pNodeAtHashedIndex->chlVal, phtable->valType, phtable->fValIsInHeap);
            hr = _CopyValIn(&pNodeAtHashedIndex->chlVal, phtable->valType, pvVal, iValSize);
        }
        goto done;
    }

    ASSERT(pNodeToInsertTo);

    // Populate node
    hr = _CopyKeyIn(&pNodeToInsertTo->chlKey, keyType, pvkey, iKeySize);
    if (SUCCEEDED(hr))
    {
        hr = _CopyValIn(&pNodeToInsertTo->chlVal, phtable->valType, pvVal, iValSize);
        if (SUCCEEDED(hr))
        {
            pNodeToInsertTo->fOccupied = TRUE;
            if (pNodeToInsertTo != pNodeAtHashedIndex)
            {
                // There was a key collision. Connect to head of linked list.
                pNodeToInsertTo->pnext = pNodeAtHashedIndex->pnext;
                pNodeAtHashedIndex->pnext = pNodeToInsertTo;
            }
        }
    }

    if (FAILED(hr))
    {
        // Check if we have to free any newly allocated node
        if ((pNodeToInsertTo != pNodeAtHashedIndex) && (pNodeToInsertTo != NULL))
        {
            _ClearNode(phtable->keyType, phtable->valType, pNodeToInsertTo, phtable->fValIsInHeap);
            CHL_MmFree((PVOID*)&pNodeToInsertTo);
        }
    }

done:
    return hr;
}

HRESULT CHL_DsFindHT(
    _In_ PCHL_HTABLE phtable,
    _In_ PCVOID pvkey,
    _In_ int iKeySize,
    _Inout_opt_ PVOID pvVal,
    _Inout_opt_ PINT piValSize,
    _In_opt_ BOOL fGetPointerOnly)
{
    int index = 0;
    HT_NODE *phtFoundNode = NULL;

    HRESULT hr = S_OK;

    ASSERT(phtable->nTableSize > 0);

    if (iKeySize <= 0 && FAILED(_GetKeySize(pvkey, phtable->keyType, &iKeySize)))
    {
        logerr("%s(): Keysize unspecified or unable to determine.", __FUNCTION__);
        hr = E_INVALIDARG;
        goto not_found;
    }

    index = _GetKeyHash(pvkey, phtable->keyType, iKeySize, phtable->nTableSize);
    if (!_FindKeyInList(
        &phtable->phtNodes[index],
        pvkey,
        iKeySize,
        phtable->keyType,
        &phtFoundNode,
        NULL))
    {
        hr = E_NOT_SET;
        goto not_found;
    }

    // Passed in or Calculated keysize should be equal to stored keysize
    ASSERT(iKeySize == phtFoundNode->chlKey.iKeySize);

    if (pvVal)
    {
        hr = _CopyValOut(&phtFoundNode->chlVal, phtable->valType, pvVal, piValSize, fGetPointerOnly);
    }

    if (SUCCEEDED(hr) && (piValSize != NULL))
    {
        *piValSize = phtFoundNode->chlVal.iValSize;
    }

    return hr;

not_found:
    if (piValSize)
    {
        *piValSize = 0;
    }
    return hr;

}

HRESULT CHL_DsRemoveHT(_In_ PCHL_HTABLE phtable, _In_ PCVOID pvkey, _In_ int iKeySize)
{
    int index = 0;
    HT_NODE *phtFoundNode = NULL;
    HT_NODE *phtPrevFound = NULL;

    HRESULT hr = S_OK;

    ASSERT(phtable);
    ASSERT(phtable->nTableSize > 0);

    if (iKeySize <= 0 && FAILED(_GetKeySize(pvkey, phtable->keyType, &iKeySize)))
    {
        logerr("%s(): Keysize unspecified or unable to determine.", __FUNCTION__);
        hr = E_INVALIDARG;
        goto fend;
    }
    index = _GetKeyHash(pvkey, phtable->keyType, iKeySize, phtable->nTableSize);

    if (!_FindKeyInList(
        &phtable->phtNodes[index],
        pvkey,
        iKeySize,
        phtable->keyType,
        &phtFoundNode,
        &phtPrevFound))
    {
        hr = E_NOT_SET;
        goto fend;
    }

    ASSERT(phtFoundNode);

    if (phtFoundNode == &phtable->phtNodes[index])
    {
        // Node in the main table is to be removed
        // Preserve pnext and restore because _ClearNode sets it to NULL
        HT_NODE* pnext = phtFoundNode->pnext;
        _ClearNode(phtable->keyType, phtable->valType, phtFoundNode, phtable->fValIsInHeap);
        phtFoundNode->pnext = pnext;
    }
    else
    {
        ASSERT(phtPrevFound);
        phtPrevFound->pnext = phtFoundNode->pnext;
        _ClearNode(phtable->keyType, phtable->valType, phtFoundNode, phtable->fValIsInHeap);
        CHL_MmFree((PVOID*)&phtFoundNode);
    }

fend:
    return hr;
}

HRESULT CHL_DsRemoveAtHT(_Inout_ CHL_HT_ITERATOR *pItr)
{
    int iFoundIndex = -1;
    HT_NODE *phtFoundNode = NULL;
    HT_NODE *phtPrevFound = NULL;

    HRESULT hr = S_OK;
    HRESULT hrIncrement = S_OK;

    ASSERT(pItr->pMyHashTable);
    ASSERT(pItr->pMyHashTable->nTableSize > 0);

    PCHL_HTABLE phtable = pItr->pMyHashTable;

    if ((pItr->opType != HT_ITR_NEXT) || (pItr->phtCurNodeInList == NULL)
        || (pItr->nCurIndex < 0) || (phtable->nTableSize <= pItr->nCurIndex)
        || !_FindKnownKeyInList(&phtable->phtNodes[pItr->nCurIndex], pItr->phtCurNodeInList, &phtFoundNode, &phtPrevFound))
    {
        hr = E_NOT_SET;
        goto fend;
    }

    ASSERT(phtFoundNode == pItr->phtCurNodeInList);
    iFoundIndex = pItr->nCurIndex;

    hrIncrement = _IncrementIterator(pItr);
    if (SUCCEEDED(hrIncrement))
    {
        ASSERT(pItr->phtCurNodeInList->fOccupied == TRUE);
        ASSERT(pItr->nCurIndex < pItr->pMyHashTable->nTableSize);
    }
    else
    {
        // Cannot use this iterator anymore before calling InitIterator again.
        ASSERT(pItr->phtCurNodeInList == NULL);
        ASSERT(pItr->pMyHashTable->nTableSize <= pItr->nCurIndex);
    }

    if (phtFoundNode == &phtable->phtNodes[iFoundIndex])
    {
        // Node in the main table is to be removed
        // Preserve pnext and restore because _ClearNode sets it to NULL
        HT_NODE* pnext = phtFoundNode->pnext;
        _ClearNode(phtable->keyType, phtable->valType, phtFoundNode, phtable->fValIsInHeap);
        phtFoundNode->pnext = pnext;
    }
    else
    {
        ASSERT(phtPrevFound);
        phtPrevFound->pnext = phtFoundNode->pnext;
        _ClearNode(phtable->keyType, phtable->valType, phtFoundNode, phtable->fValIsInHeap);
        CHL_MmFree((PVOID*)&phtFoundNode);
    }

fend:
    return hr;
}

HRESULT CHL_DsInitIteratorHT(_In_ PCHL_HTABLE phtable, _Out_ CHL_HT_ITERATOR *pItr)
{
    pItr->opType = HT_ITR_FIRST;
    pItr->nCurIndex = 0;
    pItr->phtCurNodeInList = NULL;
    pItr->pMyHashTable = phtable;
    pItr->MoveNext = CHL_DsMoveNextHT;
    pItr->GetCurrent = CHL_DsGetCurrentHT;
    return _IncrementIterator(pItr); // move to first element or the end if none exist
}

HRESULT CHL_DsMoveNextHT(_Inout_ CHL_HT_ITERATOR *pItr)
{
    ASSERT(pItr && pItr->pMyHashTable);
    return _IncrementIterator(pItr);
}

HRESULT CHL_DsGetCurrentHT(
    _In_ CHL_HT_ITERATOR *pItr,
    _Inout_opt_ PCVOID pvKey,
    _Inout_opt_ PINT piKeySize,
    _Inout_opt_ PVOID pvVal,
    _Inout_opt_ PINT piValSize,
    _In_opt_ BOOL fGetPointerOnly)
{
    ASSERT(pItr && pItr->pMyHashTable);

    if (pItr->phtCurNodeInList == NULL)
    {
        return E_NOT_SET;
    }
    return _CopyNodeOut(pItr->pMyHashTable, pItr->phtCurNodeInList, pvKey, piKeySize, pvVal, piValSize, fGetPointerOnly);
}

int CHL_DsGetNearestSizeIndexHT(_In_ int maxNumberOfEntries)
{
    int index = 0;
    while ((index < (_countof(s_hashSizes) - 1)) && (maxNumberOfEntries > s_hashSizes[index]))
    {
        ++index;
    }
    return index;
}

void CHL_DsDumpHT(_In_ PCHL_HTABLE phtable)
{
    DBG_UNREFERENCED_PARAMETER(phtable);

    //int i;
    //int nNodes = 0;
    //int uTableSize = -1;

    //CHL_KEYTYPE keyType;
    //CHL_VALTYPE valType;
    //
    //HT_NODE **phtNodes = NULL;
    //HT_NODE *phtCurNode = NULL;

    //ASSERT(phtable && phtable->nTableSize > 0);

    //uTableSize = phtable->nTableSize;
    //keyType = phtable->keyType;
    //valType = phtable->valType;
    //
    //phtNodes = phtable->phtNodes;
    //
    //printf("-----------------------------------\n");
    //printf("Hashtable Dump::\nkey  :  iValSize  :  chlVal\n");
    //printf("-----------------------------------\n");
    //
    //if(!phtNodes)
    //{
    //    printf("Hastable empty");
    //    goto done;
    //}
    //
    //for(i = 0; i < uTableSize; ++i)
    //{
    //    if((phtCurNode = phtNodes[i]) == NULL) continue;
    //    
    //    while(phtCurNode)
    //    {
    //        ++nNodes;
    //        
    //        // print the key
    //        switch(keyType)
    //        {
    //            case HT_KEY_STR:
    //            {
    //                int chars = 0;
    //                while(chars < phtCurNode->iKeySize)
    //                    putchar(phtCurNode->chlKey.skey[chars++]);
    //                putchar(':');
    //                break;
    //            }

    //            case HT_KEY_DWORD:printf("%u:", phtCurNode->chlKey.dwkey); break;
    //            default: printf("Invalid keyType %d:", keyType); break;
    //        }

    //        // print the iValSize
    //        printf("%d:", phtCurNode->iValSize);

    //        // print the value
    //        if(valType == HT_VAL_STR)
    //            printf("[%s]\n", phtCurNode->chlVal.pbVal);
    //        else if(valType == HT_VAL_DWORD)
    //            printf("[%u]\n", phtCurNode->chlVal.dwVal);
    //        else if(valType == HT_VAL_PTR)
    //            printf("[0x%p]\n", phtCurNode->chlVal.pbVal);
    //        else
    //            printf("Invalid valType %d\n", valType);
    //        phtCurNode = phtCurNode->pnext;
    //    
    //    }// while phtCurNode
    //    
    //}// for i
    //
    //printf("Hashtable stats:\n");
    //printf("Total size     : %u\n", uTableSize);
    //printf("Occupied       : %d\n", nNodes);
    //    
    //done:
    //return;

}// CHL_DsDumpHT

void _ClearNode(CHL_KEYTYPE ktype, CHL_VALTYPE vtype, HT_NODE *pnode, BOOL fFreeVal)
{
    ASSERT(pnode);

    _DeleteKey(&pnode->chlKey, ktype);
    _DeleteVal(&pnode->chlVal, vtype, fFreeVal);

    pnode->fOccupied = FALSE;
    pnode->pnext = NULL;

    return;
}// _ClearNode()

HRESULT _CopyNodeOut(
    _In_ PCHL_HTABLE phtable,
    _In_ HT_NODE* phtNode,
    _Inout_opt_ PCVOID pvKey,
    _Inout_opt_ PINT piKeySize,
    _Inout_opt_ PVOID pvVal,
    _Inout_opt_ PINT piValSize,
    _In_opt_ BOOL fGetPointerOnly)
{
    ASSERT(phtNode->fOccupied == TRUE);

    HRESULT hr = S_OK;
    
    if (pvKey)
    {
        hr = _CopyKeyOut(&phtNode->chlKey, phtable->keyType, pvKey, piKeySize, fGetPointerOnly);
    }
    if (SUCCEEDED(hr) && pvVal)
    {
        hr = _CopyValOut(&phtNode->chlVal, phtable->valType, pvVal, piValSize, fGetPointerOnly);
    }

    if (SUCCEEDED(hr))
    {
        if (piKeySize != NULL)
        {
            *piKeySize = phtNode->chlKey.iKeySize;
        }

        if (piValSize != NULL)
        {
            *piValSize = phtNode->chlVal.iValSize;
        }
    }
    return hr;
}

BOOL _FindKeyInList(
    _In_ HT_NODE *pFirstHTNode,
    _In_ PVOID pvkey,
    _In_ int iKeySize,
    _In_ CHL_KEYTYPE keyType,
    _Out_ HT_NODE **phtFoundNode,
    _Out_opt_ HT_NODE **phtPrevFound)
{
    HT_NODE *prevNode = NULL;
    HT_NODE *pcurNode = NULL;

    ASSERT(phtFoundNode);

    *phtFoundNode = NULL;

    pcurNode = pFirstHTNode;
    while (pcurNode)
    {
        if (pcurNode->fOccupied &&
            (pcurNode->chlKey.iKeySize == iKeySize) &&
            _IsDuplicateKey(&pcurNode->chlKey, pvkey, keyType, pcurNode->chlKey.iKeySize))
        {
            break;
        }

        prevNode = pcurNode;
        pcurNode = pcurNode->pnext;
    }

    *phtFoundNode = pcurNode;
    if (phtPrevFound)
    {
        *phtPrevFound = prevNode;
    }

    return (pcurNode != NULL);
}

BOOL _FindKnownKeyInList(
    _In_ HT_NODE *pFirstHTNode,
    _In_ HT_NODE *pTarget,
    _Out_ HT_NODE **phtFoundNode,
    _Out_ HT_NODE **phtPrevFound)
{
    HT_NODE *prevNode = NULL;
    HT_NODE *pcurNode = NULL;

    ASSERT(phtFoundNode && phtPrevFound);

    *phtFoundNode = NULL;

    pcurNode = pFirstHTNode;
    while (pcurNode && (pcurNode != pTarget))
    {
        prevNode = pcurNode;
        pcurNode = pcurNode->pnext;
    }

    *phtFoundNode = pcurNode;
    *phtPrevFound = prevNode;
    return (pcurNode != NULL);
}

HRESULT _IncrementIterator(_In_ CHL_HT_ITERATOR *pItr)
{
    int indexIterator;
    int iCurIndex = 0;
    HT_NODE* pCurNode = NULL;

    HRESULT hr = S_OK;
    PCHL_HTABLE phtable = pItr->pMyHashTable;
    if (pItr->opType == HT_ITR_FIRST) // Trivial check to see if iterator was initialized or not
    {
        for (indexIterator = 0; indexIterator < phtable->nTableSize; ++indexIterator)
        {
            if (phtable->phtNodes[indexIterator].fOccupied)
            {
                iCurIndex = indexIterator;
                pCurNode = &phtable->phtNodes[indexIterator];
                pItr->opType = HT_ITR_NEXT;
                break;  // out of for loop
            }
        }

        if (indexIterator >= phtable->nTableSize)
        {
            pCurNode = NULL;
            iCurIndex = phtable->nTableSize;
            hr = E_NOT_SET; // reached End of table
        }
    }
    else if (pItr->opType == HT_ITR_NEXT)
    {
        pCurNode = pItr->phtCurNodeInList;
        iCurIndex = pItr->nCurIndex;
        if (pCurNode && pCurNode->pnext)
        {
            pCurNode = pCurNode->pnext;
        }
        else
        {
            for (indexIterator = iCurIndex + 1; indexIterator < phtable->nTableSize; ++indexIterator)
            {
                if (phtable->phtNodes[indexIterator].fOccupied)
                {
                    iCurIndex = indexIterator;
                    pCurNode = &phtable->phtNodes[indexIterator];
                    pItr->opType = HT_ITR_NEXT;
                    break;  // out of for loop
                }
            }

            if (indexIterator >= phtable->nTableSize)
            {
                pCurNode = NULL;
                iCurIndex = phtable->nTableSize;
                hr = E_NOT_SET; // reached End of table
            }
        }
    }
    else
    {
        logerr("Iterator invalid opType %x", pItr->opType);
        hr = E_UNEXPECTED;
    }

    pItr->nCurIndex = iCurIndex;
    pItr->phtCurNodeInList = pCurNode;
    return hr;
}
