// CHelpLib.h
// Exported symbols from CHelpLib DLL
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      Unknown history!
//      09/09/14 Refactor to store defs in individual headers.
//      09/12/14 Naming convention modifications
//

#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#ifdef __cplusplus
extern "C" {  
#endif

#include "CommonInclude.h"
#include "MemFunctions.h"

// hashtable node
typedef struct _hashTableNode {
    //BOOL fOccupied;     // FALSE = not occupied
    CHL_key chlKey;
    CHL_val chlVal;
    int iKeySize;
    int iValSize;
    struct _hashTableNode *pnext;
}HT_NODE;

// Structure that defines the iterator for the hashtable
// Callers can use this to iterate through the hashtable
// and get all (key,value) pairs one-by-one
typedef struct _hashtableIterator {
    int opType;
    int nCurIndex;              // current position in the main bucket
    HT_NODE *phtCurNodeInList;  // current position in the sibling list
}CHL_HT_ITERATOR;

// hashtable itself
typedef struct _hashtable {
    CHL_KEYTYPE keyType;
    CHL_VALTYPE valType;
    BOOL fValIsInHeap;
    HT_NODE **phtNodes;
    int nTableSize;
    HANDLE hMuAccess;

    // Access methods
    HRESULT (*Destroy)(struct _hashtable* phtable);

    HRESULT (*Insert)(
        struct _hashtable* phtable, 
        PVOID const pvkey, 
        int keySize, 
        PVOID pvVal, 
        int iValSize);

    HRESULT (*Find)(
        struct _hashtable* phtable, 
        PVOID const pvkey, 
        int keySize, 
        PVOID pvVal, 
        PINT pvalsize,
        BOOL fGetPointerOnly);

    HRESULT (*Remove)(struct _hashtable* phtable, PVOID const pvkey, int keySize);

    HRESULT (*InitIterator)(CHL_HT_ITERATOR *pItr);
    HRESULT (*GetNext)(
        struct _hashtable* phtable, 
        CHL_HT_ITERATOR *pItr, 
        PVOID const pvKey, 
        PINT pkeysize,
        PVOID pvVal, 
        PINT pvalsize,
        BOOL fGetPointerOnly);

    void (*Dump)(struct _hashtable* phtable);

}CHL_HTABLE, *PCHL_HTABLE;

// -------------------------------------------
// Functions exported

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
DllExpImp HRESULT CHL_DsCreateHT(
    _Inout_ CHL_HTABLE **pHTableOut, 
    _In_ int nEstEntries, 
    _In_ CHL_KEYTYPE keyType, 
    _In_ CHL_VALTYPE valType, 
    _In_opt_ BOOL fValInHeapMem);

DllExpImp HRESULT CHL_DsDestroyHT(_In_ CHL_HTABLE *phtable);

DllExpImp HRESULT CHL_DsInsertHT(
    _In_ CHL_HTABLE *phtable, 
    _In_ PVOID const pvkey, 
    _In_ int keySize, 
    _In_ PVOID pvVal, 
    _In_ int iValSize);

DllExpImp HRESULT CHL_DsFindHT(
    _In_ CHL_HTABLE *phtable, 
    _In_ PVOID const pvkey, 
    _In_ int keySize, 
    _Inout_opt_ PVOID pvVal, 
    _Inout_opt_ PINT pvalsize,
    _In_opt_ BOOL fGetPointerOnly);

DllExpImp HRESULT CHL_DsRemoveHT(_In_ CHL_HTABLE *phtable, _In_ PVOID const pvkey, _In_ int keySize);

DllExpImp HRESULT CHL_DsInitIteratorHT(_In_ CHL_HT_ITERATOR *pItr);
DllExpImp HRESULT CHL_DsGetNextHT(
    _In_ CHL_HTABLE *phtable, 
    _In_ CHL_HT_ITERATOR *pItr, 
    _Inout_opt_ PVOID const pvKey, 
    _Inout_opt_ PINT pkeysize,
    _Inout_opt_ PVOID pvVal, 
    _Inout_opt_ PINT pvalsize,
    _In_opt_ BOOL fGetPointerOnly);

DllExpImp int CHL_DsGetNearestSizeIndexHT(_In_ int maxNumberOfEntries);
DllExpImp void CHL_DsDumpHT(_In_ CHL_HTABLE *phtable);

#ifdef __cplusplus
}
#endif

#endif // _HASHTABLE_H
