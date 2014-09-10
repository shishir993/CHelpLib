// StringFunctions.cpp
// Contains functions that provide string operations
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      08/10/13 Initial version
//

#include "StringFunctions.h"

WCHAR* pszChlSzGetFilenameFromPath(WCHAR *pwsFilepath, int inputLen)
{
    WCHAR *pws = NULL;

    if(!pwsFilepath) return NULL;
    if(inputLen <= 1) return NULL;

    pws = pwsFilepath + inputLen - 1;

    // Go in reverse until we find a '\' or we reach the 
    // beginning of the string
    while(*(--pws) != '\\' && pws > pwsFilepath);

    if(pws <= pwsFilepath)
        pws = pwsFilepath;
    else
        pws += 1;

    return pws;
}
