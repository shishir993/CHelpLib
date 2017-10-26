
// Assert.c
// Contains functions that provides the ASSERT() service
// Shishir Bhat (http://www.shishirbhat.com)
// History
//      06/23/13 Initial version
//

#include "Assert.h"
#ifdef WIN32
#include <Windows.h>
#endif

void vAssert(const char* psFile, unsigned int uLine)
{
    fflush(NULL);		// ensure that all buffers are written out first
    fprintf(stderr, "Assertion failed in %s at Line %u\n", psFile, uLine);
    fflush(stderr);

#ifdef WIN32
    if (IsDebuggerPresent())
    {
        OutputDebugString(L"Assertion failure\n");
        DebugBreak();
    }
#endif

    exit(1);

}// _Assert()
