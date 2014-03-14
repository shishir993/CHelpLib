This library is to make the life of C/Win32 programmer easier by providing function wrappers such as read line from stdout, simple memory alloc functions. More details below. This repo also includes UnitTests that are tests for the functions in CHelpLib DLL.

-------------------
Functions exported
-------------------

-- MemFunctions
DllExpImp BOOL fChlMmAlloc(__out void **pvAddr, __in size_t uSizeBytes, OPTIONAL DWORD *pdwError);
DllExpImp void vChlMmFree(__in void **pvToFree);

-- IOFunctions
// Read characters from the stdin stream until newline is received or 
// dwBufSize-1 characters are read
DllExpImp BOOL fChlIoReadLineFromStdin(__in DWORD dwBufSize, __out WCHAR *psBuffer);

-- StringFunctions
// Given the full path to a file, returns only the filename component(with extension, if any)
DllExpImp WCHAR* pszChlSzGetFilenameFromPath(WCHAR *pwsFilepath, int numCharsInput);

-- DataStructure Functions
-- Hastable functions
// Create a hashtable with the specified properties
DllExpImp BOOL fChlDsCreateHT(CHL_HTABLE **pHTableOut, int nKeyToTableSize, int keyType, int valType);

// Destroy the hashtable. Memory is freed. Do not reference the CHL_HTABLE pointer.
DllExpImp BOOL fChlDsDestroyHT(CHL_HTABLE *phtable);

// Insert a new item into hashtable. If key already exists, then the value just gets updated.
DllExpImp BOOL fChlDsInsertHT (CHL_HTABLE *phtable, void *pvkey, int keySize, void *pval, int valSize);

// Find a key in the hashtable.
DllExpImp BOOL fChlDsFindHT   (CHL_HTABLE *phtable, void *pvkey, int keySize, __out void *pval, __out int *pvalsize);

// Remove the item which has the specified key from hashtable.
DllExpImp BOOL fChlDsRemoveHT (CHL_HTABLE *phtable, void *pvkey, int keySize);

// Get an iterator pointer that be used for the call to fChlDsGetNextHT() to 
// iterate through all items in the hashtable.
DllExpImp BOOL fChlDsInitIteratorHT(CHL_HT_ITERATOR *pItr);

// Given an iterator that was retrieved earlier, this function returns the next
// hashtable item's key and value.
DllExpImp BOOL fChlDsGetNextHT(CHL_HTABLE *phtable, CHL_HT_ITERATOR *pItr, 
                            __out void *pkey, __out int *pkeysize,
                            __out void *pval, __out int *pvalsize);

// Given the estimated max hashtable entries, returns the best size 
// table index to be used when calling fChlDsCreateHT().
DllExpImp int  fChlDsGetNearestTableSizeIndex(int maxNumberOfEntries);

// Prints out all name:value pairs present in the hashtable.
DllExpImp void fChlDsDumpHT(CHL_HTABLE *phtable);

-- General functions
// Check if addition of two signed integers can cause an overflow.
DllExpImp BOOL fChlGnIsOverflowINT(int a, int b);

// Check if addition of two unsigned integers can cause an overflow.
DllExpImp BOOL fChlGnIsOverflowUINT(unsigned int a, unsigned int b);
