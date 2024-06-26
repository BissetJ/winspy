//
//  WinSpyDlgs.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  Contains all the dialog box procedures for
//  each tab-pane dialog control.
//

#include "WinSpy.h"

#include "resource.h"
#include "BitmapButton.h"
#include "CaptureWindow.h"
#include "Utils.h"

void  MakeHyperlink(HWND hwnd, UINT staticid, COLORREF crLink);
void  RemoveHyperlink(HWND hwnd, UINT staticid);


//
// User clicked the WndProc N/A link on either the General or Class tab.
//

VOID OnWndProcLinkClicked(HWND hwndDlg)
{
    // Hide the link, show the non-link peer.

    ShowDlgItem(hwndDlg, IDC_WNDPROC_LINK, SW_HIDE);
    ShowDlgItem(hwndDlg, IDC_WNDPROC, SW_SHOW);

    // Attempt to fetch the extra information via thread injection.

    GetRemoteInfo();

    // Refresh the current tab.

    UpdateActiveTab();
}

//
//
//
UINT WinSpy_PopupCommandHandler(HWND hwndDlg, UINT uCmdId, HWND hwndTarget)
{
    DWORD dwStyle, dwStyleEx;
    DWORD dwSWPflags;
    HWND  hwndZ;

    dwStyle = GetWindowLong(hwndTarget, GWL_STYLE);
    dwStyleEx = GetWindowLong(hwndTarget, GWL_EXSTYLE);

    switch (uCmdId)
    {
        // Show / Hide
    case IDM_POPUP_VISIBLE:

        dwSWPflags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER;

        if (dwStyle & WS_VISIBLE)
            dwSWPflags |= SWP_HIDEWINDOW;
        else
            dwSWPflags |= SWP_SHOWWINDOW;

        SetWindowPos(hwndTarget, 0, 0, 0, 0, 0, dwSWPflags);

        return 0;

        // Enable / Disable
    case IDM_POPUP_ENABLED:
        EnableWindow(hwndTarget, (dwStyle & WS_DISABLED) ? TRUE : FALSE);
        return 0;

        // Ontop / Not ontop
    case IDM_POPUP_ONTOP:

        dwSWPflags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE;

        if (dwStyleEx & WS_EX_TOPMOST)
            hwndZ = HWND_NOTOPMOST;
        else
            hwndZ = HWND_TOPMOST;

        SetWindowPos(hwndTarget, hwndZ, 0, 0, 0, 0, dwSWPflags);

        return 0;

    case IDM_POPUP_POSTER:
        ShowPosterDlg(hwndDlg, hwndTarget);
        return 0;

        // Show the edit-size dialog
    case IDM_POPUP_SETPOS:

        ShowEditSizeDlg(hwndDlg, hwndTarget);
        return 0;

    case IDM_POPUP_TOFRONT:
        SetWindowPos(hwndTarget, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        return 0;

    case IDM_POPUP_TOBACK:
        SetWindowPos(hwndTarget, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        return 0;

        // Close window
    case IDM_POPUP_CLOSE:
        PostMessage(hwndTarget, WM_CLOSE, 0, 0);
        return 0;

        // new for 1.6
    case IDM_POPUP_CAPTURE:
        CaptureWindow(hwndDlg, hwndTarget);
        return 0;

    default:
        return 0;

    }
}

//
//  Configure the popup menu
//
void WinSpy_SetupPopupMenu(HMENU hMenu, HWND hwndTarget)
{
    HWND hParentWnd;
    BOOL fParentVisible;
    BOOL fParentEnabled;

    DWORD dwStyle;
    DWORD dwStyleEx;

    dwStyle = GetWindowLong(hwndTarget, GWL_STYLE);
    dwStyleEx = GetWindowLong(hwndTarget, GWL_EXSTYLE);

    hParentWnd = GetRealParent(hwndTarget);
    if (hParentWnd)
    {
        fParentVisible = IsWindowVisible(hParentWnd);
        fParentEnabled = IsWindowEnabled(hParentWnd);
    }
    else
    {
        fParentVisible = TRUE;
        fParentEnabled = TRUE;
    }

    if (dwStyle & WS_VISIBLE)
        CheckMenuItem(hMenu, IDM_POPUP_VISIBLE, MF_BYCOMMAND | MF_CHECKED);
    else
        CheckMenuItem(hMenu, IDM_POPUP_VISIBLE, MF_BYCOMMAND | MF_UNCHECKED);

    if (dwStyle & WS_DISABLED)
        CheckMenuItem(hMenu, IDM_POPUP_ENABLED, MF_BYCOMMAND | MF_UNCHECKED);
    else
        CheckMenuItem(hMenu, IDM_POPUP_ENABLED, MF_BYCOMMAND | MF_CHECKED);

    if (dwStyleEx & WS_EX_TOPMOST)
        CheckMenuItem(hMenu, IDM_POPUP_ONTOP, MF_BYCOMMAND | MF_CHECKED);
    else
        CheckMenuItem(hMenu, IDM_POPUP_ONTOP, MF_BYCOMMAND | MF_UNCHECKED);

    EnableMenuItem(hMenu, IDM_POPUP_VISIBLE, MF_BYCOMMAND | (fParentVisible ? MF_ENABLED : MF_DISABLED | MF_GRAYED));
    EnableMenuItem(hMenu, IDM_POPUP_ONTOP, MF_BYCOMMAND | (fParentVisible ? MF_ENABLED : MF_DISABLED | MF_GRAYED));
    EnableMenuItem(hMenu, IDM_POPUP_ENABLED, MF_BYCOMMAND | (fParentEnabled ? MF_ENABLED : MF_DISABLED | MF_GRAYED));
}

//
//  General tab
//
BOOL GeneralDlg_OnDoubleClick(HWND hwnd, LPARAM lParam)
{
    HWND hwndHit;
    POINT pt;

    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    hwndHit = ChildWindowFromPointEx(hwnd, pt, CWP_ALL);

    if (hwndHit == GetDlgItem(hwnd, IDC_CLIENTRECT_LABEL))
    {
        g_fShowClientRectAsMargins = !g_fShowClientRectAsMargins;
        DisplayWindowInfo(g_hCurWnd);
        return TRUE;
    }

    return FALSE;
}

INT_PTR CALLBACK GeneralDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND     hCtrl;
    WCHAR    ach[256];
    HWND     hwndEdit1, hwndEdit2;
    HMENU    hMenu, hPopup;
    RECT     rect;
    UINT     uCmd;
    int      index;
    LONG_PTR lp;
    POINT    pt;
    RECT     rc;

    switch (iMsg)
    {
    case WM_INITDIALOG:

        // Convert standard buttons into bitmapped-buttons
        MakeDlgBitmapButton(hwnd, IDC_HANDLE_MENU, IDI_DOWN_ARROW);
        MakeDlgBitmapButton(hwnd, IDC_EDITSIZE, IDI_DOTS);
        MakeDlgBitmapButton(hwnd, IDC_SETCAPTION, IDI_ENTER);

        MakeHyperlink(hwnd, IDC_WNDPROC_LINK, RGB(0, 0, 255));

        return TRUE;

    case WM_CONTEXTMENU:
        if ((HWND)wParam == GetDlgItem(hwnd, IDC_BYTESLIST))
        {
            index = (int)SendDlgItemMessage(hwnd, IDC_BYTESLIST, CB_GETCURSEL, 0, 0);
            if (index == CB_ERR)
                break;

            lp = SendDlgItemMessage(hwnd, IDC_BYTESLIST, CB_GETITEMDATA, index, 0);

            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);

            // Calculate x, y if using keyboard
            if (pt.x == -1 && pt.y == -1)
            {
                GetClientRect(GetDlgItem(hwnd, IDC_BYTESLIST), &rc);
                pt.x = rc.left + (rc.right - rc.left) / 2;
                pt.y = rc.top + (rc.bottom - rc.top) / 2;

                ClientToScreen(GetDlgItem(hwnd, IDC_BYTESLIST), &pt);
            }

            hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU_BYTES));

            // Show the menu
            uCmd = TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, 0);

            // Act accordingly
            switch (uCmd)
            {
            case IDM_BYTES_COPY:
                swprintf_s(ach, ARRAYSIZE(ach), L"%p", (void*)lp);
                CopyTextToClipboard(hwnd, ach);
                break;
            }

            DestroyMenu(hMenu);
            return TRUE;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_WNDPROC_LINK:
            OnWndProcLinkClicked(hwnd);
            return TRUE;

        case IDC_EDITSIZE:

            // Display the edit-size dialog
            ShowEditSizeDlg(hwnd, g_hCurWnd);
            return TRUE;

        case IDC_HANDLE_MENU:

            // Show our popup menu under this button
            hCtrl = g_hCurWnd;

            hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU_WINDOW_ONTAB));
            hPopup = GetSubMenu(hMenu, 0);

            GetWindowRect((HWND)lParam, &rect);

            WinSpy_SetupPopupMenu(hPopup, hCtrl);

            uCmd = TrackPopupMenu(hPopup, TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_RETURNCMD,
                rect.right, rect.bottom, 0, hwnd, 0);

            if (uCmd)
            {
                WinSpy_PopupCommandHandler(hwnd, uCmd, hCtrl);
                UpdateActiveTab();
            }

            DestroyMenu(hMenu);

            return TRUE;

        case IDC_SETCAPTION:

            // Set the target's window caption to the contents of the edit box
            hwndEdit1 = GetDlgItem(hwnd, IDC_CAPTION1);
            hwndEdit2 = GetDlgItem(hwnd, IDC_CAPTION2);

            // Show the combo box and hide the edit box
            if (IsWindowVisible(hwndEdit1))
            {
                // Copy the contents of the edit box to the combo box
                GetWindowText(hwndEdit1, ach, ARRAYSIZE(ach));
                SetWindowText(hwndEdit2, ach);

                ShowWindow(hwndEdit2, SW_SHOW);
                ShowWindow(hwndEdit1, SW_HIDE);
                SetFocus(hwndEdit2);
            }

            hCtrl = (HWND)g_hCurWnd;

            // get the original text and add it to the combo list
            GetWindowText(hCtrl, ach, ARRAYSIZE(ach));

            SendMessage(hwndEdit2, CB_ADDSTRING, 0, (LPARAM)ach);

            // now see what the new caption is to be
            GetWindowText(hwndEdit2, ach, ARRAYSIZE(ach));

            // set the text to the new string
            if (hCtrl != 0 && IsWindow(hCtrl))
                SendMessage(hCtrl, WM_SETTEXT, 0, (LPARAM)ach);

            return TRUE;
        }

        return FALSE;

    case WM_DRAWITEM:

        if (wParam == IDC_EDITSIZE || wParam == IDC_HANDLE_MENU || wParam == IDC_SETCAPTION)
        {
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, DrawBitmapButton((DRAWITEMSTRUCT *)lParam));
            return TRUE;
        }
        else
            break;


    case WM_LBUTTONDBLCLK:
        if (GeneralDlg_OnDoubleClick(hwnd, lParam))
        {
            return TRUE;
        }
    }

    return FALSE;
}

