
#include "stdafx.h"
#include "Helpers.h"

namespace Helpers
{

#ifdef _DEBUG
void logHelper(_In_ bool fDebugLog, _In_z_ PCWSTR pszFmt, ...)
#else
void logHelper(_In_z_ PCWSTR pszFmt, ...)
#endif
{
    wchar_t s_szBuffer[1024];

    va_list arg;
    va_start(arg, pszFmt);
    HRESULT hr = StringCchVPrintf(s_szBuffer, ARRAYSIZE(s_szBuffer), pszFmt, arg);
    va_end(arg);

    if (SUCCEEDED(hr))
    {
        wprintf(L"%s\n", s_szBuffer);

#ifdef _DEBUG
        if (fDebugLog)
        {
            OutputDebugString(s_szBuffer);
        }
#endif
    }
}

}