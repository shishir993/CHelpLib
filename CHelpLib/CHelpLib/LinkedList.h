
// LinkedList.h
// Contains functions that implement a linked list data structure
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      04/05/14 Initial version
//      09/09/14 Refactor to store defs in individual headers.
//

#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H

#ifdef __cplusplus
extern "C" {  
#endif

#include "CommonInclude.h"

// Custom error codes
#define CHLE_LLIST_EINVAL   17100
#define CHLE_LLIST_VALTYPE  17101

// LinkedList value types
#define LL_VAL_BYTE     20
#define LL_VAL_UINT     21
#define LL_VAL_DWORD    22
#define LL_VAL_LONGLONG 23
#define LL_VAL_PTR      24

typedef int LL_VALTYPE;

union _nodeval {
    BYTE bval;
    UINT uval;
    DWORD dwval;
    LONGLONG longlongval;
    void *pval;
};

typedef struct _LlNode {
    union _nodeval nodeval;
    DWORD dwValSize;
    struct _LlNode *pleft;
    struct _LlNode *pright;
}LLNODE, *PLLNODE;

typedef struct _LinkedList {
    int nCurNodes;
    int nMaxNodes;
    LL_VALTYPE valType;
    PLLNODE pHead;
    PLLNODE pTail;
    HANDLE hMuAccess;
}CHL_LLIST, *PCHL_LLIST;

// -------------------------------------------
// Functions exported

DllExpImp BOOL fChlDsCreateLL(__out PCHL_LLIST *ppLList, LL_VALTYPE valType, OPTIONAL int nEstEntries);
DllExpImp BOOL fChlDsInsertLL(PCHL_LLIST pLList, void *pval, int valsize);
DllExpImp BOOL fChlDsRemoveLL(PCHL_LLIST pLList, void *pvValToFind, BOOL fStopOnFirstFind, BOOL (*pfnComparer)(void*, void*), __out OPTIONAL void **ppval);
DllExpImp BOOL fChlDsRemoveAtLL(PCHL_LLIST pLList, int iIndexToRemove, __out OPTIONAL void **ppval);
DllExpImp BOOL fChlDsPeekAtLL(PCHL_LLIST pLList, int iIndexToPeek, __out void **ppval);
DllExpImp BOOL fChlDsFindLL(PCHL_LLIST pLList, __in void *pvValToFind, BOOL (*pfnComparer)(void*, void*), __out OPTIONAL void **ppval);
DllExpImp BOOL fChlDsDestroyLL(PCHL_LLIST pLList);

#ifdef __cplusplus
}
#endif

#endif // _LINKEDLIST_H
