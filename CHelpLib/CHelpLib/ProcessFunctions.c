
// ProcessFunctions.cpp
// Contains functions that provide help Process related tasks
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      03/25/14 Initial version
//

#include "ProcessFunctions.h"

// GetProcNameFromID()
// Given a process ID, returns the name of the executable
// 
BOOL fChlPsGetProcNameFromID(DWORD pid, WCHAR *pwsProcName, DWORD dwBufSize)
{
    HANDLE hProcess = NULL;
    HMODULE ahModules[512];
    DWORD dwNeeded = 0;

    ASSERT(dwBufSize > 5);    // 3(extension) + 1(.) + 1(imagename atleast one char)

    if(pid == 0)
    {
        return FALSE;
    }

    if((hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, pid)) == NULL)
    {
        return FALSE;
    }

    if(!EnumProcessModules(hProcess, ahModules, sizeof(ahModules), &dwNeeded))
    {
        goto error_return;
    }

    //logdbg("#Modules = %d", dwNeeded/sizeof(HMODULE));
    if(!GetModuleBaseName(hProcess, ahModules[0], pwsProcName, dwBufSize))
    {
        goto error_return;
    }

    CloseHandle(hProcess);
    return TRUE;

error_return:
    CloseHandle(hProcess);
    return FALSE;

}// GetProcNameFromID()
