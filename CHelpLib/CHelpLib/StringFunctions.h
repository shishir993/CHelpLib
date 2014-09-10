// StringFunctions.h
// Contains functions that provide string operations
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      08/10/13 Initial version
//      09/09/14 Refactor to store defs in individual headers.
//

#ifndef _STRINGFUNCTIONS_H
#define _STRINGFUNCTIONS_H

#ifdef __cplusplus
extern "C" {  
#endif

#include "CommonInclude.h"

// -------------------------------------------
// Functions exported

DllExpImp WCHAR* pszChlSzGetFilenameFromPath(WCHAR *pwsFilepath, int numCharsInput);

#ifdef __cplusplus
}
#endif

#endif // _STRINGFUNCTIONS_H