//
//  Style tab
//
INT_PTR CALLBACK StyleDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch (iMsg)
    {
    case WM_INITDIALOG:

        MakeDlgBitmapButton(hwnd, IDC_EDITSTYLE, IDI_DOTS);
        MakeDlgBitmapButton(hwnd, IDC_EDITSTYLEEX, IDI_DOTS);
        MakeDlgBitmapButton(hwnd, IDC_EDITSTYLEEXT, IDI_DOTS);

        return TRUE;

    case WM_MEASUREITEM:
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FunkyList_MeasureItem(hwnd, (MEASUREITEMSTRUCT *)lParam));
        return TRUE;

    case WM_DRAWITEM:

        if (wParam == IDC_LIST1 || wParam == IDC_LIST2)
        {
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FunkyList_DrawItem(hwnd, (UINT)wParam, (DRAWITEMSTRUCT *)lParam));
            return TRUE;
        }
        else if (wParam == IDC_EDITSTYLE || wParam == IDC_EDITSTYLEEX || wParam == IDC_EDITSTYLEEXT)
        {
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, DrawBitmapButton((DRAWITEMSTRUCT *)lParam));
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDC_EDITSTYLE:
            ShowWindowStyleEditor(hwnd, g_hCurWnd, STYLE_FLAVOR_REGULAR);
            return TRUE;

        case IDC_EDITSTYLEEX:
            ShowWindowStyleEditor(hwnd, g_hCurWnd, STYLE_FLAVOR_EX);
            return TRUE;

        case IDC_EDITSTYLEEXT:
            ShowWindowStyleEditor(hwnd, g_hCurWnd, STYLE_FLAVOR_EXTRA);
            return TRUE;

        default:
            break;
        }

        return FALSE;
    }
    return FALSE;
}

