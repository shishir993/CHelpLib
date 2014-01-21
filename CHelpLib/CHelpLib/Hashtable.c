

#include "Hashtable.h"

#define HT_MUTEX_NAME   (TEXT("CHL_MUTEX_NAME"))
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

static void vDeleteNode(int ktype, int vtype, HT_NODE *pnode);

// File-local Functions
static DWORD _hashu(int tablesize, DWORD key);
static DWORD _hashs(int tablesize, const BYTE *key, int keysize);
static BOOL fOwnMutex(HANDLE hMutex);
static BOOL fIsDuplicateKey(union _key *pLeftKey, void *pRightKey, HT_KEYTYPE keytype, int keysize);
static BOOL fIsDuplicateVal(union _val *pLeftVal, void *pRightVal, HT_VALTYPE valtype, int valsize);
static BOOL fUpdateNodeVal(HT_NODE *pnode, void *pval, HT_VALTYPE valType, int valSize);
static BOOL fFindKeyInList(HT_NODE *pFirstHTNode, void *pvkey, int keysize, HT_KEYTYPE keyType, 
                            __out HT_NODE **phtFoundNode, __out OPTIONAL HT_NODE **phtPrevFound);

static void vCopyKeyOut(union _key *pukey, HT_KEYTYPE keyType, __out void *pkey);
static void vCopyValOut(union _val *puval, HT_VALTYPE valType, __out void *pval);


DWORD _hashu(int tablesize, DWORD key)
{
    ASSERT(tablesize > 0);
    
    return key % tablesize;
}


DWORD _hashs(int tablesize, const BYTE *key, int keysize)
{
    DWORD hash = 5381;
    BYTE c;
    UINT i = 0;

    ASSERT(tablesize > 0);

    //
    // hash function from
    // http://www.cse.yorku.ca/~oz/hash.html
    //

    while ((c = *key) && i < keysize)
    {
      hash = ((hash << 5) + hash) + c; // hash * 33 + c
      ++i;
      ++key;
    }

    return (DWORD)(hash % tablesize);
}

BOOL HT_fCreate(CHL_HTABLE **pHTableOut, int nKeyToTableSize, int keyType, int valType)
{   
    int newTableSize = 0;
    CHL_HTABLE *pnewtable = NULL;
    WCHAR wsMutexName[32];

    unsigned int uiRand;

    // validate parameters
    if(!pHTableOut) return FALSE;
    if(nKeyToTableSize < 0 || nKeyToTableSize >= _countof(hashSizes)) return FALSE;
    if(keyType != HT_KEY_STR && keyType != HT_KEY_DWORD) return FALSE;
    if(valType < HT_VAL_DWORD && valType > HT_VAL_STR) return FALSE;
    
    // Create a unique number to be appended to the mutex name
    if( rand_s(&uiRand) != 0 )
    {
        logerr("Unable to get a random number");
        return FALSE;
    }
    swprintf_s(wsMutexName, 32, L"%s_%08X", HT_MUTEX_NAME, uiRand);

    // 
    newTableSize = hashSizes[nKeyToTableSize];
    if((pnewtable = (CHL_HTABLE*)malloc(sizeof(CHL_HTABLE))) == NULL)
    {
        logerr("HT_fCreate(): malloc() ");
        goto error_return;
    }
    
    pnewtable->nTableSize = newTableSize;
    pnewtable->htKeyType = keyType;
    pnewtable->htValType = valType;
    pnewtable->hMuAccess = CreateMutex( NULL,   // default security descriptor
                                        FALSE,  // I do not need to be the owner!!
                                        wsMutexName);
    if(pnewtable->hMuAccess == NULL)
        goto error_return;
    
    pnewtable->phtNodes = (HT_NODE**)calloc(1, newTableSize * sizeof(HT_NODE*));
    if(pnewtable->phtNodes == NULL)
    {
        logerr("HT_fCreate(): calloc() ");
        goto error_return;
    }

    *pHTableOut = pnewtable;
    return TRUE;
    
    error_return:
    if(pnewtable->phtNodes) free(pnewtable->phtNodes);
    if(pnewtable) free(pnewtable);
    *pHTableOut = NULL;
    return FALSE;
    
}// HT_fCreate()


