
// LinkedList.h
// Contains functions that implement a linked list data structure
// Shishir Bhat (http://www.shishirbhat.com)
// History
//      04/05/14 Initial version
//      09/09/14 Refactor to store defs in individual headers.
//      09/12/14 Naming convention modifications
//      2016/01/30 Replace compare function with the standard CHL_CompareFn type
//

#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H

#ifdef __cplusplus
extern "C" {  
#endif

#include "Defines.h"


typedef struct _LlNode {
    CHL_VAL chlVal;
    struct _LlNode *pleft;
    struct _LlNode *pright;
}LLNODE, *PLLNODE;

// Foward declare the iterator struct
struct _linkedListIterator;

typedef struct _LinkedList CHL_LLIST, *PCHL_LLIST;
typedef struct _linkedListIterator CHL_ITERATOR_LL;
struct _LinkedList {
    int nCurNodes;
    int nMaxNodes;
    CHL_VALTYPE valType;
    PLLNODE pHead;
    PLLNODE pTail;

    // Access methods

    HRESULT (*Insert)
    (
        PCHL_LLIST pLList,
        PCVOID pvVal, 
        int iValSize
    );

    HRESULT(*Remove)
    (
        PCHL_LLIST pLList,
        PCVOID pvValToFind,
        BOOL fStopOnFirstFind,
        CHL_CompareFn pfnComparer
    );

    HRESULT (*RemoveAt)
    (
        PCHL_LLIST pLList,
        int iIndexToRemove, 
        PVOID pvValOut,
        PINT piValBufSize,
        BOOL fGetPointerOnly
    );

    HRESULT (*RemoveAtItr)(CHL_ITERATOR_LL *pItr);

    HRESULT (*Peek)
    (
        PCHL_LLIST pLList,
        int iIndexToPeek, 
        PVOID pvValOut,
        PINT piValBufSize,
        BOOL fGetPointerOnly
    );

    HRESULT (*Find)
    (
        PCHL_LLIST pLList,
        PCVOID pvValToFind, 
        CHL_CompareFn pfnComparer,
        PVOID pvValOut,
        PINT piValBufSize,
        BOOL fGetPointerOnly
    );

    HRESULT (*FindItr)
    (
        PCHL_LLIST pLList,
        PCVOID pvValToFind,
        CHL_CompareFn pfnComparer,
        CHL_ITERATOR_LL* pItr
    );


    HRESULT (*Destroy)(PCHL_LLIST pLList);

    BOOL (*IsEmpty)(PCHL_LLIST pLList);

    HRESULT (*InitIterator)(PCHL_LLIST pList, CHL_ITERATOR_LL *pItr);

};

// Structure that defines the iterator for the linked list.
// Callers can use this to iterate through the linked list
// and get values one-by-one.
struct _linkedListIterator {
    PLLNODE pCur;       // current position in the list
    PCHL_LLIST pMyList; // Pointer to the linked list to work on

    HRESULT (*MoveNext)(CHL_ITERATOR_LL *pItr);

    HRESULT (*GetCurrent)(
        CHL_ITERATOR_LL *pItr,
        PVOID pvVal,
        PINT piValSize,
        BOOL fGetPointerOnly);
};

// -------------------------------------------
// Functions exported

DllExpImp HRESULT CHL_DsCreateLL(_Out_ PCHL_LLIST *ppLList, _In_ CHL_VALTYPE valType, _In_opt_ int nEstEntries);

// CHL_DsInsertLL()
// Inserts an element into the linked list. Always inserts at the tail.
// Insertion is an O(1) operation.
//      pLList: Pointer to a linked list object that was returned by a successful call
//              to CHL_DsCreateLL.
//      pvVal: The value or pointer to the value to be stored. All primitive types are passed by 
//              value(like: CHL_DsInsertLL(pListObj, (PVOID)intValue, 0). All other complex
//              types are passed by reference(their address) and they are stored in the heap.
//      iValSize: Size in bytes of the value. Optional for primitive types, mandatory for others.
//
DllExpImp HRESULT CHL_DsInsertLL
(
    _In_ PCHL_LLIST pLList,
    _In_ PCVOID pvVal,
    _In_opt_ int iValSize
);

DllExpImp HRESULT CHL_DsRemoveLL
(
    _In_ PCHL_LLIST pLList, 
    _In_ PCVOID pvValToFind, 
    _In_ BOOL fStopOnFirstFind, 
    _In_ CHL_CompareFn pfnComparer
);

DllExpImp HRESULT CHL_DsRemoveAtLL
(
    _In_ PCHL_LLIST pLList, 
    _In_ int iIndexToRemove, 
    _Inout_opt_ PVOID pvValOut, 
    _Inout_opt_ PINT piValBufSize,
    _In_opt_ BOOL fGetPointerOnly
);

DllExpImp HRESULT CHL_DsRemoveAtItrLL(_Inout_ CHL_ITERATOR_LL *pItr);

DllExpImp HRESULT CHL_DsPeekAtLL
(
    _In_ PCHL_LLIST pLList, 
    _In_ int iIndexToPeek, 
    _Inout_opt_ PVOID pvValOut, 
    _Inout_opt_ PINT piValBufSize,
    _In_opt_ BOOL fGetPointerOnly
);

DllExpImp HRESULT CHL_DsFindLL
(
    _In_ PCHL_LLIST pLList, 
    _In_ PCVOID pvValToFind, 
    _In_ CHL_CompareFn pfnComparer,
    _Inout_opt_ PVOID pvValOut,
    _Inout_opt_ PINT piValBufSize,
    _In_opt_ BOOL fGetPointerOnly
);

DllExpImp HRESULT CHL_DsFindItrLL
(
    _In_ PCHL_LLIST pLList,
    _In_ PCVOID pvValToFind,
    _In_opt_ CHL_CompareFn pfnComparer,
    _Out_ CHL_ITERATOR_LL* pItr
);

DllExpImp HRESULT CHL_DsDestroyLL(_In_ PCHL_LLIST pLList);

DllExpImp BOOL CHL_DsIsEmptyLL(_In_ PCHL_LLIST pLList);

DllExpImp HRESULT CHL_DsInitIteratorLL(_In_ PCHL_LLIST pList, _Out_ CHL_ITERATOR_LL *pItr);

DllExpImp HRESULT CHL_DsMoveNextLL(_Inout_ CHL_ITERATOR_LL *pItr);

DllExpImp HRESULT CHL_DsGetCurrentLL
(
    _In_ CHL_ITERATOR_LL *pItr,
    _Inout_opt_ PVOID pvVal,
    _Inout_opt_ PINT piValSize,
    _In_opt_ BOOL fGetPointerOnly
);

#ifdef __cplusplus
}
#endif

#endif // _LINKEDLIST_H
