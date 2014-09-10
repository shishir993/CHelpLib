
// GuiFunctions.h
// Contains functions that provide IO operation services
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      03/25/14 Initial version
//      09/09/14 Refactor to store defs in individual headers.
//

#ifndef _GUIFUNCTIONS_H
#define _GUIFUNCTIONS_H

#ifdef __cplusplus
extern "C" {  
#endif

#include "CommonInclude.h"
#include "MemFunctions.h"
#include <Commctrl.h>

// -------------------------------------------
// Functions exported

DllExpImp BOOL fChlGuiCenterWindow(HWND hWnd);

// Given the window handle and the number of characters, returns the 
// width and height in pixels that will be occupied by a string of that
// consisting of those number of characters
DllExpImp BOOL fChlGuiGetTextArea(HWND hWindow, int nCharsInText, __out int *pnPixelsWidth, __out int *pnPixelsHeight);

DllExpImp BOOL fChlGuiInitListViewColumns(
    HWND hList, 
    WCHAR *apszColumNames[], 
    int nColumns, 
    OPTIONAL int *paiColumnSizePercent);

DllExpImp BOOL fChlGuiAddListViewRow(HWND hList, WCHAR *apszItemText[], int nItems, __in_opt LPARAM lParam);

#ifdef __cplusplus
}
#endif

#endif // _GUIFUNCTIONS_H