//
//  Window tab
//
INT_PTR CALLBACK WindowDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND      hwndList1, hwndList2;
    LVCOLUMN  lvcol;
    RECT      rect;
    int       width;
    //int       xs[] = { 64, 100, 140 };
    WCHAR     ach[10];
    NMITEMACTIVATE *nmatv;

    switch (iMsg)
    {
    case WM_INITDIALOG:

        hwndList1 = GetDlgItem(hwnd, IDC_LIST1);
        hwndList2 = GetDlgItem(hwnd, IDC_LIST2);

        // Full row select for both ListViews
        ListView_SetExtendedListViewStyle(hwndList1, LVS_EX_FULLROWSELECT);
        ListView_SetExtendedListViewStyle(hwndList2, LVS_EX_FULLROWSELECT);

        // See how much space we have for the header columns to
        // fit exactly into the dialog
        GetClientRect(hwndList1, &rect);
        width = rect.right;
        width -= GetSystemMetrics(SM_CXVSCROLL);

        lvcol.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvcol.cx = DPIScale(hwnd, 64);
        lvcol.iSubItem = 0;
        lvcol.pszText = L"Handle";
        ListView_InsertColumn(hwndList1, 0, &lvcol);
        ListView_InsertColumn(hwndList2, 0, &lvcol);
        width -= lvcol.cx;

        lvcol.pszText = L"Class Name";
        lvcol.cx = DPIScale(hwnd, 100);
        ListView_InsertColumn(hwndList1, 1, &lvcol);
        ListView_InsertColumn(hwndList2, 1, &lvcol);
        width -= lvcol.cx;

        lvcol.pszText = L"Window Text";
        lvcol.cx = max(width, DPIScale(hwnd, 64));
        ListView_InsertColumn(hwndList1, 2, &lvcol);
        ListView_InsertColumn(hwndList2, 2, &lvcol);

        // Make hyperlinks from our two static controls
        MakeHyperlink(hwnd, IDC_PARENT, RGB(0, 0, 255));
        MakeHyperlink(hwnd, IDC_OWNER, RGB(0, 0, 255));

        return TRUE;

    case WM_NOTIFY:
        nmatv = (NMITEMACTIVATE *)lParam;

        if (nmatv->hdr.code == NM_DBLCLK)
        {
            ListView_GetItemText(nmatv->hdr.hwndFrom, nmatv->iItem, 0, ach, ARRAYSIZE(ach));
            DisplayWindowInfo((HWND)_tstrtoib16(ach));
        }

        return FALSE;

    case WM_SYSCOLORCHANGE:
        ListView_SetBkColor(GetDlgItem(hwnd, IDC_LIST1), GetSysColor(COLOR_WINDOW));
        ListView_SetBkColor(GetDlgItem(hwnd, IDC_LIST2), GetSysColor(COLOR_WINDOW));
        return FALSE;

        // if clicked on one of the underlined static controls, then
        // display window info.
    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDC_PARENT:
        case IDC_OWNER:
            if (HIWORD(wParam) == STN_CLICKED)
            {
                GetDlgItemText(hwnd, LOWORD(wParam), ach, ARRAYSIZE(ach));
                DisplayWindowInfo((HWND)_tstrtoib16(ach));
                return TRUE;
            }
        }

        return FALSE;
    }
    return FALSE;
}

