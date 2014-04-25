
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

BOOL fChlGuiInitListViewColumns(HWND hList, WCHAR *apszColumNames[], int nColumns, OPTIONAL int *paiColumnSizePercent)
{
    int index;
    LVCOLUMN lvColumn = {0};

    int *paiColumnSizes = NULL;
    
    RECT rcList;
    LONG lListWidth;

    // Parameter validation
    if(!ISVALID_HANDLE(hList) || !apszColumNames || nColumns <= 0)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        return FALSE;
    }

    // Calculate listview width
    if(!GetWindowRect(hList, &rcList))
    {
        goto error_return;
    }

    lListWidth = rcList.right - rcList.left;

    // Create memory to hold calculated column sizes
    if(!fChlMmAlloc((void**)&paiColumnSizes, sizeof(int) * nColumns, NULL))
    {
        goto error_return;
    }

    // Calculate column sizes
    if(!paiColumnSizePercent)
    {
        for(index = 0; index < nColumns; ++index)
        {
            paiColumnSizes[index] = 0.5 * lListWidth;
        }
    }
    else
    {
        for(index = 0; index < nColumns; ++index)
        {
            paiColumnSizes[index] = paiColumnSizePercent[index] / 100.0 * lListWidth;
        }
    }

    // List view headers
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    for(index = 0; index < nColumns; ++index)
    {
        lvColumn.pszText = apszColumNames[index];
        lvColumn.cx = paiColumnSizes[index];
        SendMessage(hList, LVM_INSERTCOLUMN, index, (LPARAM)&lvColumn);
    }

    vChlMmFree((void**)&paiColumnSizes);
    return TRUE;

error_return:
    IFPTR_FREE(paiColumnSizes);
    return FALSE;
}

BOOL fChlGuiAddListViewRow(HWND hList, WCHAR *apszItemText[], int nItems)
{
    int index;
    int iRetVal;
    int nItemsInListView;

    LVITEM lvItem;

    // Parameter validation
    if(!ISVALID_HANDLE(hList) || !apszItemText || nItems <= 0)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        return FALSE;
    }

    // First, get the current number of list items to use as item index
    nItemsInListView = ListView_GetItemCount(hList);

    // Insert the item, as in, append to the list
    ZeroMemory(&lvItem, sizeof(lvItem));

    lvItem.iItem = nItemsInListView;
	lvItem.mask = LVIF_TEXT;
	lvItem.pszText = apszItemText[0];
	if( (iRetVal = SendMessage(hList, LVM_INSERTITEM, 0, (LPARAM)&lvItem)) == -1 )
	{
		goto error_return;
	}

    ASSERT(iRetVal == nItemsInListView);

    // Now, insert sub items
    for(index = 1; index < nItems; ++index)
    {
        lvItem.iSubItem = index;
		lvItem.pszText = apszItemText[index];
		if( !SendMessage(hList, LVM_SETITEMTEXT, lvItem.iItem, (LPARAM)&lvItem) )
		{
			goto error_return;
		}
    }

    return TRUE;

error_return:
    return FALSE;
}
