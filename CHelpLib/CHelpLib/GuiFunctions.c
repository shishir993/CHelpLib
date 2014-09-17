
// GuiFunctions.cpp
// Contains functions that provide help in Gui code
// Shishir Bhat (http://www.shishirprasad.net)
// History
//      03/25/14 Initial version
//

#include "GuiFunctions.h"

// DoCenterWindow()
// Given the window handle, centers the window within the
// parent window's client area.
//
HRESULT CHL_GuiCenterWindow(_In_ HWND hWnd)
{
    int x, y;
	int iWidth, iHeight;
	RECT rect, rectP;
	HWND hwndParent = NULL;
	int iScreenWidth, iScreenHeight;

    // Make the window relative to its parent
    if( (hwndParent = GetParent(hWnd)) == NULL)
    {
		return E_INVALIDARG;
    }

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
    return S_OK;

}// fChlGuiCenterWindow()

// Given the window handle and the number of characters, returns the 
// width and height in pixels that will be occupied by a string of that
// consisting of those number of characters
HRESULT CHL_GuiGetTextArea(_In_ HWND hWindow, _In_ int nCharsInText, _Out_ int *pnPixelsWidth, _Out_ int *pnPixelsHeight)
{
    HDC hDC = NULL;
    TEXTMETRIC stTextMetric;

    HRESULT hr = S_OK;

    // Parameter validation
    if(!ISVALID_HANDLE(hWindow) || nCharsInText < 0 || !pnPixelsWidth || !pnPixelsHeight)
    {
        hr = E_INVALIDARG;
        goto done;
    }
    
    hDC = GetDC(hWindow);
    if(hDC == NULL)
    {
        hr = E_FAIL;    // GetDC doesn't set last error
        goto done;
    }

    // TODO: hDC is uninitialized so system (bitmap) font by default? Should I select the font into the DC first?
    if(!GetTextMetrics(hDC, &stTextMetric))
    {
        hr = E_FAIL;    // GetTextMetric doesn't set last error
        goto done;
    }

    *pnPixelsWidth = stTextMetric.tmMaxCharWidth * nCharsInText;
    *pnPixelsHeight = stTextMetric.tmHeight + stTextMetric.tmExternalLeading;

done:
    if(hDC != NULL)
    {
        ReleaseDC(hWindow, hDC);
    }
    return hr;
}

HRESULT CHL_GuiInitListViewColumns(
    _In_ HWND hList, 
    _In_ WCHAR *apszColumNames[], 
    _In_ int nColumns, 
    _In_opt_ int *paiColumnSizePercent)
{
    int index;
    LVCOLUMN lvColumn = {0};

    int *paiColumnSizes = NULL;
    
    RECT rcList;
    LONG lListWidth;

    HRESULT hr = S_OK;

    // Parameter validation
    if(!ISVALID_HANDLE(hList) || !apszColumNames || nColumns <= 0)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    // Create memory to hold calculated column sizes
    hr = CHL_MmAlloc((void**)&paiColumnSizes, sizeof(int) * nColumns, NULL);
    if(FAILED(hr))
    {
        goto done;
    }

    // Calculate listview width
    if(!GetWindowRect(hList, &rcList))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    // Calculate column sizes
    lListWidth = rcList.right - rcList.left;
    if(!paiColumnSizePercent)
    {
        for(index = 0; index < nColumns; ++index)
        {
            paiColumnSizes[index] = (int)(0.5 * lListWidth);
        }
    }
    else
    {
        for(index = 0; index < nColumns; ++index)
        {
            paiColumnSizes[index] = (int)(paiColumnSizePercent[index] / 100.0 * lListWidth);
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

done:
    IFPTR_FREE(paiColumnSizes);
    return hr;
}

HRESULT CHL_GuiAddListViewRow(
    _In_ HWND hList, 
    _In_ WCHAR *apszItemText[], 
    _In_ int nItems, 
    _In_opt_ LPARAM lParam)
{
    int index;
    int iInsertedAt;
    int nItemsInListView;

    LVITEM lvItem;

    HRESULT hr = S_OK;

    // Parameter validation
    if(!ISVALID_HANDLE(hList) || !apszItemText || nItems <= 0)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    // First, get the current number of list items to use as item index
    nItemsInListView = ListView_GetItemCount(hList);

    // Insert the item, as in, append to the list
    ZeroMemory(&lvItem, sizeof(lvItem));

    lvItem.iItem = nItemsInListView;
	lvItem.mask = LVIF_TEXT|LVIF_PARAM;
	lvItem.pszText = apszItemText[0];
    lvItem.lParam = lParam;
	if( (iInsertedAt = SendMessage(hList, LVM_INSERTITEM, 0, (LPARAM)&lvItem)) == -1 )
	{
        hr = E_FAIL;
		goto done;
	}

    // Now, insert sub items
    for(index = 1; index < nItems; ++index)
    {
        lvItem.iSubItem = index;
		lvItem.pszText = apszItemText[index];
		if( !SendMessage(hList, LVM_SETITEMTEXT, iInsertedAt, (LPARAM)&lvItem) )
		{
			hr = E_FAIL;
		}
    }

done:
    return hr;
}
