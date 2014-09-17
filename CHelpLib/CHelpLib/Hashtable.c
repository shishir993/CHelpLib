

#include "Hashtable.h"
#include "General.h"

#define HT_ITR_FIRST    0xdeedbeed
#define HT_ITR_NEXT     0xabcddcba

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
static DWORD _hashu(_In_ int tablesize, _In_ DWORD key);
static DWORD _hashs(_In_ int tablesize, _In_bytecount_c_(keysize) const BYTE *key, _In_ int keysize);
static void _DeleteNode(_In_ int ktype, _In_ int vtype, _In_ HT_NODE *pnode, _In_ BOOL fFreeVal);
static BOOL _IsDuplicateKey(_In_ union _key *pLeftKey, _In_ PVOID pRightKey, _In_ HT_KEYTYPE keytype, _In_ int keysize);
static BOOL _IsDuplicateVal(_In_ union _val *pLeftVal, _In_ PVOID pRightVal, _In_ HT_VALTYPE valtype, _In_ int valsize);
static BOOL _UpdateNodeVal(_In_ HT_NODE *pnode, _In_ PVOID pval, _In_ HT_VALTYPE valType, _In_ int valSize);

static BOOL _FindKeyInList(
    _In_ HT_NODE *pFirstHTNode, 
    _In_ PVOID pvkey, 
    _In_ int keysize, 
    _In_ HT_KEYTYPE keyType, 
    _Out_ HT_NODE **phtFoundNode, 
    _Out_opt_ HT_NODE **phtPrevFound);

static void _CopyKeyOut(_In_ union _key *pukey, _In_ HT_KEYTYPE keyType, _In_ PVOID pkey);
static void _CopyValOut(_In_ union _val *puval, _In_ HT_VALTYPE valType, _In_ PVOID pval);


DWORD _hashu(_In_ int tablesize, _In_ DWORD key)
{
    ASSERT(tablesize > 0);
    
    return key % tablesize;
}

DWORD _hashs(_In_ int tablesize, _In_bytecount_c_(keysize) const BYTE *key, int keysize)
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
    while ((c != 0) && (i < keysize))
    {
      hash = ((hash << 5) + hash) + c; // hash * 33 + c
      c = key[i++];
    }

    return (DWORD)(hash % tablesize);
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
    _Inout_ CHL_HTABLE **pHTableOut, 
    _In_ int nEstEntries, 
    _In_ int keyType, 
    _In_ int valType, 
    _In_opt_ BOOL fValInHeapMem)
{   
    int newTableSize = 0;
    CHL_HTABLE *pnewtable = NULL;
    WCHAR wsMutexName[32];

    unsigned int uiRand;

    HRESULT hr = S_OK;

    // validate parameters
    if((pHTableOut == NULL) || 
        (nEstEntries < 0) || 
        (keyType != HT_KEY_STR && keyType != HT_KEY_DWORD) ||
        (valType < HT_VAL_DWORD && valType > HT_VAL_STR))
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
    swprintf_s(wsMutexName, 32, L"%s_%08X", HT_MUTEX_NAME, uiRand);

    // 
    newTableSize = hashSizes[CHL_DsGetNearestSizeIndexHT(nEstEntries)];
    if((pnewtable = (CHL_HTABLE*)malloc(sizeof(CHL_HTABLE))) == NULL)
    {
        logerr("fChlDsCreateHT(): malloc() ");
        hr = E_OUTOFMEMORY;
        goto error_return;
    }
    
    pnewtable->fValIsInHeap = fValInHeapMem;
    pnewtable->nTableSize = newTableSize;
    pnewtable->htKeyType = keyType;
    pnewtable->htValType = valType;

    pnewtable->hMuAccess = CreateMutex( NULL, FALSE, wsMutexName);
    if(pnewtable->hMuAccess == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto error_return;
    }
    
    pnewtable->phtNodes = (HT_NODE**)calloc(1, newTableSize * sizeof(HT_NODE*));
    if(pnewtable->phtNodes == NULL)
    {
        logerr("fChlDsCreateHT(): calloc() ");
        hr = E_OUTOFMEMORY;
        goto error_return;
    }

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
    
}// fChlDsCreateHT()

