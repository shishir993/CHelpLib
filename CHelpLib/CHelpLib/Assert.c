
// Assert.cpp
// Contains functions that provides the ASSERT() service
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      06/23/13 Initial version
//

#include "Assert.h"

void vAssert(const char* psFile, unsigned int uLine)
{
	fflush(NULL);		// ensure that all buffers are written out first
	fprintf(stderr, "Assertion failed in %s at Line %u\n", psFile, uLine);
	fflush(stderr);
    while(getchar() != '\n');
	exit(1);

}// _Assert()
