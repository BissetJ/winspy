//
//  StaticCtrl.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  void MakeHyperlink(HWND hwnd, UINT staticid, COLORREF crLink)
//
//  Creates a very basic hyperlink control from a standard
//  static label.
//
//  Use the standard control notifications (WM_COMMAND) to
//  detect mouse clicks.
//

#include "WinSpy.h"

// Keep track of how many URL controls we have.
static LONG    lRefCount = 0;

//
static HFONT   hfUnderlined;
static HCURSOR hCursor;

typedef struct
{
    WNDPROC  oldproc;
    COLORREF crLink;
    COLORREF crVisited;
} URLCtrl;

void FreeHyperlink(HWND hwnd)
{
    void *mem = (void *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    HeapFree(GetProcessHeap(), 0, mem);

    // Clean up font and cursor resources when the last
    // hyperlink is destroyed
    if (InterlockedDecrement(&lRefCount) == 0)
    {
        DeleteObject(hfUnderlined);
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
}

static LRESULT CALLBACK URLCtrlProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC     hdc;

    URLCtrl *url = (URLCtrl *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    WNDPROC oldproc = url->oldproc;

    RECT    rect;
    SIZE    sz;
    int     nTextLen;
    HANDLE  hOld;

    static  WCHAR szWinText[MAX_PATH];

    switch (iMsg)
    {
    case WM_NCDESTROY:
        FreeHyperlink(hwnd);
        break;

    case WM_PAINT:

        GetClientRect(hwnd, &rect);

        hdc = BeginPaint(hwnd, &ps);

        // Set the font colors
        SetTextColor(hdc, url->crLink);

        hOld = SelectObject(hdc, hfUnderlined);

        // find text to draw
        nTextLen = GetWindowText(hwnd, szWinText, ARRAYSIZE(szWinText));

        // find width / height of text
        GetTextExtentPoint32(hdc, szWinText, nTextLen, &sz);

        // Draw text + fill background at the same time
        ExtTextOut(hdc, 0, (rect.bottom - sz.cy), 0/*ETO_OPAQUE*/, &rect, szWinText, nTextLen, 0);

        SelectObject(hdc, hOld);

        EndPaint(hwnd, &ps);

        return 0;

    case WM_SETTEXT:
        CallWindowProc(oldproc, hwnd, iMsg, wParam, lParam);
        InvalidateRect(hwnd, 0, 0);
        return 0;

    case WM_SETCURSOR:
        SetCursor(hCursor);
        return TRUE;
    }

    return CallWindowProc(oldproc, hwnd, iMsg, wParam, lParam);
}

void MakeHyperlink(HWND hwnd, UINT staticid, COLORREF crLink)
{
    URLCtrl *url;
    HWND hwndCtrl = GetDlgItem(hwnd, staticid);

    // If already a hyperlink
    if ((UINT_PTR)GetWindowLongPtr(hwndCtrl, GWLP_WNDPROC) == (UINT_PTR)URLCtrlProc)
        return;

    url = (URLCtrl *)HeapAlloc(GetProcessHeap(), 0, sizeof(URLCtrl));

    // Create font and cursor resources if this is
    // the first control being created
    if (InterlockedIncrement(&lRefCount) == 1)
    {
        LOGFONT lf;
        HFONT hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        GetObject(hf, sizeof lf, &lf);
        lf.lfUnderline = TRUE;
        hfUnderlined = CreateFontIndirect(&lf);

        hCursor = LoadCursor(NULL, IDC_HAND);
    }

    //turn on notify style
    SetWindowLong(hwndCtrl, GWL_STYLE, GetWindowLong(hwndCtrl, GWL_STYLE) | SS_NOTIFY);
    SetWindowLong(hwndCtrl, GWL_EXSTYLE, GetWindowLong(hwndCtrl, GWL_EXSTYLE) | WS_EX_TRANSPARENT);


    // setup colors
    if (crLink != -1) url->crLink = crLink;
    else url->crLink = RGB(0, 0, 255);
    url->crVisited = RGB(128, 0, 128);

    SendMessage(hwndCtrl, WM_SETFONT, (WPARAM)hfUnderlined, 0);

    // subclass
    url->oldproc = (WNDPROC)SetWindowLongPtr(hwndCtrl, GWLP_WNDPROC, (LONG_PTR)URLCtrlProc);
    SetWindowLongPtr(hwndCtrl, GWLP_USERDATA, (LONG_PTR)url);

    return;
}

void RemoveHyperlink(HWND hwnd, UINT staticid)
{
    HWND hwndCtrl = GetDlgItem(hwnd, staticid);

    URLCtrl *url = (URLCtrl *)GetWindowLongPtr(hwndCtrl, GWLP_USERDATA);

    // if this isn't a hyperlink control...
    if (url == 0 || (UINT_PTR)GetWindowLongPtr(hwndCtrl, GWLP_WNDPROC) != (UINT_PTR)URLCtrlProc)
        return;

    // Restore the window procedure
    SetWindowLongPtr(hwndCtrl, GWLP_WNDPROC, (LONG_PTR)url->oldproc);

    FreeHyperlink(hwndCtrl);
}
