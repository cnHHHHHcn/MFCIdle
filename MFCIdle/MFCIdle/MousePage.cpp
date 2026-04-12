// MousePage.cpp: 实现文件

#include "pch.h"
#include "MFCIdle.h"
#include "afxdialogex.h"
#include "MousePage.h"

// MousePage 对话框

std::map<CString, WinSysDevice::MouseKey> MouseKeyMap = {
	{L"LeftClick", WinSysDevice::LeftClick},
	{L"LeftDown", WinSysDevice::LeftDown},
	{L"LeftUp", WinSysDevice::LeftUp},
	{L"RightClick", WinSysDevice::RightClick},
	{L"RightDown", WinSysDevice::RightDown},
	{L"RightUp", WinSysDevice::RightUp},
	{L"MiddleClick", WinSysDevice::MiddleClick},
	{L"MiddleDown", WinSysDevice::MiddleDown},
	{L"MiddleUp", WinSysDevice::MiddleUp}
};

IMPLEMENT_DYNAMIC(MousePage, CDialog)

MousePage::MousePage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_KEYBOARDPAGE, pParent)
{
}

MousePage::~MousePage()
{
}

BOOL MousePage::OnInitDialog() {
	CDialogEx::OnInitDialog();
	cmb_MouseKey.SetCurSel(0);
	chk_IsGetPos.SetCheck(1);
	btn_UpdateData.ShowWindow(SW_HIDE);
	CString DesktopInfo;
	WinSysDevice::WinSize Area = Device.GetDesktopSize();
	DesktopInfo.Format(L"分辨率: %d * %d", Area.Width, Area.Height);
	lbl_DesktopSize.SetWindowTextW(DesktopInfo);
	LVCOLUMNW col[6]; ZeroMemory(&col, sizeof(LVCOLUMNW) * 6);
	col[0].pszText = L"序号"; col[1].pszText = L"按键码"; col[2].pszText = L"位置模式";
	col[3].pszText = L"X"; col[4].pszText = L"Y"; col[5].pszText = L"滚轮值";
	for (int i = 0; i < 6; i++) {
		col[i].mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
		col[i].cx = 95;
		col[i].fmt = LVCFMT_CENTER;
		lvw_MouseInfo.InsertColumn(i, &col[i]);
	}
	SetTimer(1, 10, NULL);
	return TRUE;
}

void MousePage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_MOUSEPOS, txt_MousePos);
	DDX_Control(pDX, IDC_CHECK_ABSOLUTE, chk_Absolute);
	DDX_Control(pDX, IDC_COMBO_MOUSEKEY, cmb_MouseKey);
	DDX_Control(pDX, IDC_COMBO_MROW, cmb_Row);
	DDX_Control(pDX, IDC_STATIC_DESKTOPSIZE, lbl_DesktopSize);
	DDX_Control(pDX, IDC_LIST_MOUSEINFO, lvw_MouseInfo);
	DDX_Control(pDX, IDC_EDIT_MOUSEWHEEL, txt_MouseWheel);
	DDX_Control(pDX, IDC_CHECK_GETPOS, chk_IsGetPos);
	DDX_Control(pDX, IDC_BUTTON_MUPDATEDATA, btn_UpdateData);
}

BEGIN_MESSAGE_MAP(MousePage, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_MINSERT, &MousePage::OnBnClickedButtonMInsert)
	ON_BN_CLICKED(IDC_BUTTON_MCLEARLIST, &MousePage::OnBnClickedButtonMClearList)
	ON_BN_CLICKED(IDC_BUTTON_MDELETECURSELLIST, &MousePage::OnBnClickedButtonMDeleteCurSelList)
	ON_BN_CLICKED(IDC_BUTTON_MMODIFYLINE, &MousePage::OnBnClickedButtonMmodifyline)
	ON_CBN_SELCHANGE(IDC_COMBO_MROW, &MousePage::OnCbnSelchangeComboMrow)
	ON_BN_CLICKED(IDC_BUTTON_MOUSEDISPLAY, &MousePage::OnBnClickedButtonMousedisplay)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_MOUSEINFO, &MousePage::OnLvnItemchangedListMouseinfo)
	ON_BN_CLICKED(IDC_BUTTON_MUPDATEDATA, &MousePage::OnBnClickedButtonMupdatedata)
END_MESSAGE_MAP()


// MousePage 消息处理程序
POINT Pos; CString PosInfo, tmp;

void MousePage::InsertMouseInfo(UINT Line) {
	CString MouseInfo[5], tmp; int Pos_tmp;
	cmb_MouseKey.GetWindowTextW(tmp);
	MouseInfo[0] = tmp;
	MouseInfo[1] = chk_Absolute.GetCheck() == 0 ? L"绝对" : L"相对";
	txt_MousePos.GetWindowTextW(tmp);
	txt_MousePos.SetWindowTextW(L"");
	Pos_tmp = _wtoi(tmp.Mid(tmp.Find(L'X:') + 2, tmp.Find(L';') - (tmp.Find(L'X:') + 2)));
	MouseInfo[2] = std::to_wstring(Pos_tmp).c_str();
	Pos_tmp = _wtoi(tmp.Right(tmp.GetLength() - (tmp.ReverseFind(L'Y:') + 2)));
	MouseInfo[3] = std::to_wstring(Pos_tmp).c_str();
	txt_MouseWheel.GetWindowTextW(MouseInfo[4]);
	txt_MouseWheel.SetWindowTextW(L"");
	if (!MouseInfo[4].GetLength()) MouseInfo[4] = L"0";
	for (int i = 0; i < 5; i++) 
		lvw_MouseInfo.SetItemText(Line, i + 1, MouseInfo[i]);
	chk_IsGetPos.SetCheck(1);
}

