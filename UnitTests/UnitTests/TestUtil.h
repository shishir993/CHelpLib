
#ifndef _TEST_UTIL_H
#define _TEST_UTIL_H

#include "Common.h"

// nChars is number of characters including null-terminator.
HRESULT CreateRandString(_Out_ PSTR pszBuffer, _In_ int nChars);

// nChars is number of characters including null-terminator.
HRESULT CreateRandString(_Out_ PWSTR pszBuffer, _In_ int nChars);

#endif // _TEST_UTIL_H