//
//  Properties tab
//
INT_PTR CALLBACK PropertyDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND      hwndList1;
    LVCOLUMN  lvcol;
    RECT      rect;
    int       width;

    int       selected;
    POINT     pt;
    RECT      rc;
    HMENU     hMenu;
    UINT      uCmd;
    LVITEM    lvitem;
    WCHAR     ach[256];

    switch (iMsg)
    {
    case WM_INITDIALOG:

        // Full-row selection for the ListView
        hwndList1 = GetDlgItem(hwnd, IDC_LIST1);
        ListView_SetExtendedListViewStyle(hwndList1, LVS_EX_FULLROWSELECT);

        // Work out how big the header-items need to be
        GetClientRect(hwndList1, &rect);
        width = rect.right;
        width -= GetSystemMetrics(SM_CXVSCROLL);

        int cxHandle = DPIScale(hwnd, 20 + 12 * sizeof(LONG_PTR));

        // Insert "Property" header-item
        lvcol.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvcol.iSubItem = 0;
        lvcol.cx = width - cxHandle;
        lvcol.pszText = L"Property Name";
        ListView_InsertColumn(hwndList1, 0, &lvcol);

        // Insert "Handle" header-item
        lvcol.cx = cxHandle;
        lvcol.pszText = L"Handle";
        ListView_InsertColumn(hwndList1, 1, &lvcol);

        return TRUE;

    case WM_CONTEXTMENU:
        hwndList1 = GetDlgItem(hwnd, IDC_LIST1);

        if ((HWND)wParam == hwndList1)
        {
            // ListView has been right-clicked, so show the popup menu

            if (ListView_GetSelectedCount(hwndList1) == 1)
                selected = ListView_GetNextItem(hwndList1, -1, LVNI_SELECTED);
            else
                selected = -1;

            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);

            // Calculate x, y if using keyboard
            if (pt.x == -1 && pt.y == -1)
            {
                if (selected == -1)
                {
                    GetClientRect(hwndList1, &rc);
                    pt.x = rc.left + (rc.right - rc.left) / 2;
                    pt.y = rc.top + (rc.bottom - rc.top) / 2;
                }
                else
                {
                    ListView_GetItemRect(hwndList1, selected, &rc, LVIR_ICON);
                    pt.x = rc.right;
                    pt.y = rc.bottom;
                }

                ClientToScreen(hwndList1, &pt);
            }

            hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU_PROPERTY));

            // Show the menu
            uCmd = TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, 0);

            // Act accordingly
            switch (uCmd)
            {
            case IDM_PROPERTY_ADD:
                ShowWindowPropertyEditor(hwnd, g_hCurWnd, TRUE);
                break;

            case IDM_PROPERTY_EDIT:
                if (selected != -1)
                    ShowWindowPropertyEditor(hwnd, g_hCurWnd, FALSE);
                break;

            case IDM_PROPERTY_REMOVE:
                if (selected != -1)
                {
                    lvitem.mask = LVIF_TEXT | LVIF_PARAM;
                    lvitem.iItem = selected;
                    lvitem.iSubItem = 0;
                    lvitem.pszText = ach;
                    lvitem.cchTextMax = 256;

                    ListView_GetItem(hwndList1, &lvitem);

                    if (RemoveProp(g_hCurWnd, lvitem.lParam ? MAKEINTATOM(lvitem.lParam) : lvitem.pszText))
                        ListView_DeleteItem(hwndList1, selected);
                }
                break;
            }

            DestroyMenu(hMenu);
            return TRUE;
        }

        break;

    case WM_NOTIFY:
        if (((NMHDR *)lParam)->idFrom == IDC_LIST1)
        {
            switch (((NMHDR *)lParam)->code)
            {
            case NM_DBLCLK:
                ShowWindowPropertyEditor(hwnd, g_hCurWnd, FALSE);
                break;
            }
        }

        return TRUE;

    case WM_SYSCOLORCHANGE:

        // Need to react to system color changes
        ListView_SetBkColor(GetDlgItem(hwnd, IDC_LIST1), GetSysColor(COLOR_WINDOW));
        return FALSE;
    }

    return FALSE;
}

