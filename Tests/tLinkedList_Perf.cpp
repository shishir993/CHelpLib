
#include "stdafx.h"
#include "LinkedList.h"

#include "CppUnitTest.h"
#include "Helpers.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
// NOTE: This is incomplete
TEST_CLASS(LinkedListPerf)
{
public:
    struct KV {
        PWSTR _pwsz;
        int _count;

        KV(PWSTR pwsz, int count)
        {
            _pwsz = pwsz;
            _count = count;
        }
    };

public:
    TEST_METHOD(StoryProcessing)
    {
        WCHAR szLine[512];
        int nFieldsRead = 0;
        const int minLen = 8;

        UINT64 totalTimeInsertionsMs = 0;
        UINT nInsertions = 0;

        list<PWSTR> inputStrings;

        FILE* pFile = nullptr;
        Assert::AreEqual(0, _wfopen_s(&pFile, s_pszInputFileTale, L"r"), L"File open must succeed");
        Assert::IsNotNull(pFile);

        Helpers::ITimer* pTimer = new Helpers::CTimerTicks();
        pTimer->Start();
        while ((nFieldsRead = fwscanf_s(pFile, L"%511s", szLine, (UINT)ARRAYSIZE(szLine))) == 1)
        {
            // Skip strings that don't meet min length
            auto cch = wcslen(szLine);
            if (cch < minLen)
            {
                continue;
            }

            auto pwstr = (PWSTR)malloc((cch + 1) * sizeof(WCHAR));
            wcscpy_s(pwstr, cch + 1, szLine);

            inputStrings.push_back(pwstr);
        }

        Assert::AreEqual(nFieldsRead, EOF);
        fclose(pFile);
        pFile = NULL;

        logInfo(L"#strings greater than length %d = %u", minLen, inputStrings.size());
        logInfo(L"Time taken to read input file = %llu ms", pTimer->GetElapsedMilliseconds());

        // Create the linked list object
        CHL_LLIST* pllist;
        Assert::IsTrue(SUCCEEDED(CHL_DsCreateLL(&pllist, CHL_VT_USEROBJECT, 100000)));

        for (auto& pwsz : inputStrings)
        {
            KV curKV(pwsz, 1);

            KV* pFoundKV;
            Helpers::CTimerTicks timer;

            timer.Start();
            if (SUCCEEDED(pllist->Find(pllist, &curKV, compareKV, &pFoundKV, nullptr, TRUE)))
            {
                ++(pFoundKV->_count);
                continue;
            }

            Assert::IsTrue(SUCCEEDED(pllist->Insert(pllist, &curKV, sizeof(KV))));
            ++nInsertions;
            totalTimeInsertionsMs += timer.GetElapsedMilliseconds();
        }

        logInfo(L"Total insertions = %u, avg insertion time = %f ms", nInsertions, ((double)totalTimeInsertionsMs / nInsertions));
        logInfo(L"Time taken for insertions = %llu ms", pTimer->GetElapsedMilliseconds());

        // Find the string with highest frequency
        PCWSTR pszMax = NULL;
        int freqMax = 0;

        int itr = 0;
        KV* pKV;
        while (SUCCEEDED(pllist->Peek(pllist, itr, &pKV, nullptr, TRUE)))
        {
            if (pKV->_count > freqMax)
            {
                freqMax = pKV->_count;
                pszMax = pKV->_pwsz;
            }
            ++itr;
        }

        logInfo(L"Time taken = %llu ms", pTimer->GetElapsedMilliseconds());
        logInfo(L"Max : %s = %d", pszMax, freqMax);

        Assert::AreEqual(nInsertions, (UINT)itr);

        Assert::IsTrue(SUCCEEDED(pllist->Destroy(pllist)));
    }

    static int compareKV(PCVOID lhs, PCVOID rhs)
    {
        KV* pleft = (KV*)lhs;
        KV* pright = (KV*)rhs;
        return wcscmp(pleft->_pwsz, pright->_pwsz);
    }

private:
    static PCWSTR s_pszInputFileTale;
};

PCWSTR Tests::LinkedListPerf::s_pszInputFileTale = L"E:\\SelfData\\Tale.txt";

}
