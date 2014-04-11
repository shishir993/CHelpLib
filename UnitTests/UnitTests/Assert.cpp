
// Assert.c
// Contains functions that provides the ASSERT() service
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      06/23/13 Initial version
//      04/10/14 getchar() loop changed to hard breakpoint

#include "Assert.h"

void vAssert(const char* psFile, unsigned int uLine)
{
	fflush(NULL);		// ensure that all buffers are written out first
	fprintf(stderr, "Assertion failed in %s at Line %u\n", psFile, uLine);
	fflush(stderr);
    
    __asm int 3

	exit(1);

}// _Assert()
