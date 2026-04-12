// InputBox.cpp
#include <afxwin.h>

// 辅助函数: 创建内存对话框模板
static HGLOBAL CreateInputBoxTemplate()
{
    // 计算字符串长度（必须以双字节对齐）
    auto Align = [](size_t size) -> WORD {
        return static_cast<WORD>((size + 3) & ~3);
        };

    // 字符串（空结尾）
    const TCHAR szCaption[] = _T("");      // 标题由 SetWindowText 设置
    const TCHAR szPrompt[] = _T("");       // 静态文本内容动态设置
    const TCHAR szEdit[] = _T("");
    const TCHAR szOK[] = _T("确定");
    const TCHAR szCancel[] = _T("取消");

    // 计算各部分大小
    WORD cbCaption = Align((_tcslen(szCaption) + 1) * sizeof(TCHAR));
    WORD cbPrompt = Align((_tcslen(szPrompt) + 1) * sizeof(TCHAR));
    WORD cbEdit = Align((_tcslen(szEdit) + 1) * sizeof(TCHAR));
    WORD cbOK = Align((_tcslen(szOK) + 1) * sizeof(TCHAR));
    WORD cbCancel = Align((_tcslen(szCancel) + 1) * sizeof(TCHAR));

    // 总大小
    SIZE_T size = sizeof(DLGTEMPLATE) +
        sizeof(WORD) + cbCaption +          // 对话框标题
        4 * (sizeof(DLGITEMTEMPLATE) +      // 4个控件
            sizeof(WORD) +                 // 控件类型（class）
            cbPrompt +                     // Prompt 文本
            sizeof(WORD) + cbEdit +        // Edit 文本
            sizeof(WORD) + cbOK +          // OK 文本
            sizeof(WORD) + cbCancel);      // Cancel 文本

    HGLOBAL hMem = GlobalAlloc(GPTR, size);
    if (!hMem) return nullptr;

    BYTE* p = (BYTE*)hMem;
    DLGTEMPLATE* pDlg = (DLGTEMPLATE*)p;
    p += sizeof(DLGTEMPLATE);

    // 填充对话框模板
    pDlg->style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_SETFONT;
    pDlg->cdit = 4; // 4个控件
    pDlg->x = 0; pDlg->y = 0;
    pDlg->cx = 180; pDlg->cy = 60; // 单位是 dialog units

    // 字体（可选）
    *(WORD*)p = 8; p += sizeof(WORD);           // 点大小
    *(WORD*)p = 0; p += sizeof(WORD);           // 字符集
    wcscpy_s((TCHAR*)p, cbCaption / sizeof(TCHAR), szCaption);
    p += cbCaption;

    // --- 控件 1: Static (提示文本) ---
    DLGITEMTEMPLATE* pItem = (DLGITEMTEMPLATE*)p;
    pItem->style = WS_CHILD | WS_VISIBLE | SS_LEFT;
    pItem->dwExtendedStyle = 0;
    pItem->x = 5; pItem->y = 5; pItem->cx = 170; pItem->cy = 10;
    pItem->id = 1000;
    p += sizeof(DLGITEMTEMPLATE);
    *(WORD*)p = PAGE_OFFSET_MASK; p += sizeof(WORD);      // 类型: WC_STATIC
    *(WORD*)p = 0x0082; p += sizeof(WORD);      // 原子值: STATIC_CLASS
    wcscpy_s((TCHAR*)p, cbPrompt / sizeof(TCHAR), szPrompt);
    p += cbPrompt;

    // --- 控件 2: Edit ---
    pItem = (DLGITEMTEMPLATE*)p;
    pItem->style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
    pItem->dwExtendedStyle = 0;
    pItem->x = 5; pItem->y = 18; pItem->cx = 170; pItem->cy = 12;
    pItem->id = 1001;
    p += sizeof(DLGITEMTEMPLATE);
    *(WORD*)p = PAGE_OFFSET_MASK; p += sizeof(WORD);
    *(WORD*)p = 0x0081; p += sizeof(WORD);      // EDIT
    wcscpy_s((TCHAR*)p, cbEdit / sizeof(TCHAR), szEdit);
    p += cbEdit;

    // --- 控件 3: OK Button ---
    pItem = (DLGITEMTEMPLATE*)p;
    pItem->style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_GROUP;
    pItem->dwExtendedStyle = 0;
    pItem->x = 30; pItem->y = 35; pItem->cx = 40; pItem->cy = 14;
    pItem->id = IDOK;
    p += sizeof(DLGITEMTEMPLATE);
    *(WORD*)p = PAGE_OFFSET_MASK; p += sizeof(WORD);
    *(WORD*)p = 0x0080; p += sizeof(WORD);      // BUTTON
    wcscpy_s((TCHAR*)p, cbOK / sizeof(TCHAR), szOK);
    p += cbOK;

    // --- 控件 4: Cancel Button ---
    pItem = (DLGITEMTEMPLATE*)p;
    pItem->style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
    pItem->dwExtendedStyle = 0;
    pItem->x = 90; pItem->y = 35; pItem->cx = 40; pItem->cy = 14;
    pItem->id = IDCANCEL;
    p += sizeof(DLGITEMTEMPLATE);
    *(WORD*)p = PAGE_OFFSET_MASK; p += sizeof(WORD);
    *(WORD*)p = 0x0080; p += sizeof(WORD);
    wcscpy_s((TCHAR*)p, cbCancel / sizeof(TCHAR), szCancel);
    p += cbCancel;

    return hMem;
}

// 全局变量（线程不安全，但 InputBox 是模态的，所以安全）
static CString g_strInputBoxResult;
static LPCTSTR g_lpszPrompt = nullptr;

// 对话框过程
static INT_PTR CALLBACK InputBoxProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        // 设置标题（lParam 是标题）
        SetWindowText(hwndDlg, (LPCTSTR)lParam);
        // 设置提示文本
        SetDlgItemText(hwndDlg, 1000, g_lpszPrompt);
        // 设置默认值
        SetDlgItemText(hwndDlg, 1001, _T(""));
        // 聚焦编辑框
        HWND hEdit = GetDlgItem(hwndDlg, 1001);
        SendMessage(hEdit, EM_SETSEL, 0, -1);
        SetFocus(hEdit);
        return FALSE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            TCHAR buffer[256] = { 0 };
            GetDlgItemText(hwndDlg, 1001, buffer, 256);
            g_strInputBoxResult = buffer;
            EndDialog(hwndDlg, IDOK);
            return TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

// 主函数
BOOL InputBox(CString& strResult, LPCTSTR lpszPrompt, LPCTSTR lpszTitle, LPCTSTR lpszDefault)
{
    g_lpszPrompt = lpszPrompt;
    g_strInputBoxResult = lpszDefault ? lpszDefault : _T("");

    HGLOBAL hTemplate = CreateInputBoxTemplate();
    if (!hTemplate) return FALSE;

    HWND hParent = AfxGetMainWnd() ? AfxGetMainWnd()->GetSafeHwnd() : NULL;
    INT_PTR nRet = DialogBoxIndirectParam(
        AfxGetInstanceHandle(),
        (LPCDLGTEMPLATE)hTemplate,
        hParent,
        InputBoxProc,
        (LPARAM)lpszTitle
    );

    GlobalFree(hTemplate);

    if (nRet == IDOK) {
        strResult = g_strInputBoxResult;
        return TRUE;
    }
    return FALSE;
}