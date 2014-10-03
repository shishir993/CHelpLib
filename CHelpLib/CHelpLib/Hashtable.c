

#include "Hashtable.h"
#include "General.h"

#define HT_ITR_FIRST    0xdeedbeed
#define HT_ITR_NEXT     0xabcddcba

#define MUTEX_NAME_HT   (TEXT("CHL_MUTEX_HT"))

/* Prime numbers from 
 * https://en.wikipedia.org/wiki/List_of_prime_numbers#Centered_heptagonal_primes
 * and Carol primes for 16769023
 * and Circular primes for 319993, 919393
 */
unsigned int hashSizes[] = {43,     197,    547,    1471,
                            4663,   8233,   11173,  14561,  
                            20483,  93563,  319993, 919393,
                            16769023};

// File-local Functions
static DWORD _hashi(_In_ int tablesize, _In_ int key);
static DWORD _hashu(_In_ int tablesize, _In_ UINT key);
static DWORD _hashs(_In_ int tablesize, _In_bytecount_c_(iKeySize) const BYTE *key, _In_ int iKeySize);
static DWORD _GetKeyHash(_In_ PVOID pvKey, _In_ CHL_KEYTYPE keyType, _In_ int iKeySize, _In_ int iTableNodes);

static void _DeleteNode(CHL_KEYTYPE ktype, CHL_VALTYPE vtype, HT_NODE *pnode, BOOL fFreeVal);

static HRESULT _UpdateNodeVal(HT_NODE *pnode, PVOID pvVal, CHL_VALTYPE valType, int iValSize);

static BOOL _FindKeyInList(
    _In_ HT_NODE *pFirstHTNode, 
    _In_ PVOID pvkey, 
    _In_ int iKeySize, 
    _In_ CHL_KEYTYPE keyType, 
    _Out_ HT_NODE **phtFoundNode, 
    _Out_opt_ HT_NODE **phtPrevFound);


DWORD _hashi(_In_ int tablesize, _In_ int key)
{
    ASSERT(tablesize > 0);
    
    return key % tablesize;
}

DWORD _hashu(_In_ int tablesize, _In_ UINT key)
{
    ASSERT(tablesize > 0);
    
    return key % tablesize;
}

DWORD _hashs(_In_ int tablesize, _In_bytecount_c_(iKeySize) const BYTE *key, int iKeySize)
{
    DWORD hash = 5381;
    BYTE c;
    int i = 0;

    ASSERT(tablesize > 0);

    //
    // hash function from
    // http://www.cse.yorku.ca/~oz/hash.html
    //

    c = key[0];
    while ((c != 0) && (i < iKeySize))
    {
      hash = ((hash << 5) + hash) + c; // hash * 33 + c
      c = key[i++];
    }

    return (DWORD)(hash % tablesize);
}

