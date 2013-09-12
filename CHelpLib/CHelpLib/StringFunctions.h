// StringFunctions.h
// Contains functions that provide string operations
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      08/10/13 Initial version
//

#ifndef _STRINGFUNCTIONS_H
#define _STRINGFUNCTIONS_H

#ifdef __cplusplus
extern "C" {  
#endif

#include "CommonInclude.h"

// Exported functions
WCHAR* Chl_GetFilenameFromPath(WCHAR *pwsFilepath, int numCharsInput);

#ifdef __cplusplus
}
#endif

#endif // _STRINGFUNCTIONS_H
