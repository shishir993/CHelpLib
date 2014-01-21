This library is to make the life of C/Win32 programmer easier by providing function wrappers such as read line from stdout, simple memory alloc functions. More details below. This repo also includes UnitTests that are tests for the functions in CHelpLib DLL.

-------------------
Functions exported
-------------------

-- MemFunctions
BOOL ChlfMemAlloc(OUT void **pvAddr, IN size_t uSizeBytes, OPTIONAL DWORD *pdwError);
void ChlvMemFree(IN void **pvToFree);

-- IOFunctions
BOOL ChlfReadLineFromStdin(OUT TCHAR *psBuffer, IN DWORD dwBufSize);

-- StringFunctions
WCHAR* Chl_GetFilenameFromPath(WCHAR *pwsFilepath, int numCharsInput);

-- Hastable functions
BOOL HT_fCreate(CHL_HTABLE **pHTableOut, int nKeyToTableSize, int keyType, int valType);
BOOL HT_fDestroy(CHL_HTABLE *phtable);

BOOL HT_fInsert (CHL_HTABLE *phtable, void *pvkey, int keySize, void *pval, int valSize);
BOOL HT_fFind   (CHL_HTABLE *phtable, void *pvkey, int keySize, OUT void *pval, OUT int *pvalsize);
BOOL HT_fRemove (CHL_HTABLE *phtable, void *pvkey, int keySize);

BOOL HT_fInitIterator(CHL_HT_ITERATOR *pItr);
BOOL HT_fGetNext(CHL_HTABLE *phtable, CHL_HT_ITERATOR *pItr, 
                            OUT void *pkey, OUT int *pkeysize,
                            OUT void *pval, OUT int *pvalsize);

void HT_vDumpTable(CHL_HTABLE *phtable);

-- General functions
BOOL IsOverflowINT(int a, int b);
BOOL IsOverflowUINT(unsigned int a, unsigned int b);