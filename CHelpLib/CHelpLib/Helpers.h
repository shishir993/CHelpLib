
// Helpers.h
// Contains handy helper defines and functions
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      06/23/13 Initial version
//

#ifndef _HELPERS_H
#define _HELPERS_H

#ifdef __cplusplus
extern "C" {  
#endif

// Pass a pointer and value to this, assignment takes place only if
// the pointer is not NULL
#define IFPTR_SETVAL(ptr, val)  { if(ptr) *ptr = val; }

#define ISVALID_HANDLE(handle)  (handle != NULL && handle != INVALID_HANDLE_VALUE)

#ifdef __cplusplus
}
#endif

#endif // _HELPERS_H