BOOL HT_fDestroy(CHL_HTABLE *phtable)
{   
    int i = 0;
    int limit;
    int ktype, vtype;
    HT_NODE **phtnodes = NULL;
    HT_NODE *pcurnode = NULL;
    HT_NODE *pnextnode = NULL;

    if(!phtable) return FALSE;
    
    if(!fOwnMutex(phtable->hMuAccess))
        return FALSE;
    
    ktype = phtable->htKeyType;
    vtype = phtable->htValType;
    limit = phtable->nTableSize;
    phtnodes = phtable->phtNodes;
    
    if(!phtnodes)
    {
        free(phtable);
        return TRUE;
    }
    
    for( ; i < limit; ++i)
    {
        pcurnode = phtnodes[i];
        while(pcurnode)
        {
            pnextnode = pcurnode->pnext;
            vDeleteNode(ktype, vtype, pcurnode);
            pcurnode = pnextnode;
        }// while pcurnode
        
    }// for
    
    //ReleaseMutex(phtable->hMuAccess); // ToDo: should we release or not??
    CloseHandle(phtable->hMuAccess);

    DBG_MEMSET(phtable, sizeof(CHL_HTABLE));
    free(phtable);
    return TRUE;
    
}// HT_Destroy()


static void vDeleteNode(int ktype, int vtype, HT_NODE *pnode)
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

    if(vtype == HT_VAL_STR && pnode->val.pbVal) free(pnode->val.pbVal);
    
    DBG_MEMSET(pnode, sizeof(HT_NODE));
    free(pnode);
    
    return;
}// vDeleteNode()


BOOL HT_fInsert(CHL_HTABLE *phtable, void *pvkey, int keySize, void *pval, int valSize)
{
    DWORD index;
    HT_NODE *pnewnode = NULL;
    
    BOOL fLocked = FALSE;
    
    BYTE *pszkey = NULL;
    DWORD dwkey = 0;

    HT_KEYTYPE keytype = 0;

    ASSERT(phtable && pvkey);
    ASSERT(pval && valSize > 0);

    keytype = phtable->htKeyType;
    
    // validate parameters
    if(!phtable) return FALSE;
    if(!pvkey) return FALSE;
    if(keySize <= 0) return FALSE;
    if(!pval) return FALSE;
    if(valSize <= 0) return FALSE;

    // create a new hashtable node
    if( (pnewnode = (HT_NODE*)malloc(sizeof(HT_NODE))) == NULL )
    { logerr("HT_fInsert(): malloc() "); goto error_return; }
    
    switch(keytype)
    {
        case HT_KEY_STR:
            if(!ChlfMemAlloc((void**)&pnewnode->key.skey, keySize, NULL))
            { logerr("HT_fInsert(): ChlfMemAlloc() "); goto delete_newnode_return; }
            memcpy(pnewnode->key.skey, pvkey, keySize);
            pnewnode->keysize = keySize;
            pszkey = (BYTE*)pvkey;
            break;
        
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
            if(!ChlfMemAlloc((void**)&pnewnode->val.pbVal, valSize, NULL))
            { logerr("HT_fInsert(): ChlfMemAlloc() "); goto delete_newnode_return; }
            memcpy(pnewnode->val.pbVal, pval, valSize);
            break;

        default:
            logerr("HT_fInsert(): Invalid valType %d\n", phtable->htValType);
            goto delete_newnode_return;
    }
    pnewnode->valsize = valSize;

    // insert into hashtable
    if(!fOwnMutex(phtable->hMuAccess))
        goto error_return;
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
        if(fIsDuplicateKey(&(phtable->phtNodes[index]->key), pvkey, keytype, keySize))
        {
            if(fIsDuplicateVal(&(phtable->phtNodes[index]->val), pval, phtable->htValType, valSize))
            {
                // Both key and value are same
                // Delete the newly created node and return
                vDeleteNode(phtable->htKeyType, phtable->htValType, pnewnode);
                ReleaseMutex(phtable->hMuAccess);
                return TRUE;
            }

            // Same key but different value, just update node with new value
            // NOTE: Old value will be lost!!
            if(!fUpdateNodeVal(phtable->phtNodes[index], pval, phtable->htValType, valSize))
            {
                ReleaseMutex(phtable->hMuAccess);
                return FALSE;
            }
            ReleaseMutex(phtable->hMuAccess);
            return TRUE;
        }
    }

    // connect to head
    pnewnode->pnext = phtable->phtNodes[index];
    phtable->phtNodes[index] = pnewnode;
    
    ReleaseMutex(phtable->hMuAccess);
    return TRUE;
    
