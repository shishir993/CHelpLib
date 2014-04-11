
// GuiFunctions.cpp
// Contains functions that provide help in Gui code
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      03/25/14 Initial version
//

#include "GuiFunctions.h"

// DoCenterWindow()
// Given the window handle, centers the window within the
// parent window's client area.
//
BOOL fChlGuiCenterWindow(HWND hWnd)
{
    int x, y;
	int iWidth, iHeight;
	RECT rect, rectP;
	HWND hwndParent = NULL;
	int iScreenWidth, iScreenHeight;

    // Make the window relative to its parent
    if( (hwndParent = GetParent(hWnd)) == NULL)
		return FALSE;

    // todo: handle the case when hWnd is the main window, i.e., 
    // parent is NULL, in which case parent must be considered as the desktop itself

    GetWindowRect(hWnd, &rect);
    GetWindowRect(hwndParent, &rectP);
        
    iWidth  = rect.right  - rect.left;
    iHeight = rect.bottom - rect.top;

    x = ((rectP.right-rectP.left) -  iWidth) / 2 + rectP.left;
    y = ((rectP.bottom-rectP.top) - iHeight) / 2 + rectP.top;

    iScreenWidth  = GetSystemMetrics(SM_CXSCREEN);
    iScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    // Make sure that the dialog box never moves outside of the screen
    if(x < 0) x = 0;
    if(y < 0) y = 0;
    if(x + iWidth  > iScreenWidth)  x = iScreenWidth  - iWidth;
    if(y + iHeight > iScreenHeight) y = iScreenHeight - iHeight;

    MoveWindow(hWnd, x, y, iWidth, iHeight, FALSE);
    return TRUE;

}// fChlGuiCenterWindow()

// Given the window handle and the number of characters, returns the 
// width and height in pixels that will be occupied by a string of that
// consisting of those number of characters
BOOL fChlGuiGetTextArea(HWND hWindow, int nCharsInText, __out int *pnPixelsWidth, __out int *pnPixelsHeight)
{
    HDC hDC;
    TEXTMETRIC stTextMetric;

    // Parameter validation
    if(!ISVALID_HANDLE(hWindow) || nCharsInText < 0 || !pnPixelsWidth || !pnPixelsHeight)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        goto error_return;
    }
    
    if(!(hDC = GetDC(hWindow)))
    {
        goto error_return;
    }

    // TODO: hDC is uninitialized so system (bitmap) font by default? Should I select the font into the DC first?

    if(!GetTextMetrics(hDC, &stTextMetric))
    {
        goto error_return;
    }

    *pnPixelsWidth = stTextMetric.tmMaxCharWidth * nCharsInText;
    *pnPixelsHeight = stTextMetric.tmHeight + stTextMetric.tmExternalLeading;

    return TRUE;

error_return:
    return FALSE;
}