HRESULT CHL_DsDestroyHT(_In_ CHL_HTABLE *phtable)
{   
    int i;
    int maxNodes;
    int ktype, vtype;
    BOOL fInHeap;
    HT_NODE **phtnodes = NULL;
    HT_NODE *pcurnode = NULL;
    HT_NODE *pnextnode = NULL;

    HRESULT hr = S_OK;
    if(!phtable)
    {
        hr = E_INVALIDARG;
        goto error_return;  // hashtable isn't destroyed
    }
    
    hr = CHL_GnOwnMutex(phtable->hMuAccess);
    if(FAILED(hr))
    {
        goto error_return;  // hashtable isn't destroyed
    }

    phtnodes = phtable->phtNodes;
    if(phtnodes != NULL)
    {
        ktype = phtable->htKeyType;
        vtype = phtable->htValType;
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
    return hr;

error_return:
    return hr;
}// HT_Destroy()

HRESULT CHL_DsInsertHT(
    _In_ CHL_HTABLE *phtable, 
    _In_ PVOID const pvkey, 
    _In_ int keySize, 
    _In_ PVOID pval, 
    _In_ int valSize)
{
    DWORD index;
    HT_NODE *pnewnode = NULL;
    
    BOOL fLocked = FALSE;
    
    BYTE *pszkey = NULL;
    DWORD dwkey = 0;

    HT_KEYTYPE keytype;

    HRESULT hr = S_OK;

    ASSERT(phtable && pvkey);
    ASSERT(pval && valSize > 0);
    
    // validate parameters
    if(!phtable || !pvkey || !pval ||
        (keySize <= 0) || (valSize <= 0))
    {
        hr = E_INVALIDARG;
        goto error_return;
    }

    // create a new hashtable node
    if( (pnewnode = (HT_NODE*)malloc(sizeof(HT_NODE))) == NULL )
    { 
        logerr("fChlDsInsertHT(): malloc() ");
        hr = E_OUTOFMEMORY;
        goto error_return;
    }
    
    keytype = phtable->htKeyType;
    switch(keytype)
    {
        case HT_KEY_STR:
        {
            hr = CHL_MmAlloc((void**)&pnewnode->key.skey, keySize, NULL);
            if(FAILED(hr))
            { 
                logerr("fChlDsInsertHT(): fChlMmAlloc() "); 
                goto delete_newnode_return;
            }
            memcpy(pnewnode->key.skey, pvkey, keySize);
            pnewnode->keysize = keySize;
            pszkey = (BYTE*)pvkey;
            break;
        }
        
        case HT_KEY_DWORD:
            pnewnode->keysize = keySize;
            pnewnode->key.dwkey = *((DWORD*)pvkey);
            dwkey = *((DWORD*)pvkey);
            break;
        
        default: logwarn("Incorrect keytype %d", phtable->htKeyType); goto delete_newnode_return;
    }
    
    switch(phtable->htValType)
    {
        case HT_VAL_DWORD:
            pnewnode->val.dwVal = *((DWORD*)pval);
            break;

        case HT_VAL_PTR:
            pnewnode->val.pval = pval;
            break;

        case HT_VAL_STR:
            hr = CHL_MmAlloc((void**)&pnewnode->val.pbVal, valSize, NULL);
            if(FAILED(hr))
            {
                logerr("fChlDsInsertHT(): fChlMmAlloc() ");
                goto delete_newnode_return;
            }
            memcpy(pnewnode->val.pbVal, pval, valSize);
            break;

        default:
            logerr("fChlDsInsertHT(): Invalid valType %d\n", phtable->htValType);
            hr = E_UNEXPECTED;
            goto delete_newnode_return;
    }
    pnewnode->valsize = valSize;

    // insert into hashtable
    hr = CHL_GnOwnMutex(phtable->hMuAccess);
    if(FAILED(hr))
    {
        goto error_return;
    }
    fLocked = TRUE;

    ASSERT(phtable->nTableSize > 0);
    if(keytype == HT_KEY_DWORD)
        index = _hashu(phtable->nTableSize, dwkey);
    else
        index = _hashs(phtable->nTableSize, pszkey, keySize);
    
    // verify that duplicate values are not inserted (same key and value)
    if(phtable->phtNodes[index])
    {
        // We have the key hashing onto a index that is already populated
        if(_IsDuplicateKey(&(phtable->phtNodes[index]->key), pvkey, keytype, keySize))
        {
            if(_IsDuplicateVal(&(phtable->phtNodes[index]->val), pval, phtable->htValType, valSize))
            {
                // Both key and value are same
                // Delete the newly created node and return
                _DeleteNode(phtable->htKeyType, phtable->htValType, pnewnode, phtable->fValIsInHeap);
                goto done;
            }
            else
            {
                // Same key but different value, just update node with new value
                // NOTE: Old value will be lost!!
                if(!_UpdateNodeVal(phtable->phtNodes[index], pval, phtable->htValType, valSize))
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
    _DeleteNode(phtable->htKeyType, phtable->htValType, pnewnode, phtable->fValIsInHeap);
    
error_return:
    if(fLocked && !ReleaseMutex(phtable->hMuAccess))
    {
        logerr("fChlDsInsertHT(): error_return mutex unlock ");
    }
    return hr;
    
}// fChlDsInsertHT()


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
    _In_ CHL_HTABLE *phtable, 
    _In_ PVOID const pvkey, 
    _In_ int keySize, 
    _Inout_opt_ PVOID pval, 
    _Inout_opt_ PINT pvalsize)
{
    int index = 0;
    HT_NODE *phtFoundNode = NULL;
    BOOL fLocked = FALSE;
    BYTE *pszkey = NULL;
    DWORD dwkey = 0;

    HT_KEYTYPE keytype;
    HRESULT hr = S_OK;

    ASSERT(phtable);
    ASSERT(pvkey && keySize > 0);

    keytype = phtable->htKeyType;
    switch(keytype)
    {
        case HT_KEY_STR:
            pszkey = (BYTE*)pvkey;
            break;
        
        case HT_KEY_DWORD:
            dwkey = *((DWORD*)pvkey);
            break;
        
        default: 
            logerr("Incorrect keytype %d", keytype); 
            hr = E_UNEXPECTED; 
            goto not_found;
    }
    
    hr = CHL_GnOwnMutex(phtable->hMuAccess);
    if(FAILED(hr))
    {
        goto not_found;
    }
    fLocked = TRUE;
    
    ASSERT(phtable->nTableSize > 0);
    if(keytype == HT_KEY_DWORD)
        index = _hashu(phtable->nTableSize, dwkey);
    else
        index = _hashs(phtable->nTableSize, pszkey, keySize);
    
    phtFoundNode = phtable->phtNodes[index];
    if(!phtFoundNode || 
        !_FindKeyInList(phtFoundNode, pvkey, keySize, keytype, 
                        &phtFoundNode, NULL))
    {
        hr = E_NOT_SET;
        goto not_found;
    }

    ASSERT(phtFoundNode);
    if(pval)
    {
        _CopyValOut(&phtFoundNode->val, phtable->htValType, pval);
    }

    if(pvalsize)
    {
        *pvalsize = phtFoundNode->valsize;
    }
    ReleaseMutex(phtable->hMuAccess);
    return hr;
    
not_found:
    if(pvalsize) *pvalsize = 0;
    if(fLocked && !ReleaseMutex(phtable->hMuAccess))
        logerr("fChlDsFindHT(): not_found mutex unlock ");
    return hr;
    
}// fChlDsFindHT()


HRESULT CHL_DsRemoveHT(_In_ CHL_HTABLE *phtable, _In_ PVOID const pvkey, _In_ int keySize)
{   
    int index = 0;
    HT_NODE *phtFoundNode = NULL;
    HT_NODE *phtPrevFound = NULL;
    
    BOOL fLocked = FALSE;
    
    BYTE *pszkey = NULL;
    DWORD dwkey = 0;
    
    HT_KEYTYPE keytype;

    HRESULT hr = S_OK;

    ASSERT(phtable && pvkey);
    ASSERT(keySize > 0);
    
    keytype = phtable->htKeyType;
    switch(keytype)
    {
        case HT_KEY_STR:
            pszkey = (BYTE*)pvkey;
            break;
        
        case HT_KEY_DWORD:
            dwkey = *((DWORD*)pvkey);
            break;
        
        default:
            logerr("Incorrect keytype %d", keytype); 
            hr = E_UNEXPECTED; 
            goto error_return;
    }
    
    hr = CHL_GnOwnMutex(phtable->hMuAccess);
    if(FAILED(hr))
    {
        goto error_return;
    }
    fLocked = TRUE;
    
    ASSERT(phtable->nTableSize > 0);
    if(keytype == HT_KEY_DWORD)
        index = _hashu(phtable->nTableSize, dwkey);
    else
        index = _hashs(phtable->nTableSize, pszkey, keySize);
    
    phtFoundNode = phtable->phtNodes[index];
    if(!phtFoundNode ||
        !_FindKeyInList(phtFoundNode, pvkey, keySize, keytype, 
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

    _DeleteNode(keytype, phtable->htValType, phtFoundNode, phtable->fValIsInHeap);
    
    ReleaseMutex(phtable->hMuAccess);
    return hr;
    
error_return:
    if(fLocked && !ReleaseMutex(phtable->hMuAccess))
        logerr("fChlDsRemoveHT(): error_return mutex unlock ");
    return hr;
    
}// fChlDsRemoveHT()

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
    _In_ CHL_HTABLE *phtable, 
    _In_ CHL_HT_ITERATOR *pItr, 
    _Inout_opt_ PVOID pkey, 
    _Inout_opt_ PINT pkeysize,
    _Inout_opt_ PVOID pval, 
    _Inout_opt_ PINT pvalsize)
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
        if(pkey)
            _CopyKeyOut(&pItr->phtCurNodeInList->key, phtable->htKeyType, pkey);
        if(pkeysize) 
            *pkeysize = pItr->phtCurNodeInList->keysize;

        if(pval)
            _CopyValOut(&pItr->phtCurNodeInList->val, phtable->htValType, pval);
        if(pvalsize)
            *pvalsize = pItr->phtCurNodeInList->valsize;
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


void CHL_DsDumpHT(_In_ CHL_HTABLE *phtable)
{   
    int i;
    int nNodes = 0;
    int uTableSize = -1;
    BOOL fLocked = FALSE;
    
    int keyType = -1;
    int valType = -1;
    
    HT_NODE **phtNodes = NULL;
    HT_NODE *phtCurNode = NULL;

    ASSERT(phtable && phtable->nTableSize > 0);

    uTableSize = phtable->nTableSize;
    keyType = phtable->htKeyType;
    valType = phtable->htValType;
    
    if(FAILED(CHL_GnOwnMutex(phtable->hMuAccess)))
        goto fend;
    fLocked = TRUE;
    
    phtNodes = phtable->phtNodes;
    
    printf("-----------------------------------\n");
    printf("Hashtable Dump::\nkey  :  valsize  :  val\n");
    printf("-----------------------------------\n");
    
    if(!phtNodes)
    {
        printf("Hastable empty");
        goto fend;
    }
    
    for(i = 0; i < uTableSize; ++i)
    {
        if((phtCurNode = phtNodes[i]) == NULL) continue;
        
        while(phtCurNode)
        {
            ++nNodes;
            
            // print the key
            switch(keyType)
            {
                case HT_KEY_STR:
                {
                    int chars = 0;
                    while(chars < phtCurNode->keysize)
                        putchar(phtCurNode->key.skey[chars++]);
                    putchar(':');
                    break;
                }

                case HT_KEY_DWORD:printf("%u:", phtCurNode->key.dwkey); break;
                default: printf("Invalid keytype %d:", keyType); break;
            }

            // print the valsize
            printf("%d:", phtCurNode->valsize);

            // print the value
            if(valType == HT_VAL_STR)
                printf("[%s]\n", phtCurNode->val.pbVal);
            else if(valType == HT_VAL_DWORD)
                printf("[%u]\n", phtCurNode->val.dwVal);
            else if(valType == HT_VAL_PTR)
                printf("[0x%p]\n", phtCurNode->val.pbVal);
            else
                printf("Invalid valtype %d\n", valType);
            phtCurNode = phtCurNode->pnext;
        
        }// while phtCurNode
        
    }// for i
    
    printf("Hashtable stats:\n");
    printf("Total size     : %u\n", uTableSize);
    printf("Occupied       : %d\n", nNodes);
        
    fend:
    if(fLocked && !ReleaseMutex(phtable->hMuAccess))
        logerr("CHL_DsDumpHT(): mutex unlock ");  
    return;
    
}// CHL_DsDumpHT

void _DeleteNode(int ktype, int vtype, HT_NODE *pnode, BOOL fFreeVal)
{
    ASSERT(pnode);

    // Assume mutex is in our ownership
    
    switch(ktype)
    {
        case HT_KEY_DWORD: break;
        
        case HT_KEY_STR:
            if(pnode->key.skey) free(pnode->key.skey);
            else {ASSERT(FALSE);}
            break;
        
        default: logerr("Incorrect keytype %d", ktype); ASSERT(FALSE);
    }

    if(vtype == HT_VAL_STR && pnode->val.pbVal)
    {
        free(pnode->val.pbVal);
    }
    else if(fFreeVal)
    {
        free(pnode->val.pval);
    }
    
    DBG_MEMSET(pnode, sizeof(HT_NODE));
    free(pnode);
    
    return;
}// _DeleteNode()

BOOL _IsDuplicateKey(union _key *pLeftKey, PVOID pRightKey, HT_KEYTYPE keytype, int keysize)
{
    switch(keytype)
    {
        case HT_KEY_DWORD:
                return (pLeftKey->dwkey == *((DWORD*)pRightKey));

        case HT_KEY_STR:
            return (memcmp(pLeftKey->skey, pRightKey, keysize) == 0);
    }
    return FALSE;
}

BOOL _IsDuplicateVal(union _val *pLeftVal, PVOID pRightVal, HT_VALTYPE valtype, int valsize)
{
    switch(valtype)
    {
        case HT_VAL_DWORD:
            return (pLeftVal->dwVal == *((DWORD*)pRightVal));

        case HT_VAL_PTR:
            return (pLeftVal->pval == pRightVal);

        case HT_VAL_STR:
            return (memcmp(pLeftVal->pbVal, pRightVal, valsize) == 0);
    }
    return FALSE;
}

BOOL _UpdateNodeVal(HT_NODE *pnode, PVOID pval, HT_VALTYPE valType, int valSize)
{
    switch(valType)
    {
        case HT_VAL_DWORD:
            pnode->val.dwVal = *((DWORD*)pval);
            pnode->valsize = valSize;
            break;

        case HT_VAL_PTR:
            pnode->val.pval = pval;
            pnode->valsize = valSize;
            break;

        case HT_VAL_STR:
        {
            if(valSize != pnode->valsize)
            {
                pnode->val.pbVal = (BYTE*)realloc(pnode->val.pbVal, valSize);
                if(!(pnode->val.pbVal))
                    return FALSE;
            }
            memcpy(pnode->val.pbVal, pval, valSize);
            break;
        }

        default: return FALSE;
    }

    return TRUE;
}

BOOL _FindKeyInList(
    _In_ HT_NODE *pFirstHTNode, 
    _In_ PVOID pvkey, 
    _In_ int keysize, 
    _In_ HT_KEYTYPE keyType, 
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
        if(pcurNode->keysize != keysize)
        {
            prevNode = pcurNode;
            pcurNode = pcurNode->pnext;
            continue;
        }

        if(_IsDuplicateKey(&pcurNode->key, pvkey, keyType, pcurNode->keysize))
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

void _CopyKeyOut(_In_ union _key *pukey, _In_ HT_KEYTYPE keyType, _In_ PVOID pkey)
{
    ASSERT(pukey && pkey);
    switch(keyType)
    {
        case HT_KEY_DWORD:
        {
            DWORD *pdwkey = (DWORD*)pkey;
            *pdwkey = pukey->dwkey;
            break;
        }

        case HT_KEY_STR:
        {
            BYTE **ppbkey = (BYTE**)pkey;
            *ppbkey = pukey->skey;
            break;
        }

        default: logerr("Invalid keytype %d\n", keyType); break;
    }
}

void _CopyValOut(_In_ union _val *puval, _In_ HT_VALTYPE valType, _In_ PVOID pval)
{
    ASSERT(puval && pval);
    switch(valType)
    {
        case HT_VAL_DWORD:
        {
            DWORD *pdwval = (DWORD*)pval;
            *pdwval = puval->dwVal;
            break;
        }
        
        case HT_VAL_PTR:
        {
            PVOID *ppval = (void**)pval;
            *ppval = puval->pval;
            break;
        }

        case HT_VAL_STR:
        {
            BYTE **ppbval = (BYTE**)pval;
            *ppbval = puval->pbVal;
            break;
        }

        default: logerr("Invalid valType %d\n", valType); break;
    }
}
