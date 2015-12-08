
#include "stdafx.h"
#include "Hashtable.h"

#include "CppUnitTest.h"
#include "Helpers.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
// NOTE: This is incomplete
TEST_CLASS(HashTablePerf)
{
public:
    TEST_METHOD(StoryProcessing)
    {
        WCHAR szLine[512];
        int nFieldsRead = 0;
        const int minLen = 8;
        int nItemsAdded = 0;

        UINT64 totalTimeInsertionsMs = 0;
        UINT nInsertions = 0;

        list<wstring> inputStrings;

        FILE* pFile = nullptr;
        Assert::AreEqual(0, _wfopen_s(&pFile, s_pszInputFileTale, L"r"), L"File open must succeed");
        Assert::IsNotNull(pFile);

        Helpers::ITimer* pTimer = new Helpers::CTimerTicks();
        pTimer->Start();
        while ((nFieldsRead = fwscanf_s(pFile, L"%511s", szLine, ARRAYSIZE(szLine))) == 1)
        {
            // Skip strings that don't meet min length
            if (wcslen(szLine) < minLen)
            {
                continue;
            }
            inputStrings.push_back(szLine);
        }

        Assert::AreEqual(nFieldsRead, EOF);
        fclose(pFile);
        pFile = NULL;

        logInfo(L"#strings greater than length %d = %u", minLen, inputStrings.size());
        logInfo(L"Time taken to read input file = %llu ms", pTimer->GetElapsedMilliseconds());

        // Create the hashtable
        CHL_HTABLE* pht;
        Assert::IsTrue(SUCCEEDED(CHL_DsCreateHT(&pht, 100000, CHL_KT_WSTRING, CHL_VT_INT32, FALSE)));

        pTimer->Reset();
        pTimer->Start();
        for (auto& curStr : inputStrings)
        {
            Helpers::CTimerTicks timer;

            int count;
            int valSize = sizeof(int);
            timer.Start();
            if (SUCCEEDED(pht->Find(pht, (PCVOID)curStr.c_str(), 0, &count, &valSize, FALSE)))
            {
                Assert::IsTrue(SUCCEEDED(pht->Insert(pht, (PCVOID)curStr.c_str(), 0, (PCVOID)(count + 1), sizeof(int))));
            }
            else
            {
                ++nItemsAdded;
                Assert::IsTrue(SUCCEEDED(pht->Insert(pht, (PCVOID)curStr.c_str(), 0, (PCVOID)1, sizeof(int))));
            }
            totalTimeInsertionsMs += timer.GetElapsedMilliseconds();
            ++nInsertions;
        }

        logInfo(L"Total insertions = %u, avg insertion time = %f ms", nInsertions, ((double)totalTimeInsertionsMs / nInsertions));
        logInfo(L"Time taken for insertions = %llu ms", pTimer->GetElapsedMilliseconds());

        // Find the string with highest frequency
        PCWSTR pszCur = NULL;
        PCWSTR pszMax = NULL;
        int freqCur = 0;
        int freqMax = 0;

        CHL_HT_ITERATOR htItr;
        Assert::IsTrue(SUCCEEDED(pht->InitIterator(pht, &htItr)));

        int keySize = sizeof(PCWSTR);
        int valSize = sizeof(int);

        while (SUCCEEDED(pht->GetNext(&htItr, &pszCur, &keySize, &freqCur, &valSize, TRUE)))
        {
            if (freqCur > freqMax)
            {
                freqMax = freqCur;
                pszMax = pszCur;
            }
        }

        logInfo(L"Time taken = %llu ms", pTimer->GetElapsedMilliseconds());
        logInfo(L"Max : %s = %d", pszMax, freqMax);

        Assert::IsTrue(SUCCEEDED(pht->Destroy(pht)));
    }

private:
    static PCWSTR s_pszInputFileTale;
};

PCWSTR Tests::HashTablePerf::s_pszInputFileTale = L"C:\\MyData\\tale.txt";

}

