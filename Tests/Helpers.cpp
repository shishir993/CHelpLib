
#include "stdafx.h"

#include "CppUnitTest.h"
#include "Helpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Helpers
{

WCHAR s_randomStrSource_AlphaNum[] = L"qwertyuiopasdfghjklzxcvbnmABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

#ifdef _DEBUG
void logHelper(_In_ bool fDebugLog, _In_z_ PCWSTR pszFmt, ...)
#else
void logHelper(_In_z_ PCWSTR pszFmt, ...)
#endif
{
    WCHAR s_szBuffer[1024];

    va_list arg;
    va_start(arg, pszFmt);
    HRESULT hr = StringCchVPrintf(s_szBuffer, ARRAYSIZE(s_szBuffer), pszFmt, arg);
    va_end(arg);

    if (SUCCEEDED(hr))
    {
        Logger::WriteMessage(s_szBuffer);

#ifdef _DEBUG
        if (fDebugLog)
        {
            OutputDebugString(s_szBuffer);
        }
#endif
    }
}

INT ExceptionFilter_ExecAll(UINT code, struct _EXCEPTION_POINTERS* exPtrs)
{
    static wchar_t* s_avTypes[] = { L"reading", L"writing", L"executing", L"unknown op" };

    if (code == EXCEPTION_ACCESS_VIOLATION)
    {
        int avTypeIdx;
        switch (exPtrs->ExceptionRecord->ExceptionInformation[0])
        {
        case 0: avTypeIdx = 0; break;
        case 1: avTypeIdx = 1; break;
        case 8: avTypeIdx = 2; break;
        default: avTypeIdx = 3; break;
        }
        
        logWarn(L"Caught exception access violation (0x%08X) at code address 0x%p %s location 0x%p",
            code, exPtrs->ExceptionRecord->ExceptionAddress, s_avTypes[avTypeIdx], exPtrs->ExceptionRecord->ExceptionInformation[1]);
    }
    else
    {
        logWarn(L"Caught exception 0x%08X at code address 0x%p", code, exPtrs->ExceptionRecord->ExceptionAddress);
    }
    
    return EXCEPTION_EXECUTE_HANDLER;
}

std::unique_ptr<WCHAR[]> BuildString(int cchMax, PCWSTR pszFmt, ...)
{
    Assert::IsTrue((cchMax > 0) && (cchMax <= MAXUINT16));   // reasonable limits

    auto spBuffer(std::make_unique<WCHAR[]>(cchMax));

    va_list arg;
    va_start(arg, pszFmt);
    HRESULT hr = StringCchVPrintf(spBuffer.get(), cchMax, pszFmt, arg);
    va_end(arg);

    if (FAILED(hr))
    {
        logDebug(L"%s(): Failed with hr = %x", __FUNCTIONW__, hr);
        spBuffer.reset();
    }
    return spBuffer;
}

std::wstring GenerateRandomString(int nRequiredChars, PCWSTR pszSource)
{
    std::wstringstream wss;
    
    auto cchSource = wcslen(pszSource);

    for (int i = 0; i < nRequiredChars; ++i)
    {
        UINT rand;
        rand_s(&rand);
        wss << pszSource[rand % cchSource];
    }

    return std::move(wss.str());
}

std::unique_ptr<std::vector<int>> GenerateRandomNumbers(_In_ int count)
{
    auto spVector(std::make_unique<std::vector<int>>(count));
    Assert::AreEqual((size_t)count, spVector->size());

    for (int i = 0; i < count; ++i)
    {
        UINT rand;
        rand_s(&rand);
        (*spVector)[i] = static_cast<int>(rand);
    }

    return spVector;
}

std::unique_ptr<std::vector<std::wstring>> GenerateRandomStrings(_In_ int count, _In_z_ PCWSTR pszSourceChars)
{
    auto spVector(std::make_unique<std::vector<std::wstring>>(count));
    Assert::AreEqual((size_t)count, spVector->size());

    const int minLen = 3;
    const int maxLen = 15;

    for (int i = 0; i < count; ++i)
    {
        UINT rand;
        rand_s(&rand);
        int cch = std::max<int>(minLen, (rand % (maxLen + 1)));
        (*spVector)[i] = std::move(GenerateRandomString(cch, pszSourceChars));
    }

    return spVector;
}

int CompareFn_Int32(const PVOID pvLeft, const PVOID pvRight)
{
    int left = (int)pvLeft;
    int right = (int)pvRight;

    if (left < right)
    {
        return -1;
    }

    if (left > right)
    {
        return 1;
    }

    return 0;
}

int CompareFn_WString(const PVOID pvLeft, const PVOID pvRight)
{
    PCWSTR pszLeft = (PCWSTR)pvLeft;
    PCWSTR pszRight = (PCWSTR)pvRight;
    return wcscmp(pszLeft, pszRight);
}

CTimerTicks::CTimerTicks()
{
    Reset();
}

CTimerTicks::CTimerTicks(const CTimerTicks& rhs)
{
    _CopyFrom(rhs);
}

CTimerTicks& CTimerTicks::operator=(const CTimerTicks& rhs)
{
    _CopyFrom(rhs);
    return *this;
}

void CTimerTicks::Start()
{
    if (!_fIsRunning)
    {
        _fIsRunning = true;
        _startTicks = GetTickCount64();
    }
}

void CTimerTicks::Stop()
{
    if (_fIsRunning)
    {
        _fIsRunning = false;
        _elapsedTicks += (GetTickCount64() - _startTicks);
    }
}

void CTimerTicks::Reset()
{
    _fIsRunning = false;
    _startTicks = _elapsedTicks = 0;
}

bool CTimerTicks::IsRunning()
{
    return _fIsRunning;
}

UINT64 CTimerTicks::GetElapsedSeconds()
{
    return (GetElapsedMilliseconds() / 1000);
}

UINT64 CTimerTicks::GetElapsedMilliseconds()
{
    return (_fIsRunning ? (_elapsedTicks + (GetTickCount64() - _startTicks)) : _elapsedTicks);
}

void CTimerTicks::_CopyFrom(const CTimerTicks& rhs)
{
    _fIsRunning = rhs._fIsRunning;
    _startTicks = rhs._startTicks;
    _elapsedTicks = rhs._elapsedTicks;
}

}