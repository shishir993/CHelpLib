This library is to make the life of C/Win32 programmer easier by providing function wrappers such as read line from stdout, simple memory alloc functions...

Memory mgmt
- BOOL ChlfMemAlloc(void **, size_t)
- void ChlvMemFree(void**)

Input/Output
- BOOL ChlIOReadLineFromStdout(char *, DWORD, DWORD*)