//
// Class tab.
//
INT_PTR CALLBACK ClassDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    WCHAR    ach[256];
    int      index;
    HMENU    hMenu;
    UINT     uCmd;
    POINT    pt;
    RECT     rc;
    LONG_PTR lp;

    switch (iMsg)
    {
    case WM_INITDIALOG:
        MakeHyperlink(hwnd, IDC_WNDPROC_LINK, RGB(0, 0, 255));
        return TRUE;

        // Just make the class-name edit-box look normal, even
        // though it is read-only
    case WM_CTLCOLORSTATIC:

        if ((HWND)lParam == GetDlgItem(hwnd, IDC_CLASSNAME))
        {
            SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
            SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
            return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
        }
        else
            return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_WNDPROC_LINK:
            OnWndProcLinkClicked(hwnd);
            return TRUE;
        }

        return FALSE;

    case WM_CONTEXTMENU:
        if ((HWND)wParam == GetDlgItem(hwnd, IDC_BYTESLIST))
        {
            index = (int)SendDlgItemMessage(hwnd, IDC_BYTESLIST, CB_GETCURSEL, 0, 0);
            if (index == CB_ERR)
                break;

            lp = SendDlgItemMessage(hwnd, IDC_BYTESLIST, CB_GETITEMDATA, index, 0);

            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);

            // Calculate x, y if using keyboard
            if (pt.x == -1 && pt.y == -1)
            {
                GetClientRect(GetDlgItem(hwnd, IDC_BYTESLIST), &rc);
                pt.x = rc.left + (rc.right - rc.left) / 2;
                pt.y = rc.top + (rc.bottom - rc.top) / 2;

                ClientToScreen(GetDlgItem(hwnd, IDC_BYTESLIST), &pt);
            }

            hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU_BYTES));

            // Show the menu
            uCmd = TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, 0);

            // Act accordingly
            switch (uCmd)
            {
            case IDM_BYTES_COPY:
                swprintf_s(ach, ARRAYSIZE(ach), L"%p", (void*)lp);
                CopyTextToClipboard(hwnd, ach);
                break;
            }

            DestroyMenu(hMenu);
            return TRUE;
        }
        break;
    }

    return FALSE;
}

