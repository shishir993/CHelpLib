
#include "InternalDefines.h"
#include "BinarySearchTree.h"


typedef struct _InputKeyValue {
    CHL_KEYTYPE keyType;
    CHL_VALTYPE valType;
    PVOID pvKey;
    int iKeySize;
    PVOID pvVal;
    int iValSize;
} CHL_INPUT_KV, *PCHL_INPUT_KV;

static PBSTNODE s_DeleteSubTree
(
    _In_ PCHL_BSTREE pbst,
    _In_ PBSTNODE pCurNode
);

static HRESULT s_Insert
(
    _Out_ PBSTNODE *ppNewNode,
    _In_ PCHL_BSTREE pbstree, 
    _In_ PBSTNODE pCurNode,
    _In_ PCHL_INPUT_KV pInputKeyValue
);

static PBSTNODE s_Find
(
    _In_ PCHL_BSTREE pbstree,
    _In_ PBSTNODE pCurNode,
    _In_ PCHL_INPUT_KV pInputKeyValue
);

static HRESULT s_NewNode
(
    _Outptr_ PBSTNODE *ppNewNode,
    _In_ PCHL_INPUT_KV pInputKeyValue
);

static UINT s_GetTreeSize(_In_opt_ PBSTNODE pnode);

// --------------------------------------------------------
// Public function definitions

HRESULT CHL_DsCreateBST
(
    _Out_ PCHL_BSTREE pbst,
    _In_ CHL_KEYTYPE keyType,
    _In_ CHL_VALTYPE valType,
    _In_ CHL_CompareFn pfnKeyCompare,
    _In_opt_ BOOL fValInHeapMem
)
{
    HRESULT hr = S_OK;

    ASSERT(IS_VALID_CHL_VALTYPE(valType));
    ASSERT(IS_VALID_CHL_KEYTYPE(keyType));

    if (!pbst || !pfnKeyCompare ||
        IS_INVALID_CHL_KEYTYPE(keyType) || IS_INVALID_CHL_VALTYPE(valType))
    {
        hr = E_INVALIDARG;
        goto fend;
    }

    memset(pbst, 0, sizeof(*pbst));

    pbst->keyType = keyType;
    pbst->valType = valType;
    pbst->fValIsInHeap = fValInHeapMem;
    pbst->fnKeyCompare = pfnKeyCompare;

    pbst->Create = CHL_DsCreateBST;
    pbst->Destroy = CHL_DsDestroyBST;
    pbst->Insert = CHL_DsInsertBST;
    pbst->Find = CHL_DsFindBST;
    pbst->InitIterator = CHL_DsInitIteratorBST;
    pbst->GetNext = CHL_DsGetNextBST;

fend:
    return hr;
}

HRESULT CHL_DsDestroyBST(_In_ PCHL_BSTREE pbst)
{
    if (IS_INVALID_CHL_KEYTYPE(pbst->keyType) || IS_INVALID_CHL_VALTYPE(pbst->valType))
    {
        return E_INVALIDARG;
    }

    // Delete nodes in DFS order
    pbst->pRoot = s_DeleteSubTree(pbst, pbst->pRoot);
    ASSERT(s_GetTreeSize(pbst->pRoot) == 0);
    ASSERT(pbst->pRoot == NULL);
    memset(pbst, 0, sizeof(*pbst));
    return S_OK;
}

HRESULT CHL_DsInsertBST
(
    _In_ PCHL_BSTREE pbst,
    _In_ PCVOID pvkey,
    _In_ int iKeySize,
    _In_ PCVOID pvVal,
    _In_ int iValSize
)
{
    HRESULT hr = S_OK;
    CHL_KEYTYPE keyType;
    CHL_INPUT_KV inputKV;

    keyType = pbst->keyType;
    if (iKeySize <= 0 && FAILED(_GetKeySize(pvkey, keyType, &iKeySize)))
    {
        hr = E_FAIL;
        goto fend;
    }

    if (iValSize <= 0 && FAILED(_GetValSize(pvVal, pbst->valType, &iValSize)))
    {
        hr = E_FAIL;
        goto fend;
    }

    inputKV.keyType = pbst->keyType;
    inputKV.valType = pbst->valType;
    inputKV.pvKey = pvkey;
    inputKV.iKeySize = iKeySize;
    inputKV.pvVal = pvVal;
    inputKV.iValSize = iValSize;
    hr = s_Insert(&pbst->pRoot, pbst, pbst->pRoot, &inputKV);

fend:
    return hr;
}