DWORD _GetKeyHash(_In_ PVOID pvKey, _In_ CHL_KEYTYPE keyType, _In_ int iKeySize, _In_ int iTableNodes)
{
    DWORD dwKeyHash;

    ASSERT(iKeySize > 0);
    ASSERT(iTableNodes > 0);
    ASSERT((keyType > CHL_KT_START) && (keyType < CHL_KT_END));

    dwKeyHash = 0;
    switch(keyType)
    {
        case CHL_KT_INT32:
        {
            dwKeyHash = _hashi(iTableNodes, (int)pvKey);
            break;
        }

        case CHL_KT_UINT32:
        case CHL_KT_POINTER:
        {
            dwKeyHash = _hashu(iTableNodes, (UINT)pvKey);
            break;
        }

        case CHL_KT_STRING:
        {
            int nBytes = iKeySize;
            if(nBytes <= 0)
            {
                nBytes = strlen((PCSTR)pvKey) + sizeof(char);
            }
            dwKeyHash = _hashs(iTableNodes, (const PBYTE)pvKey, nBytes);
            break;
        }

        case CHL_KT_WSTRING:
        {
            // TODO
            break;
        }

        default:
        {
            logerr("%s(): Invalid keyType %d", __FUNCTION__, keyType);
            ASSERT(FALSE);
            break;
        }
    }
    return dwKeyHash;
}

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
    WCHAR wsMutexName[32];

    unsigned int uiRand;

    HRESULT hr = S_OK;

    // validate parameters
    if((pHTableOut == NULL) || 
        (nEstEntries < 0) || 
        (keyType < CHL_KT_START) || (keyType > CHL_KT_END) ||
        (valType < CHL_VT_START) || (valType > CHL_VT_END))
    {
        hr = E_INVALIDARG;
        goto error_return;
    }
    
    // Create a unique number to be appended to the mutex name
    if( rand_s(&uiRand) != 0 )
    {
        logerr("Unable to get a random number");
        hr = E_FAIL;
        goto error_return;
    }
    swprintf_s(wsMutexName, 32, L"%s_%08X", MUTEX_NAME_HT, uiRand);

    // 
    newTableSize = hashSizes[CHL_DsGetNearestSizeIndexHT(nEstEntries)];
    if((pnewtable = (CHL_HTABLE*)malloc(sizeof(CHL_HTABLE))) == NULL)
    {
        logerr("%s(): malloc() ", __FUNCTION__);
        hr = E_OUTOFMEMORY;
        goto error_return;
    }
    
    pnewtable->fValIsInHeap = fValInHeapMem;
    pnewtable->nTableSize = newTableSize;
    pnewtable->keyType = keyType;
    pnewtable->valType = valType;

    pnewtable->hMuAccess = CreateMutex( NULL, FALSE, wsMutexName);
    if(pnewtable->hMuAccess == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto error_return;
    }
    
    pnewtable->phtNodes = (HT_NODE**)calloc(1, newTableSize * sizeof(HT_NODE*));
    if(pnewtable->phtNodes == NULL)
    {
        logerr("%s(): calloc() ", __FUNCTION__);
        hr = E_OUTOFMEMORY;
        goto error_return;
    }

    pnewtable->Destroy = CHL_DsDestroyHT;
    pnewtable->Insert = CHL_DsInsertHT;
    pnewtable->Find = CHL_DsFindHT;
    pnewtable->Remove = CHL_DsRemoveHT;
    pnewtable->InitIterator = CHL_DsInitIteratorHT;
    pnewtable->GetNext = CHL_DsGetNextHT;
    pnewtable->Dump = CHL_DsDumpHT;

    *pHTableOut = pnewtable;
    return hr;
    
error_return:
    if(pnewtable->phtNodes)
    {
        free(pnewtable->phtNodes);
    }
    if(pnewtable)
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
    HT_NODE **phtnodes = NULL;
    HT_NODE *pcurnode = NULL;
    HT_NODE *pnextnode = NULL;

    HRESULT hr = S_OK;
    if(!phtable)
    {
        hr = E_INVALIDARG;
        goto done;  // hashtable isn't destroyed
    }
    
    hr = CHL_GnOwnMutex(phtable->hMuAccess);
    if(FAILED(hr))
    {
        goto done;  // hashtable isn't destroyed
    }

    phtnodes = phtable->phtNodes;
    if(phtnodes != NULL)
    {
        ktype = phtable->keyType;
        vtype = phtable->valType;
        maxNodes = phtable->nTableSize;
        fInHeap = phtable->fValIsInHeap;
        for(i = 0; i < maxNodes; ++i)
        {
            pcurnode = phtnodes[i];
            while(pcurnode)
            {
                pnextnode = pcurnode->pnext;
                _DeleteNode(ktype, vtype, pcurnode, fInHeap);
                pcurnode = pnextnode;
            }   
        }   
    }
    
    //ReleaseMutex(phtable->hMuAccess); // ToDo: should we release or not??
    CloseHandle(phtable->hMuAccess);

    DBG_MEMSET(phtable, sizeof(CHL_HTABLE));
    free(phtable);

done:
    return hr;
}

