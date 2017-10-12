
// CHelpLibDllMain.cpp
// Contains the DllMain() function for CHelpLib DLL
// Shishir Bhat (http://www.shishirbhat.com)
// History
//      06/23/13 Initial version
//

#include "CommonInclude.h"

BOOL WINAPI DllMain(HINSTANCE hiDLL, DWORD dwReason,
                    LPVOID pvReserved)
{
    UNREFERENCED_PARAMETER(hiDLL);
    UNREFERENCED_PARAMETER(pvReserved);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            // Init Code here
            break;

        case DLL_THREAD_ATTACH:
            // Thread-specific init code here 
            break;

        case DLL_THREAD_DETACH:
            // Thread-specific cleanup code here.
            break;

        case DLL_PROCESS_DETACH:
            // Cleanup code here 
            break;
    }

    // The return value is used for successful DLL_PROCESS_ATTACH 
    return TRUE;

}// DllMain()
