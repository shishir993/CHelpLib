
#include "stdafx.h"

#include "CppUnitTest.h"
#include "Helpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Helpers
{

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