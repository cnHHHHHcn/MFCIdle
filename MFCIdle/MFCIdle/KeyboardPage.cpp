// KeyBoardPage.cpp: 实现文件
//

#include "pch.h"
#include "MFCIdle.h"
#include "afxdialogex.h"
#include "KeyboardPage.h"


// KeyBoardPage 对话框

std::map<CString, WinSysDevice::ControlHotKey> CtrlOpare = {
	{L"Cut", WinSysDevice::Cut},
	{L"Copy", WinSysDevice::Copy},
	{L"Paste", WinSysDevice::Paste},
	{L"New", WinSysDevice::New},
	{L"Open", WinSysDevice::Open},
	{L"All", WinSysDevice::All},
	{L"Save", WinSysDevice::Save},
	{L"Repeat", WinSysDevice::Repeat},
	{L"Recall", WinSysDevice::Recall},
	{L"Restore", WinSysDevice::Restore}
};

int iItem, iSubItem;

IMPLEMENT_DYNAMIC(KeyboardPage, CDialog)

KeyboardPage::KeyboardPage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_KEYBOARDPAGE, pParent)
{
}

KeyboardPage::~KeyboardPage()
{
}

BOOL KeyboardPage::OnInitDialog() {
	CDialogEx::OnInitDialog();	
	//txt_lvwKeyboardSet.IsWindowVisible();
	btn_UpdateData.ShowWindow(SW_HIDE);
	LVCOLUMNW col[5]; ZeroMemory(&col, sizeof(LVCOLUMNW) * 5);
	col[0].pszText = L"序号"; col[0].cx = 50;
	col[1].pszText = L"Ctrl热键"; col[1].cx = 80;
	col[2].pszText = L"输入法"; col[2].cx = 80;
	col[3].pszText = L"Shift"; col[3].cx = 50;
	col[4].pszText = L"按键字符"; col[4].cx = 305;
	for (int i = 0; i < 5; i++) {
		col[i].mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
		col[i].fmt = LVCFMT_CENTER;
		lvw_KeyboardInfo.InsertColumn(i, &col[i]);
	}
	WinSysDevice::LayoutMap CurrentLayout = Device.InputMethod(WinSysDevice::List_Current);
	cmb_Layout.SetWindowTextW(CurrentLayout[0].LanguageName.c_str());
	WinSysDevice::LayoutMap Layout = Device.InputMethod(WinSysDevice::List_All);
	for (int i = 0; i < Layout.size(); i++) {
		cmb_Layout.AddString(Layout[i].LanguageName.c_str());
	}
	CurrentLayout = Device.InputMethod(WinSysDevice::List_Current);
	cmb_Layout.SetCurSel(cmb_Layout.FindString(0, CurrentLayout[0].LanguageName.c_str()));
	return TRUE;
}

void KeyboardPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_LAYOUT, cmb_Layout);
	DDX_Control(pDX, IDC_COMBO_CTRLHOTKEY, cmb_CtrlHotKey);
	DDX_Control(pDX, IDC_COMBO_KROW, cmb_Row);
	DDX_Control(pDX, IDC_CHECK_SHIFT, chk_Shift);
	DDX_Control(pDX, IDC_EDIT_KEYWORD, txt_KeyWord);
	DDX_Control(pDX, IDC_LIST_KEYBOARDINFO, lvw_KeyboardInfo);

	DDX_Control(pDX, IDC_BUTTON_KUPDATEDATA, btn_UpdateData);
}


BEGIN_MESSAGE_MAP(KeyboardPage, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_LAYOUTNEXT, &KeyboardPage::OnBnClickedButtonLayoutnext)
	ON_CBN_SELENDOK(IDC_COMBO_LAYOUT, &KeyboardPage::On_AppointLayout)
	ON_BN_CLICKED(IDC_BUTTON_KINSERT, &KeyboardPage::OnBnClickedButtonKInsert)
	ON_BN_CLICKED(IDC_BUTTON_KDELETECURSELLIST, &KeyboardPage::OnBnClickedButtonKDeleteCurSelList)
	ON_BN_CLICKED(IDC_BUTTON_KCLEARLIST, &KeyboardPage::OnBnClickedButtonKClearList)
	ON_BN_CLICKED(IDC_BUTTON_KMODIFYLINE, &KeyboardPage::OnBnClickedButtonKmodifyline)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_KEYBOARDINFO, &KeyboardPage::OnLvnItemchangedListKeyboardinfo)
	ON_CBN_SELCHANGE(IDC_COMBO_KROW, &KeyboardPage::OnCbnSelchangeComboKrow)
	ON_BN_CLICKED(IDC_BUTTON_KUPDATEDATA, &KeyboardPage::OnBnClickedButtonKupdatedata)
END_MESSAGE_MAP()


