
// CommonInclude.h
// Contains common includes, defs and typedefs
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      06/23/13 Initial version
//

#ifndef _COMMONINCLUDE_H
#define _COMMONINCLUDE_H

#ifdef __cplusplus
extern "C" {  
#endif

#define _CRT_RAND_S
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <Psapi.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#include "Assert.h"
#include "DbgHelpers.h"
#include "CHelpLibDll.h"
#include "Helpers.h"

#define HT_MUTEX_NAME   (TEXT("CHL_MUTEX_NAME"))

#ifdef __cplusplus
}
#endif

#endif // _COMMONINCLUDE_H