void MousePage::GetMouseInfoForEdit(UINT Line)
{
	CString tmp_Pos;
	chk_IsGetPos.SetCheck(0);
	cmb_MouseKey.SetCurSel(cmb_MouseKey.FindString(0, lvw_MouseInfo.GetItemText(Line, 1)));
	chk_Absolute.SetCheck(lvw_MouseInfo.GetItemText(Line, 2).Find(L"绝对"));
	tmp_Pos.Format(L"X:%s; Y:%s", lvw_MouseInfo.GetItemText(Line, 3), lvw_MouseInfo.GetItemText(Line, 4));
	txt_MousePos.SetWindowTextW(tmp_Pos);
	txt_MouseWheel.SetWindowTextW(lvw_MouseInfo.GetItemText(Line, 5));
}

void MousePage::OnTimer(UINT_PTR nIDEvent){
	CDialogEx::OnTimer(nIDEvent);
	if (chk_IsGetPos.GetCheck()) {
		if (chk_Absolute.GetCheck()) 
			Pos = Device.GetAbsolutePos();
		else
			Pos = Device.GetScreenPos();
		PosInfo.Format(L"X:%d; Y:%d", Pos.x, Pos.y);
		txt_MousePos.SetWindowTextW(PosInfo);
	}
}


void MousePage::OnBnClickedButtonMInsert()
{
	int LineIndex = lvw_MouseInfo.InsertItem(Index++, std::to_wstring(Index).c_str());
	cmb_Row.AddString(std::to_wstring(LineIndex + 1).c_str());	InsertMouseInfo(LineIndex);
}

void MousePage::OnBnClickedButtonMDeleteCurSelList()
{
	int DeleteCount = 0, SelectCount = lvw_MouseInfo.GetSelectedCount();
	for (int i = lvw_MouseInfo.GetItemCount() - 1; i > -1; i--) {
		if (lvw_MouseInfo.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED) {
			lvw_MouseInfo.DeleteItem(i);;
			if (++DeleteCount == SelectCount) break;
		}
	}
	cmb_Row.ResetContent();
	for (int i = 0; i < lvw_MouseInfo.GetItemCount(); i++) {
		cmb_Row.AddString(std::to_wstring(i + 1).c_str());
		lvw_MouseInfo.SetItemText(i, 0, std::to_wstring(i + 1).c_str());
	}
	Index = lvw_MouseInfo.GetItemCount();
}

void MousePage::OnBnClickedButtonMClearList()
{
	lvw_MouseInfo.DeleteAllItems();
	cmb_Row.ResetContent();
	Index = 0;
}

void MousePage::OnBnClickedButtonMmodifyline()
{
	CString tmp; UINT Row;
	cmb_Row.GetWindowTextW(tmp); Row = (tmp == L"") ? 0 : _wtoi(tmp);
	if (Row != 0) {
		InsertMouseInfo(Row);
	}else {
		MessageBoxW(L"未找到指定数据行!", L"警告", MB_ICONWARNING);
	}
}

void MousePage::OnCbnSelchangeComboMrow()
{
	GetMouseInfoForEdit(cmb_Row.GetCurSel());
}

void MousePage::OnBnClickedButtonMousedisplay()
{
	WinSysDevice::MouseInfo MouseData;
	CString tmp[5]; int CurrentIndex = 0;
	cmb_Row.GetWindowTextW(tmp[0]);
	if (tmp[0].IsEmpty()) {
		MessageBoxW(L"未找到指定数据行!", L"警告", MB_ICONWARNING);
		return;
	}
	CurrentIndex = _wtoi(tmp[0]) - 1;
	for (int i = 0; i < 5; i++)
		tmp[i] = lvw_MouseInfo.GetItemText(CurrentIndex, i + 1);
	MouseData.Key = MouseKeyMap[tmp[0]];
	if (tmp[1].Find(L"绝对") == 0) {
		MouseData.ScreenPos.x = _wtoi(tmp[2]);
		MouseData.ScreenPos.y = _wtoi(tmp[3]);
	}else {
		MouseData.AbsolutePos.x = _wtoi(tmp[2]);
		MouseData.AbsolutePos.y = _wtoi(tmp[3]);
	}
	Device.DisplayMouseInfo(MouseData);
}

void MousePage::OnLvnItemchangedListMouseinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	cmb_Row.SetCurSel(pNMLV->iItem);
	GetMouseInfoForEdit(pNMLV->iItem);
	*pResult = 0;
}

void MousePage::OnBnClickedButtonMupdatedata()
{
	// TODO: 数据更新操作

	UpdateInfo.IsUpdate = false;
	UpdateInfo.DlgID = 0xFF;
	PageDlg[4]->ShowWindow(SW_SHOW);
	ShowWindow(SW_HIDE);
	btn_UpdateData.ShowWindow(SW_HIDE);
}