HRESULT CHL_DsInsertHT(
    _In_ PCHL_HTABLE phtable, 
    _In_ PCVOID pvkey, 
    _In_ int keySize, 
    _In_ PCVOID pvVal, 
    _In_ int iValSize)
{
    DWORD index;
    HT_NODE *pnewnode = NULL;
    
    BOOL fLocked = FALSE;
    
    CHL_KEYTYPE keyType;

    HRESULT hr = S_OK;

    ASSERT(phtable);
    ASSERT(iValSize > 0);
    
    // validate parameters
    if(!phtable || (keySize <= 0))
    {
        hr = E_INVALIDARG;
        goto error_return;
    }

    // create a new hashtable node
    if( (pnewnode = (HT_NODE*)malloc(sizeof(HT_NODE))) == NULL )
    { 
        logerr("CHL_DsInsertHT(): malloc() ");
        hr = E_OUTOFMEMORY;
        goto error_return;
    }
    
    keyType = phtable->keyType;
    hr = _CopyKeyIn(&pnewnode->chlKey, keyType, pvkey, keySize);
    if(FAILED(hr))
    {
        goto delete_newnode_return;
    }
    pnewnode->iKeySize = keySize;

    hr = _CopyValIn(&pnewnode->chlVal, phtable->valType, pvVal, iValSize);
    if(FAILED(hr))
    {
        goto delete_newnode_return;
    }
    pnewnode->iValSize = iValSize;

    // insert into hashtable
    hr = CHL_GnOwnMutex(phtable->hMuAccess);
    if(FAILED(hr))
    {
        goto error_return;
    }
    fLocked = TRUE;

    ASSERT(phtable->nTableSize > 0);
    index = _GetKeyHash(pvkey, keyType, keySize, phtable->nTableSize);
    
    // verify that duplicate values are not inserted (same key and value)
    if(phtable->phtNodes[index])
    {
        // We have the key hashing onto a index that is already populated
        if(_IsDuplicateKey(&(phtable->phtNodes[index]->chlKey), pvkey, keyType, keySize))
        {
            if(_IsDuplicateVal(&(phtable->phtNodes[index]->chlVal), pvVal, phtable->valType, iValSize))
            {
                // Both key and value are same
                // Delete the newly created node and return
                _DeleteNode(phtable->keyType, phtable->valType, pnewnode, phtable->fValIsInHeap);
                goto done;
            }
            else
            {
                // Same key but different value, just update node with new value
                // NOTE: Old value will be lost!!
                if(!_UpdateNodeVal(phtable->phtNodes[index], pvVal, phtable->valType, iValSize))
                {
                    hr = E_FAIL;
                }
                goto done;
            }
        }
    }
    
    // connect to head
    pnewnode->pnext = phtable->phtNodes[index];
    phtable->phtNodes[index] = pnewnode;
    
done:
    ReleaseMutex(phtable->hMuAccess);
    return hr;
    
delete_newnode_return:
    _DeleteNode(phtable->keyType, phtable->valType, pnewnode, phtable->fValIsInHeap);
    
error_return:
    if(fLocked && !ReleaseMutex(phtable->hMuAccess))
    {
        logerr("CHL_DsInsertHT(): error_return mutex unlock ");
    }
    return hr;
    
}


int exhandler(int excode, LPEXCEPTION_POINTERS exptrs)
{
    EXCEPTION_RECORD *pexrec = exptrs->ExceptionRecord;
    PCONTEXT pcontext = exptrs->ContextRecord;

    DBG_UNREFERENCED_PARAMETER(excode);
    DBG_UNREFERENCED_LOCAL_VARIABLE(pexrec);
    DBG_UNREFERENCED_LOCAL_VARIABLE(pcontext);

    __asm int 3
    return 0;
}

HRESULT CHL_DsFindHT(
    _In_ PCHL_HTABLE phtable, 
    _In_ PCVOID pvkey, 
    _In_ int keySize, 
    _Inout_opt_ PVOID pvVal, 
    _Inout_opt_ PINT pvalsize,
    _In_opt_ BOOL fGetPointerOnly)
{
    int index = 0;
    HT_NODE *phtFoundNode = NULL;
    BOOL fLocked = FALSE;

    HRESULT hr = S_OK;

    ASSERT(phtable);
    ASSERT(keySize > 0);
    
    hr = CHL_GnOwnMutex(phtable->hMuAccess);
    if(FAILED(hr))
    {
        goto not_found;
    }
    fLocked = TRUE;
    
    ASSERT(phtable->nTableSize > 0);
    index = _GetKeyHash(pvkey, phtable->keyType, keySize, phtable->nTableSize);
    
    phtFoundNode = phtable->phtNodes[index];
    if(!phtFoundNode || 
        !_FindKeyInList(phtFoundNode, pvkey, keySize, phtable->keyType, 
                        &phtFoundNode, NULL))
    {
        hr = E_NOT_SET;
        goto not_found;
    }

    ASSERT(phtFoundNode);
    if(pvVal)
    {
        _CopyValOut(&phtFoundNode->chlVal, phtable->valType, pvVal, fGetPointerOnly);
    }

    if(pvalsize)
    {
        *pvalsize = phtFoundNode->iValSize;
    }
    ReleaseMutex(phtable->hMuAccess);
    return hr;
    
not_found:
    if(pvalsize) *pvalsize = 0;
    if(fLocked && !ReleaseMutex(phtable->hMuAccess))
        logerr("CHL_DsFindHT(): not_found mutex unlock ");
    return hr;
    
}

