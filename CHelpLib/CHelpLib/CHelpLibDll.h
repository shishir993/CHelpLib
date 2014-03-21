
// CHelpLib.h
// Exported symbols from CHelpLib DLL
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      06/23/13 Initial version
//      03/13/13 Function name modifications. Naming convention.

#ifndef _CHELPLIBDLL_H
#define _CHELPLIBDLL_H

#ifdef __cplusplus
extern "C" {  
#endif

#ifdef CHELPLIB_EXPORTS
#define DllExpImp __declspec( dllexport )
#else
#define DllExpImp __declspec( dllimport )
#endif // CHELPLIB_EXPORTS

// Defines

// hashtable key types
#define HT_KEY_STR      10
#define HT_KEY_DWORD    11

// hashtable value types
#define HT_VAL_DWORD    12
#define HT_VAL_PTR      13
#define HT_VAL_STR      14

typedef int HT_KEYTYPE;
typedef int HT_VALTYPE;

// Structures
union _key {
    BYTE *skey;
    DWORD dwkey;
};

union _val {
    DWORD dwVal;
    void *pval;
    BYTE *pbVal;
};

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

// ** Functions exported **

// MemFunctions
DllExpImp BOOL fChlMmAlloc(__out void **pvAddr, __in size_t uSizeBytes, OPTIONAL DWORD *pdwError);
DllExpImp void vChlMmFree(__in void **pvToFree);

// IOFunctions
DllExpImp BOOL fChlIoReadLineFromStdin(__in DWORD dwBufSize, __out WCHAR *psBuffer);

// StringFunctions
DllExpImp WCHAR* pszChlSzGetFilenameFromPath(WCHAR *pwsFilepath, int numCharsInput);

// DataStructure Functions
// Hastable functions

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
DllExpImp BOOL fChlDsCreateHT(CHL_HTABLE **pHTableOut, int nEstEntries, int keyType, int valType, BOOL fValInHeapMem);
DllExpImp BOOL fChlDsDestroyHT(CHL_HTABLE *phtable);

DllExpImp BOOL fChlDsInsertHT (CHL_HTABLE *phtable, void *pvkey, int keySize, void *pval, int valSize);
DllExpImp BOOL fChlDsFindHT   (CHL_HTABLE *phtable, void *pvkey, int keySize, __out void *pval, __out int *pvalsize);
DllExpImp BOOL fChlDsRemoveHT (CHL_HTABLE *phtable, void *pvkey, int keySize);

DllExpImp BOOL fChlDsInitIteratorHT(CHL_HT_ITERATOR *pItr);
DllExpImp BOOL fChlDsGetNextHT(CHL_HTABLE *phtable, CHL_HT_ITERATOR *pItr, 
                            __out void *pkey, __out int *pkeysize,
                            __out void *pval, __out int *pvalsize);

DllExpImp int  iChlDsGetNearestTableSizeIndex(int maxNumberOfEntries);
DllExpImp void vChlDsDumpHT(CHL_HTABLE *phtable);

// General functions
DllExpImp BOOL fChlGnIsOverflowINT(int a, int b);
DllExpImp BOOL fChlGnIsOverflowUINT(unsigned int a, unsigned int b);

#ifdef __cplusplus
}
#endif

#endif // _CHELPLIBDLL_H