//
//  Process Tab
//
INT_PTR CALLBACK ProcessDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    RECT   rect;

    switch (iMsg)
    {
    case WM_INITDIALOG:
        MakeDlgBitmapButton(hwnd, IDC_PROCESS_MENU, IDI_DOWN_ARROW);
        return TRUE;

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDC_PROCESS_MENU:

            GetWindowRect((HWND)lParam, &rect);

            ShowProcessContextMenu(
                (HWND)lParam,
                rect.right,
                rect.bottom,
                TRUE,
                g_hCurWnd,
                g_dwSelectedPID);

            return TRUE;

        }

        return FALSE;

    case WM_CTLCOLORSTATIC:

        if ((HWND)lParam == GetDlgItem(hwnd, IDC_PROCESSNAME) ||
            (HWND)lParam == GetDlgItem(hwnd, IDC_PROCESSPATH))
        {
            SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
            SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
            return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
        }

        return 0;

    case WM_DRAWITEM:
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, DrawBitmapButton((DRAWITEMSTRUCT *)lParam));
        return TRUE;
    }

    return FALSE;
}

//
// DPI Tab
//
INT_PTR CALLBACK DpiDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    switch(iMsg)
    {
    case WM_INITDIALOG:
        return TRUE;
    }

    return FALSE;
}
