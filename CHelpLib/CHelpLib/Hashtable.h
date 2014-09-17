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

// hashtable key types
#define HT_KEY_STR      10
#define HT_KEY_DWORD    11

// hashtable value types
#define HT_VAL_DWORD    12
#define HT_VAL_PTR      13
#define HT_VAL_STR      14

typedef int HT_KEYTYPE;
typedef int HT_VALTYPE;

// hashtable node
typedef struct _hashTableNode {
    //BOOL fOccupied;     // FALSE = not occupied
    union _key key;
    union _val val;
    int keysize;
    int valsize;
    struct _hashTableNode *pnext;
}HT_NODE;

// hashtable itself
typedef struct _hashtable {
    HT_KEYTYPE htKeyType;
    HT_VALTYPE htValType;
    BOOL fValIsInHeap;
    HT_NODE **phtNodes;
    int nTableSize;
    HANDLE hMuAccess;
}CHL_HTABLE;

// Structure that defines the iterator for the hashtable
// Callers can use this to iterate through the hashtable
// and get all (key,value) pairs one-by-one
typedef struct _hashtableIterator {
    int opType;
    int nCurIndex;              // current position in the main bucket
    HT_NODE *phtCurNodeInList;  // current position in the sibling list
}CHL_HT_ITERATOR;

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
    _In_ int keyType, 
    _In_ int valType, 
    _In_opt_ BOOL fValInHeapMem);

DllExpImp HRESULT CHL_DsDestroyHT(_In_ CHL_HTABLE *phtable);

DllExpImp HRESULT CHL_DsInsertHT(
    _In_ CHL_HTABLE *phtable, 
    _In_ PVOID const pvkey, 
    _In_ int keySize, 
    _In_ PVOID pval, 
    _In_ int valSize);

DllExpImp HRESULT CHL_DsFindHT(
    _In_ CHL_HTABLE *phtable, 
    _In_ PVOID const pvkey, 
    _In_ int keySize, 
    _Inout_opt_ PVOID pval, 
    _Inout_opt_ PINT pvalsize);

DllExpImp HRESULT CHL_DsRemoveHT(_In_ CHL_HTABLE *phtable, _In_ PVOID const pvkey, _In_ int keySize);

DllExpImp HRESULT CHL_DsInitIteratorHT(_In_ CHL_HT_ITERATOR *pItr);
DllExpImp HRESULT CHL_DsGetNextHT(
    _In_ CHL_HTABLE *phtable, 
    _In_ CHL_HT_ITERATOR *pItr, 
    _Inout_opt_ PVOID const pkey, 
    _Inout_opt_ PINT pkeysize,
    _Inout_opt_ PVOID pval, 
    _Inout_opt_ PINT pvalsize);

DllExpImp int CHL_DsGetNearestSizeIndexHT(_In_ int maxNumberOfEntries);
DllExpImp void CHL_DsDumpHT(_In_ CHL_HTABLE *phtable);

#ifdef __cplusplus
}
#endif

#endif // _HASHTABLE_H