delete_newnode_return:
    vDeleteNode(phtable->htKeyType, phtable->htValType, pnewnode);
    
error_return:
    if(fLocked && !ReleaseMutex(phtable->hMuAccess))
        logerr("HT_fInsert(): error_return mutex unlock ");
    return FALSE;
    
}// HT_fInsert()


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


BOOL HT_fFind(CHL_HTABLE *phtable, void *pvkey, int keySize, __out void *pval, __out int *pvalsize)
{
    int index = 0;
    HT_NODE *phtFoundNode = NULL;
    BOOL fLocked = FALSE;
    BYTE *pszkey = NULL;
    DWORD dwkey = 0;

    HT_KEYTYPE keytype;

    ASSERT(phtable);
    ASSERT(pvkey && keySize > 0);
    ASSERT(pval);

    keytype = phtable->htKeyType;

    switch(keytype)
    {
        case HT_KEY_STR:
            pszkey = (BYTE*)pvkey;
            break;
        
        case HT_KEY_DWORD:
            dwkey = *((DWORD*)pvkey);
            break;
        
        default: logerr("Incorrect keytype %d", keytype); goto not_found;
    }
    
    if(!fOwnMutex(phtable->hMuAccess))
        goto not_found;
    fLocked = TRUE;
    
    ASSERT(phtable->nTableSize > 0);
    if(keytype == HT_KEY_DWORD)
        index = _hashu(phtable->nTableSize, dwkey);
    else
        index = _hashs(phtable->nTableSize, pszkey, keySize);
    
    phtFoundNode = phtable->phtNodes[index];
    if(!phtFoundNode) goto not_found;

    if(!fFindKeyInList(phtFoundNode, pvkey, keySize, keytype, 
                        &phtFoundNode, NULL))
        goto not_found;

    ASSERT(phtFoundNode);

    if(pval)
    {
        vCopyValOut(&phtFoundNode->val, phtable->htValType, pval);
    }

    if(pvalsize) *pvalsize = phtFoundNode->valsize;

    ReleaseMutex(phtable->hMuAccess);
    return TRUE;
    
    not_found:
    if(pvalsize) *pvalsize = 0;
    if(fLocked && !ReleaseMutex(phtable->hMuAccess))
        logerr("HT_fFind(): not_found mutex unlock ");
    return FALSE;
    
}// HT_fFind()


BOOL HT_fRemove(CHL_HTABLE *phtable, void *pvkey, int keySize)
{   
    int index = 0;
    HT_NODE *phtFoundNode = NULL;
    HT_NODE *phtPrevFound = NULL;
    
    BOOL fLocked = FALSE;
    
    BYTE *pszkey = NULL;
    DWORD dwkey = 0;
    
    HT_KEYTYPE keytype = 0;

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
        
        default: logerr("Incorrect keytype %d", phtable->htKeyType); goto error_return;
    }
    
    if(!fOwnMutex(phtable->hMuAccess))
        goto error_return;
    fLocked = TRUE;
    
    ASSERT(phtable->nTableSize > 0);
    if(keytype == HT_KEY_DWORD)
        index = _hashu(phtable->nTableSize, dwkey);
    else
        index = _hashs(phtable->nTableSize, pszkey, keySize);
    
    phtFoundNode = phtable->phtNodes[index];
    if(!phtFoundNode) goto error_return;

    if(!fFindKeyInList(phtFoundNode, pvkey, keySize, keytype, 
                            &phtFoundNode, &phtPrevFound))
        goto error_return;

    ASSERT(phtFoundNode);

    // re-arrange pnext pointers
    if(phtPrevFound)
        phtPrevFound->pnext = phtFoundNode->pnext;
    else
        phtable->phtNodes[index] = NULL;

    vDeleteNode(keytype, phtable->htValType, phtFoundNode);
    
    ReleaseMutex(phtable->hMuAccess);
    return TRUE;
    
    error_return:
    if(fLocked && !ReleaseMutex(phtable->hMuAccess))
        logerr("HT_fRemove(): error_return mutex unlock ");
    return FALSE;
    
}// HT_fRemove()