// KeyBoardPage 消息处理程序
void KeyboardPage::InsertKeyboardInfo(UINT Line)
{
	CString KeyboardInfo[4], tmp;
	cmb_CtrlHotKey.GetWindowTextW(tmp);
	KeyboardInfo[0] = tmp;
	cmb_Layout.GetWindowTextW(tmp);
	KeyboardInfo[1] = tmp;
	KeyboardInfo[2] = std::to_wstring(chk_Shift.GetCheck()).c_str();
	txt_KeyWord.GetWindowTextW(tmp);
	KeyboardInfo[3] = tmp;
	cmb_Row.AddString(std::to_wstring(Index).c_str());
	for (int i = 0; i < 4; i++) {
		lvw_KeyboardInfo.SetItemText(Line, i + 1, KeyboardInfo[i]);
	}
}

void KeyboardPage::GetKeyboardInfoForEdit(UINT Line)
{
	cmb_CtrlHotKey.SetCurSel(cmb_CtrlHotKey.FindString(0, lvw_KeyboardInfo.GetItemText(Line, 1)));
	cmb_Layout.SetCurSel(cmb_Layout.FindString(0, lvw_KeyboardInfo.GetItemText(Line, 2)));
	chk_Shift.SetCheck(_wtoi(lvw_KeyboardInfo.GetItemText(Line, 3)));
	txt_KeyWord.SetWindowTextW(lvw_KeyboardInfo.GetItemText(Line, 4));
}

void KeyboardPage::OnBnClickedButtonLayoutnext()
{
	Device.InputMethod(WinSysDevice::Set_Next);
	SetTimer(1, 10, NULL);
	// 注: 不能在后面直接获取，这样实时显示有问题
}


void KeyboardPage::On_AppointLayout()
{
	CString AppointLanguageName; 
	cmb_Layout.GetWindowTextW(AppointLanguageName);
	WinSysDevice::LayoutMap Layout = Device.InputMethod(WinSysDevice::List_All);
	int i = 0;
	for (; i < Layout.size(); i++) {
		if (AppointLanguageName.Find(Layout[i].LanguageName.c_str()) != -1) {
			Device.InputMethod(WinSysDevice::Set_Appoint, Layout[i].LayoutHandle);
			break;
		}
	}
	cmb_Layout.SetCurSel(cmb_Layout.FindString(0, Layout[i].LanguageName.c_str()));
}

void KeyboardPage::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case 1:
		{
			WinSysDevice::LayoutMap CurrentLayout = Device.InputMethod(WinSysDevice::List_Current);
			cmb_Layout.SetCurSel(cmb_Layout.FindString(0, CurrentLayout[0].LanguageName.c_str()));
			KillTimer(1);
		}
	// TODO: 扩展
	}
}

void KeyboardPage::OnBnClickedButtonKInsert()
{
	UINT IndexLine = lvw_KeyboardInfo.InsertItem(Index++, std::to_wstring(Index).c_str());
	InsertKeyboardInfo(IndexLine);
}

void KeyboardPage::OnBnClickedButtonKDeleteCurSelList()
{
	int DeleteCount = 0, SelectCount = lvw_KeyboardInfo.GetSelectedCount();
	for (int i = lvw_KeyboardInfo.GetItemCount() - 1; i > -1; i--) {
		if (lvw_KeyboardInfo.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED) {
			lvw_KeyboardInfo.DeleteItem(i);
			if (++DeleteCount == SelectCount) break;
		}
	}
	cmb_Row.ResetContent();
	for (int i = 0; i < lvw_KeyboardInfo.GetItemCount(); i++) {
		cmb_Row.AddString(std::to_wstring(i + 1).c_str());
		lvw_KeyboardInfo.SetItemText(i, 0, std::to_wstring(i + 1).c_str());
	}
	Index = lvw_KeyboardInfo.GetItemCount();
}

void KeyboardPage::OnBnClickedButtonKClearList()
{
	lvw_KeyboardInfo.DeleteAllItems();
	cmb_Row.ResetContent();
	Index = 0;
}

void KeyboardPage::OnBnClickedButtonKmodifyline()
{
	CString tmp, strText;
	UINT Row = cmb_Row.GetCurSel(); 
	if (Row != 0) {
		InsertKeyboardInfo(Row);
	}
	else {
		MessageBoxW(L"未找到指定数据行!", L"警告", MB_ICONWARNING);
	}
}

void KeyboardPage::OnLvnItemchangedListKeyboardinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	cmb_Row.SetCurSel(pNMLV->iItem);
	GetKeyboardInfoForEdit(pNMLV->iItem);
	*pResult = 0;
}

void KeyboardPage::OnCbnSelchangeComboKrow()
{
	GetKeyboardInfoForEdit(cmb_Row.GetCurSel());
}

void KeyboardPage::OnBnClickedButtonKupdatedata()
{
	// TODO: 数据更新操作

	UpdateInfo.IsUpdate = false;
	UpdateInfo.DlgID = 0xFF;
	PageDlg[4]->ShowWindow(SW_SHOW);
	ShowWindow(SW_HIDE);
	btn_UpdateData.ShowWindow(SW_HIDE);
}