HRESULT CHL_DsRemoveHT(_In_ PCHL_HTABLE phtable, _In_ PCVOID pvkey, _In_ int keySize)
{   
    int index = 0;
    HT_NODE *phtFoundNode = NULL;
    HT_NODE *phtPrevFound = NULL;
    
    BOOL fLocked = FALSE;

    HRESULT hr = S_OK;

    ASSERT(phtable && pvkey);
    
    hr = CHL_GnOwnMutex(phtable->hMuAccess);
    if(FAILED(hr))
    {
        goto error_return;
    }
    fLocked = TRUE;
    
    ASSERT(phtable->nTableSize > 0);
    index = _GetKeyHash(pvkey, phtable->keyType, keySize, phtable->nTableSize);
    
    phtFoundNode = phtable->phtNodes[index];
    if(!phtFoundNode ||
        !_FindKeyInList(phtFoundNode, pvkey, keySize, phtable->keyType, 
            &phtFoundNode, &phtPrevFound))
    {
        hr = E_NOT_SET;
        goto error_return;
    }

    ASSERT(phtFoundNode);

    // re-arrange pnext pointers
    if(phtPrevFound)
        phtPrevFound->pnext = phtFoundNode->pnext;
    else
        phtable->phtNodes[index] = NULL;

    _DeleteNode(phtable->keyType, phtable->valType, phtFoundNode, phtable->fValIsInHeap);
    
    ReleaseMutex(phtable->hMuAccess);
    return hr;
    
error_return:
    if(fLocked && !ReleaseMutex(phtable->hMuAccess))
        logerr("CHL_DsRemoveHT(): error_return mutex unlock ");
    return hr;
    
}

HRESULT CHL_DsInitIteratorHT(_In_ CHL_HT_ITERATOR *pItr)
{
    if(!pItr)
    {
        return E_INVALIDARG;
    }

    pItr->opType = HT_ITR_FIRST;
    pItr->nCurIndex = 0;
    pItr->phtCurNodeInList = NULL;
    return S_OK;
}

HRESULT CHL_DsGetNextHT(
    _In_ PCHL_HTABLE phtable, 
    _In_ CHL_HT_ITERATOR *pItr, 
    _Inout_opt_ PVOID pvKey, 
    _Inout_opt_ PINT pkeysize,
    _Inout_opt_ PVOID pvVal, 
    _Inout_opt_ PINT pvalsize,
    _In_opt_ BOOL fGetPointerOnly)
{
    
    int indexIterator;
    HRESULT hr;

    ASSERT(phtable && pItr);

    if(!phtable || !pItr)
        return E_INVALIDARG;

    // Trivial check to see if iterator was initialized or not
    hr = S_OK;
    if(pItr->opType == HT_ITR_FIRST)
    {
        for(indexIterator = 0; indexIterator < phtable->nTableSize; ++indexIterator)
        {
            if(phtable->phtNodes[indexIterator])
            {
                pItr->nCurIndex = indexIterator;
                pItr->phtCurNodeInList = phtable->phtNodes[indexIterator];
                pItr->opType = HT_ITR_NEXT;
                break;  // out of for loop
            }
        }

        if(indexIterator >= phtable->nTableSize)
        {
            pItr->phtCurNodeInList = NULL;
            pItr->nCurIndex = phtable->nTableSize;
            hr = E_NOT_SET;// return FALSE;  // reached End of table
        }
    }
    else if(pItr->opType == HT_ITR_NEXT)
    {
        if(pItr->phtCurNodeInList && pItr->phtCurNodeInList->pnext)
        {
            pItr->phtCurNodeInList = pItr->phtCurNodeInList->pnext;
        }
        else
        {
            for(indexIterator = pItr->nCurIndex + 1; indexIterator < phtable->nTableSize; ++indexIterator)
            {
                if(phtable->phtNodes[indexIterator])
                {
                    pItr->nCurIndex = indexIterator;
                    pItr->phtCurNodeInList = phtable->phtNodes[indexIterator];
                    pItr->opType = HT_ITR_NEXT;
                    break;  // out of for loop
                }
            }

            if(indexIterator >= phtable->nTableSize)
            {
                pItr->phtCurNodeInList = NULL;
                pItr->nCurIndex = phtable->nTableSize;
                hr = E_NOT_SET; // return FALSE;  // reached End of table
            }
        }
    }
    else
    {
        logerr("Iterator invalid opType %x", pItr->opType);
        hr = E_UNEXPECTED;
    }

    if(SUCCEEDED(hr))
    {
        if(pvKey)
            _CopyKeyOut(&pItr->phtCurNodeInList->chlKey, phtable->keyType, pvKey, fGetPointerOnly);
        if(pkeysize) 
            *pkeysize = pItr->phtCurNodeInList->iKeySize;

        if(pvVal)
            _CopyValOut(&pItr->phtCurNodeInList->chlVal, phtable->valType, pvVal, fGetPointerOnly);
        if(pvalsize)
            *pvalsize = pItr->phtCurNodeInList->iValSize;
    }

    return hr;
}