DllExpImp BOOL HT_fInitIterator(CHL_HT_ITERATOR *pItr)
{
    if(!pItr) return FALSE;

    pItr->opType = HT_ITR_FIRST;
    pItr->nCurIndex = 0;
    pItr->phtCurNodeInList = NULL;
    return TRUE;
}


BOOL HT_fGetNext(CHL_HTABLE *phtable, CHL_HT_ITERATOR *pItr, 
                    OUT void *pkey, OUT int *pkeysize,
                    OUT void *pval, OUT int *pvalsize)
{
    
    int indexIterator;

    ASSERT(phtable && pItr);

    if(!phtable) return FALSE;
    if(!pItr) return FALSE;

    // Trivial check to see if iterator was initialized or not
    switch(pItr->opType)
    {
        case HT_ITR_FIRST:
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
                return FALSE;  // reached End of table
            }
            break;  // out of switch
        }

        case HT_ITR_NEXT:
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
                    return FALSE;  // reached End of table
                }
            }
            break;  // out of switch
        }

        default: logerr("Iterator invalid opType %x", pItr->opType); return FALSE;
    }

    if(pkey)
        vCopyKeyOut(&pItr->phtCurNodeInList->key, phtable->htKeyType, pkey);
    if(pkeysize) 
        *pkeysize = pItr->phtCurNodeInList->keysize;

    if(pval)
        vCopyValOut(&pItr->phtCurNodeInList->val, phtable->htValType, pval);
    if(pvalsize)
        *pvalsize = pItr->phtCurNodeInList->valsize;

    return TRUE;
}


int HT_iGetNearestTableSizeIndex(int maxNumberOfEntries)
{
    int index = 0;

    while(index < (_countof(hashSizes)-1) && maxNumberOfEntries > hashSizes[index])
        ++index;

    return index;
}


void HT_vDumpTable(CHL_HTABLE *phtable)
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
    
    if(!fOwnMutex(phtable->hMuAccess))
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
        logerr("HT_vDumpTable(): mutex unlock ");  
    return;
    
}// HT_vDumpTable


BOOL fOwnMutex(HANDLE hMutex)
{
    int nTries = 0;

    ASSERT(hMutex && hMutex != INVALID_HANDLE_VALUE);

    while(nTries < 10)
    {
        switch(WaitForSingleObject(hMutex, 500))
        {
            case WAIT_OBJECT_0:
                goto got_it;

            case WAIT_ABANDONED: // todo
                return FALSE;

            case WAIT_TIMEOUT:
                break;

            case WAIT_FAILED:
                return FALSE;
        }
        ++nTries;
    }

    got_it:
    if(nTries == 10)
        return FALSE;

    return TRUE;
}


BOOL fIsDuplicateKey(union _key *pLeftKey, void *pRightKey, HT_KEYTYPE keytype, int keysize)
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

BOOL fIsDuplicateVal(union _val *pLeftVal, void *pRightVal, HT_VALTYPE valtype, int valsize)
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

BOOL fUpdateNodeVal(HT_NODE *pnode, void *pval, HT_VALTYPE valType, int valSize)
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

BOOL fFindKeyInList(HT_NODE *pFirstHTNode, void *pvkey, int keysize, HT_KEYTYPE keyType, 
                            __out HT_NODE **phtFoundNode, __out OPTIONAL HT_NODE **phtPrevFound)
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
        if(fIsDuplicateKey(&pcurNode->key, pvkey, keyType, pcurNode->keysize))
            break;

        prevNode = pcurNode;
        pcurNode = pcurNode->pnext;
    }

    if(pcurNode)
    {
        if(phtPrevFound) *phtPrevFound = prevNode;
        *phtFoundNode = pcurNode;
        return TRUE;
    }

    if(phtPrevFound) *phtPrevFound = NULL;
    *phtFoundNode = NULL;
    return FALSE;
}


void vCopyKeyOut(union _key *pukey, HT_KEYTYPE keyType, OUT void *pkey)
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


void vCopyValOut(union _val *puval, HT_VALTYPE valType, OUT void *pval)
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
            void **ppval = (void**)pval;
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
