// StringFunctions.cpp
// Contains functions that provide string operations
// Shishir Bhat (http://www.shishirbhat.com)
// History
//      08/10/13 Initial version
//      09/09/14 Refactor to store defs in individual headers.
//

#include "StringFunctions.h"

PCWSTR CHL_SzGetFilenameFromPath(_In_z_ PCWSTR pszFilepath, _In_ int inputLen)
{
    PCWSTR psz = NULL;

    if (!pszFilepath || (*pszFilepath == L'\0') || (inputLen < 1))
    {
        return NULL;
    }

    psz = pszFilepath + inputLen - 1;

    // Go in reverse until we find a '\' or we reach the 
    // beginning of the string
    while(*(--psz) != '\\' && psz > pszFilepath);

    if(psz <= pszFilepath)
        psz = pszFilepath;
    else
        psz += 1;

    return psz;
}