HRESULT CHL_DsFindBST
(
    _In_ PCHL_BSTREE pbst,
    _In_ PCVOID pvkey,
    _In_ int iKeySize,
    _Inout_opt_ PVOID pvVal,
    _Inout_opt_ PINT pValsize,
    _In_opt_ BOOL fGetPointerOnly
)
{
    HRESULT hr = S_OK;
    PBSTNODE pFoundNode = NULL;

    CHL_INPUT_KV kvInput;

    if (iKeySize <= 0 && FAILED(_GetKeySize(pvkey, pbst->keyType, &iKeySize)))
    {
        logerr("%s(): Keysize unspecified or unable to determine.", __FUNCTION__);
        hr = E_INVALIDARG;
        goto fend;
    }

    // Copy the key info (only key info is required for s_find)
    kvInput.pvKey = pvkey;
    kvInput.iKeySize = iKeySize;

    pFoundNode = s_Find(pbst, pbst->pRoot, &kvInput);
    if (!pFoundNode)
    {
        hr = E_NOT_SET;
        goto fend;
    }

    if (pvVal)
    {
        // Ensure sufficient buffer is provided in this case.
        if (!fGetPointerOnly)
        {
            hr = _EnsureSufficientValBuf(
                &pFoundNode->chlVal,
                ((pValsize && (*pValsize > 0)) ? *pValsize : sizeof(PVOID)),
                pValsize);
        }

        if (SUCCEEDED(hr))
        {
            hr = _CopyValOut(&pFoundNode->chlVal, pbst->valType, pvVal, fGetPointerOnly);
        }
    }

fend:
    return hr;
}

HRESULT CHL_DsInitIteratorBST
(
    _Out_ PCHL_BST_ITERATOR pItr,
    _In_ PCHL_BSTREE pbst,
    _In_ CHL_BstIterationType itrType
)
{
    if ((itrType <= BstIterationType_PreOrder) || (itrType >= BstIterationType_PostOrder))
    {
        return E_INVALIDARG;
    }

    pItr->itType = itrType;
    pItr->pbst = pbst;
    pItr->pCur = pbst->pRoot;
    return S_OK;
}

HRESULT CHL_DsGetNextBST
(
    _In_ PCHL_BST_ITERATOR pItr,
    _Inout_opt_ PCVOID pvKey,
    _Inout_opt_ PINT pKeysize,
    _Inout_opt_ PVOID pvVal,
    _Inout_opt_ PINT pValsize,
    _In_opt_ BOOL fGetPointerOnly
)
{
    DBG_UNREFERENCED_PARAMETER(pItr);
    DBG_UNREFERENCED_PARAMETER(pvKey);
    DBG_UNREFERENCED_PARAMETER(pKeysize);
    DBG_UNREFERENCED_PARAMETER(pvVal);
    DBG_UNREFERENCED_PARAMETER(pValsize);
    DBG_UNREFERENCED_PARAMETER(fGetPointerOnly);
    return E_NOTIMPL;
}

// --------------------------------------------------------
// Private function definitions

PBSTNODE s_DeleteSubTree
(
    _In_ PCHL_BSTREE pbst,
    _In_ PBSTNODE pCurNode
)
{
    if (!pCurNode)
    {
        return NULL;
    }

    if (pCurNode->pLeft)
    {
        pCurNode->treeSize -= s_GetTreeSize(pCurNode->pLeft);
        pCurNode->pLeft = s_DeleteSubTree(pbst, pCurNode->pLeft);
    }

    if (pCurNode->pRight)
    {
        pCurNode->treeSize -= s_GetTreeSize(pCurNode->pRight);
        pCurNode->pRight = s_DeleteSubTree(pbst, pCurNode->pRight);
    }

    ASSERT(pCurNode->treeSize == 1);
    ASSERT(pCurNode->pLeft == NULL);
    ASSERT(pCurNode->pRight == NULL);

    _DeleteVal(&pCurNode->chlVal, pbst->valType, pbst->fValIsInHeap);
    _DeleteKey(&pCurNode->chlKey, pbst->keyType);
    CHL_MmFree(&pCurNode);

    return NULL;
}

