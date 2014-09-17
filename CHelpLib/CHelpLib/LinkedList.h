
// LinkedList.h
// Contains functions that implement a linked list data structure
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      04/05/14 Initial version
//      09/09/14 Refactor to store defs in individual headers.
//      09/12/14 Naming convention modifications
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
    PVOID pval;
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

DllExpImp HRESULT CHL_DsCreateLL(_Out_ PCHL_LLIST *ppLList, _In_ LL_VALTYPE valType, _In_opt_ int nEstEntries);
DllExpImp HRESULT CHL_DsInsertLL(_In_ PCHL_LLIST pLList, _In_ PVOID pval, _In_ int valsize);
DllExpImp HRESULT CHL_DsRemoveLL(
    _In_ PCHL_LLIST pLList, 
    _In_ PVOID pvValToFind, 
    _In_ BOOL fStopOnFirstFind, 
    _In_ BOOL (*pfnComparer)(PVOID, PVOID), 
    _Inout_opt_ PVOID *ppval);

DllExpImp HRESULT CHL_DsRemoveAtLL(_In_ PCHL_LLIST pLList, _In_ int iIndexToRemove, _Inout_opt_ PVOID *ppval);
DllExpImp HRESULT CHL_DsPeekAtLL(_In_ PCHL_LLIST pLList, _In_ int iIndexToPeek, _Inout_ PVOID *ppval);
DllExpImp HRESULT CHL_DsFindLL(
    _In_ PCHL_LLIST pLList, 
    _In_ PVOID pvValToFind, 
    _In_ BOOL (*pfnComparer)(PVOID, PVOID), 
    _Inout_opt_ PVOID *ppval);

DllExpImp HRESULT CHL_DsDestroyLL(_In_ PCHL_LLIST pLList);

#ifdef __cplusplus
}
#endif

#endif // _LINKEDLIST_H
