#pragma once

#ifdef _DEBUG

#define logDebug(M, ...)    Helpers::logHelper(true, L"[D] " M, ##__VA_ARGS__)
#define logInfo(M, ...)     Helpers::logHelper(false, L"[I] " M, ##__VA_ARGS__)
#define logWarn(M, ...)     Helpers::logHelper(false, L"[W] " M, ##__VA_ARGS__)
#define logError(M, ...)    Helpers::logHelper(false, L"[E] " M, ##__VA_ARGS__)

#else

#define logDebug(M, ...)
#define logInfo(M, ...)     Helpers::logHelper(L"[I] " M, ##__VA_ARGS__)
#define logWarn(M, ...)     Helpers::logHelper(L"[W] " M, ##__VA_ARGS__)
#define logError(M, ...)    Helpers::logHelper(L"[E] " M, ##__VA_ARGS__)

#endif

#define LOG_FUNC_ENTRY      logInfo(L"Entry: %s()", __FUNCTIONW__)
#define LOG_FUNC_EXIT       logInfo(L"Exit : %s()\n", __FUNCTIONW__)

#define ISVALID_HANDLE(h)   (((h) != NULL) && ((h) != INVALID_HANDLE_VALUE))


namespace Helpers
{

#pragma region Functions

#ifdef _DEBUG
void logHelper(_In_ bool fDebugLog, _In_z_ PCWSTR pszFmt, ...);
#else
void logHelper(_In_z_ PCWSTR pszFmt, ...);
#endif

INT ExceptionFilter_ExecAll(UINT code, struct _EXCEPTION_POINTERS* exPtrs);

std::unique_ptr<WCHAR[]> BuildString(int cchMax, _In_ PCWSTR pszFmt, ...);

std::wstring GenerateRandomString(_In_ int nRequiredChars, _In_z_ PCWSTR pszSource);
std::unique_ptr<std::vector<int>> GenerateRandomNumbers(_In_ int count);
std::unique_ptr<std::vector<std::wstring>> GenerateRandomStrings(_In_ int count, _In_z_ PCWSTR pszSourceChars);

template <class T, class Fn>
typename std::list<T>::const_iterator FindInList(_In_ const std::list<T>& srcList, _In_ const T& rhs, _In_ const Fn&& fnCompare)
{
    auto cItr = srcList.cbegin();
    for ( ; cItr != srcList.cend(); ++cItr)
    {
        if (fnCompare(*cItr, rhs))
        {
            break;
        }
    }
    return cItr;
}

#pragma endregion Functions

class ITimer
{
public:
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Reset() = 0;
    virtual bool IsRunning() = 0;
    virtual UINT64 GetElapsedSeconds() = 0;
    virtual UINT64 GetElapsedMilliseconds() = 0;
};

class CTimerTicks : public ITimer
{
public:
    CTimerTicks();
    CTimerTicks(const CTimerTicks& rhs);
    CTimerTicks& operator=(const CTimerTicks& rhs);

    void Start() override;
    void Stop() override;
    void Reset() override;
    bool IsRunning() override;
    UINT64 GetElapsedSeconds() override;
    UINT64 GetElapsedMilliseconds() override;

private:
    void _CopyFrom(const CTimerTicks& rhs);

private:
    bool _fIsRunning;
    UINT64 _startTicks;
    UINT64 _elapsedTicks;
};

struct TestStruct
{
    char _c;
    int _i;
    PCWSTR _psz;
    ULONGLONG _ull;
    LARGE_INTEGER _li;

    TestStruct() = default;

    TestStruct(char c, int i, PCWSTR psz, ULONGLONG ull)
    {
        _c = c;
        _i = i;
        _psz = psz;
        _ull = ull;
        _li.LowPart = (DWORD)i;
        _li.HighPart = (LONG)ull;
    }

    bool operator==(const TestStruct& rhs)
    {
        return (_c == rhs._c && _i == rhs._i && _psz == rhs._psz && _ull == rhs._ull);
    }
};

};
