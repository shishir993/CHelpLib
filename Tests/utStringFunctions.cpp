
#include "stdafx.h"
#include "StringFunctions.h"

#include "CppUnitTest.h"
#include "Helpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
TEST_CLASS(StringFunctionsTests)
{
public:
    TEST_METHOD(NullEmptyTest);
    TEST_METHOD(NonHierarchicalTest);
    TEST_METHOD(SimpleHierarchicalTest);
    TEST_METHOD(SpecialCharsTest);
};

void StringFunctionsTests::NullEmptyTest()
{
    Assert::IsNull(CHL_SzGetFilenameFromPath(NULL, 128), L"NULL string returns NULL");
    Assert::IsNull(CHL_SzGetFilenameFromPath(L"", 0), L"Empty string returns NULL");
    Assert::IsNull(CHL_SzGetFilenameFromPath(L"", 8), L"Empty string with positive char count returns NULL");
}

void StringFunctionsTests::NonHierarchicalTest()
{
    WCHAR aszTestStrings[][MAX_PATH + 1] =
        {
            L"z", 
            L"zy", 
            L"dbg.dll",
            L"ServerRM.vcproj.SHISHIR-0C6645C.Shishir.user"
        };

    for (int i = 0; i < ARRAYSIZE(aszTestStrings); ++i)
    {
        PCWSTR psz = CHL_SzGetFilenameFromPath(aszTestStrings[i], wcslen(aszTestStrings[0]));
        Assert::IsNotNull(psz, L"Legitimate non-hierarchical string returns non-NULL");
        Assert::IsTrue((wcscmp(psz, aszTestStrings[i]) == 0), L"Non-hierarchical string returns input itself");
    }
}

void StringFunctionsTests::SimpleHierarchicalTest()
{
    WCHAR szTestString[] = L"C:\\Users\\Shishir_Limited\\Downloads\\Matrix_wallpaper_.jpg";

    PCWSTR psz = CHL_SzGetFilenameFromPath(szTestString, wcsnlen_s(szTestString, (ARRAYSIZE(szTestString)) + 1));
    Assert::IsNotNull(psz, L"Legitimate hierarchical string returns non-NULL");
    Assert::IsTrue((wcscmp(psz, L"Matrix_wallpaper_.jpg") == 0), L"Hierarchical string returns leaf only");
}

void StringFunctionsTests::SpecialCharsTest()
{
    WCHAR szSpacesTest[] = L"S:\\MyBriefcase\\Coding\\C\\Remote Monitoring - Internship at RFL\\RM\\Code\\"
        L"Remote Monitoring - To RFL\\Source Code\\ServerRM\\ServerRM\\ScanIPAddresses.cpp";
    
    PCWSTR psz = CHL_SzGetFilenameFromPath(szSpacesTest, wcsnlen_s(szSpacesTest, (ARRAYSIZE(szSpacesTest)) + 1));
    Assert::IsNotNull(psz, L"String with embedded spaces returns non-NULL");
    Assert::IsTrue((wcscmp(psz, L"ScanIPAddresses.cpp") == 0), L"String with embedded spaces returns leaf only");

    WCHAR szMultipleDotsTest[] = L"S:\\MyBriefcase\\Coding\\C\\Remote Monitoring\\ServerRM.vcproj.SHISHIR-0C6645C.Shishir.user";

    psz = CHL_SzGetFilenameFromPath(szMultipleDotsTest, wcsnlen_s(szMultipleDotsTest, (ARRAYSIZE(szMultipleDotsTest)) + 1));
    Assert::IsNotNull(psz, L"String with multiple dots returns non-NULL");
    Assert::IsTrue((wcscmp(psz, L"ServerRM.vcproj.SHISHIR-0C6645C.Shishir.user") == 0), L"String with multiple dots returns leaf only");

}

}