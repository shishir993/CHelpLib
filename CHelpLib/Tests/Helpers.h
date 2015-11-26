#pragma once

#ifdef _DEBUG

#define logDebug(M, ...)    Helpers::logHelper(false, L"[D] " M, ##__VA_ARGS__)
#define logInfo(M, ...)     Helpers::logHelper(false, L"[I] " M, ##__VA_ARGS__)
#define logWarn(M, ...)     Helpers::logHelper(false, L"[W] " M, ##__VA_ARGS__)
#define logError(M, ...)    Helpers::logHelper(false, L"[E] " M, ##__VA_ARGS__)

#else

#define logDebug(M, ...)
#define logInfo(M, ...)     Helpers::logHelper(L"[I] " M, ##__VA_ARGS__)
#define logWarn(M, ...)     Helpers::logHelper(L"[W] " M, ##__VA_ARGS__)
#define logError(M, ...)    Helpers::logHelper(L"[E] " M, ##__VA_ARGS__)

#endif

namespace Helpers
{

#ifdef _DEBUG
void logHelper(_In_ bool fDebugLog, _In_z_ PCWSTR pszFmt, ...);
#else
void logHelper(_In_z_ PCWSTR pszFmt, ...);
#endif

};