HRESULT s_Insert(_Out_ PBSTNODE *ppNewNode, _In_ PCHL_BSTREE pbstree, _In_ PBSTNODE pCurNode, _In_ PCHL_INPUT_KV pInputKeyValue)
{
    HRESULT hr = S_OK;
    HRESULT hrTemp = S_OK;
    PBSTNODE pNewNode = NULL;

    PVOID pvExistingKey;

    if (pCurNode == NULL)
    {
        hr = s_NewNode(&pNewNode, pInputKeyValue);
        goto fend;
    }

    // Extract key from current node for comparison against user-specified key
    hrTemp = _CopyKeyOut(&pCurNode->chlKey, pbstree->keyType, &pvExistingKey, TRUE /*fGetPointerOnly*/);
    ASSERT(SUCCEEDED(hrTemp));

    // Based on key comparison, the new key-value should be inserted in left/right subtree
    int cmp = pbstree->fnKeyCompare(pInputKeyValue->pvKey, pvExistingKey);
    if (cmp < 0)
    {
        hr = s_Insert(&pCurNode->pLeft, pbstree, pCurNode->pLeft, pInputKeyValue);
    }
    else if (cmp > 0)
    {
        hr = s_Insert(&pCurNode->pRight, pbstree, pCurNode->pRight, pInputKeyValue);
    }
    else
    {
        CHL_VAL val;
        hr = _CopyValIn(&val, pbstree->valType, pInputKeyValue->pvVal, pInputKeyValue->iValSize);
        if (SUCCEEDED(hr))
        {
            // Successfully constructed CHL_VAL, now replace the cur node's value with new one
            _DeleteVal(&pCurNode->chlVal, pbstree->valType, pbstree->fValIsInHeap);
            CopyMemory(&pCurNode->chlVal, &val, sizeof(val));
        }
    }

    pCurNode->treeSize = s_GetTreeSize(pCurNode->pLeft) + s_GetTreeSize(pCurNode->pRight) + 1;

    pNewNode = pCurNode;

fend:
    *ppNewNode = pNewNode;
    return hr;
}

PBSTNODE s_Find
(
    _In_ PCHL_BSTREE pbstree,
    _In_ PBSTNODE pCurNode,
    _In_ PCHL_INPUT_KV pInputKeyValue
)
{
    PVOID pvExistingKey;
    HRESULT hrTemp = S_OK;

    if (pCurNode == NULL)
    {
        return NULL;
    }

    hrTemp = _CopyKeyOut(&pCurNode->chlKey, pbstree->keyType, &pvExistingKey, TRUE /*fGetPointerOnly*/);
    ASSERT(SUCCEEDED(hrTemp));

    int cmp = pbstree->fnKeyCompare(pInputKeyValue->pvKey, pvExistingKey);
    if (cmp < 0)
    {
        return s_Find(pbstree, pCurNode->pLeft, pInputKeyValue);
    }

    if (cmp > 0)
    {
        return s_Find(pbstree, pCurNode->pRight, pInputKeyValue);
    }

    return pCurNode;
}

HRESULT s_NewNode(_Outptr_ PBSTNODE *ppNewNode, _In_ PCHL_INPUT_KV pInputKeyValue)
{
    HRESULT hr;
    PBSTNODE pNewNode;

    hr = CHL_MmAlloc(&pNewNode, sizeof(BSTNODE), NULL);
    if (FAILED(hr))
    {
        goto fend;
    }

    pNewNode->treeSize = 1;

    hr = _CopyKeyIn(&pNewNode->chlKey, pInputKeyValue->keyType, pInputKeyValue->pvKey, pInputKeyValue->iKeySize);
    if (SUCCEEDED(hr))
    {
        hr = _CopyValIn(&pNewNode->chlVal, pInputKeyValue->valType, pInputKeyValue->pvVal, pInputKeyValue->iValSize);
    }

    if (SUCCEEDED(hr))
    {
        *ppNewNode = pNewNode;
    }
    else
    {
        CHL_MmFree(&pNewNode);
        *ppNewNode = NULL;
    }

fend:
    return hr;
}

UINT s_GetTreeSize(_In_opt_ PBSTNODE pnode)
{
    return ((pnode != NULL) ? pnode->treeSize : 0);
}