int CHL_DsGetNearestSizeIndexHT(_In_ int maxNumberOfEntries)
{
    int index = 0;

    while(index < (_countof(hashSizes)-1) && maxNumberOfEntries > hashSizes[index])
        ++index;

    return index;
}


void CHL_DsDumpHT(_In_ PCHL_HTABLE phtable)
{   
    DBG_UNREFERENCED_PARAMETER(phtable);

    //int i;
    //int nNodes = 0;
    //int uTableSize = -1;
    //BOOL fLocked = FALSE;
    //
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
    //if(FAILED(CHL_GnOwnMutex(phtable->hMuAccess)))
    //    goto done;
    //fLocked = TRUE;
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
    //if(fLocked && !ReleaseMutex(phtable->hMuAccess))
    //    logerr("CHL_DsDumpHT(): mutex unlock ");  
    //return;
    
}// CHL_DsDumpHT

void _DeleteNode(CHL_KEYTYPE ktype, CHL_VALTYPE vtype, HT_NODE *pnode, BOOL fFreeVal)
{
    ASSERT(pnode);

    // Assume mutex is in our ownership
    
    _DeleteKey(&pnode->chlKey, ktype);

    if(vtype == CHL_VT_POINTER && fFreeVal)
    {
        free(pnode->chlVal.pvPtr);
    }

    _DeleteVal(&pnode->chlVal, vtype);
    
    DBG_MEMSET(pnode, sizeof(HT_NODE));
    free(pnode);
    
    return;
}// _DeleteNode()

HRESULT _UpdateNodeVal(HT_NODE *pnode, PVOID pvVal, CHL_VALTYPE valType, int iValSize)
{
    // Free previous value if allocated on heap
    switch(valType)
    {
        case CHL_VT_USEROBJECT:
        {
            if(pnode->chlVal.pvUserObj)
            {
                CHL_MmFree((PVOID*)&pnode->chlVal.pvUserObj);
            }
            break;
        }

        case CHL_VT_STRING:
        {
            if(pnode->chlVal.pszVal)
            {
                CHL_MmFree((PVOID*)&pnode->chlVal.pszVal);
            }
            break;
        }

        case CHL_VT_WSTRING:
        {
            if(pnode->chlVal.pwszVal)
            {
                CHL_MmFree((PVOID*)&pnode->chlVal.pwszVal);
            }
            break;
        }

        default:
        {
            break;
        }
    }

    return _CopyValIn(&pnode->chlVal, valType, pvVal, iValSize);
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
    
    pcurNode = pFirstHTNode;
    
    // In case of collisions, find the key in the sibling linked list

    while(pcurNode)
    {
        if(pcurNode->iKeySize != iKeySize)
        {
            prevNode = pcurNode;
            pcurNode = pcurNode->pnext;
            continue;
        }

        if(_IsDuplicateKey(&pcurNode->chlKey, pvkey, keyType, pcurNode->iKeySize))
        {
            break;
        }

        prevNode = pcurNode;
        pcurNode = pcurNode->pnext;
    }

    if(pcurNode)
    {
        if(phtPrevFound) *phtPrevFound = prevNode;
        *phtFoundNode = pcurNode;
        return TRUE;
    }

    if(phtPrevFound)
    {
        *phtPrevFound = NULL;
    }
    *phtFoundNode = NULL;
    return FALSE;
}

