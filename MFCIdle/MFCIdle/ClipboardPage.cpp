// ClipboardPage.cpp: 实现文件
//

#include "pch.h"
#include "MFCIdle.h"
#include "afxdialogex.h"
#include "ClipboardPage.h"

#define WARINGCAPTION  L"警告"

// ClipboardPage 对话框

IMPLEMENT_DYNAMIC(ClipboardPage, CDialogEx)

ClipboardPage::ClipboardPage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIPBOARDPAGE, pParent)
{

}

ClipboardPage::~ClipboardPage()
{
}

void ClipboardPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_CLIPTEXT, txt_ClipText);
	DDX_Control(pDX, IDC_COMBO_CLIPMODE, cmb_ClipMode);
	DDX_Control(pDX, IDC_COMBO_CLIPWORDVERSION, cmb_ClipWordVersion);
	DDX_Control(pDX, IDC_BUTTON_CUPDATEDATA, btn_UpdateData);
}


BEGIN_MESSAGE_MAP(ClipboardPage, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_CLIPRUN, &ClipboardPage::OnBnClickedButtonCliprun)
	ON_BN_CLICKED(IDC_BUTTON_CUPDATEDATA, &ClipboardPage::OnBnClickedButtonCupdatedata)
END_MESSAGE_MAP()

// ClipboardPage 消息处理程序

BOOL ClipboardPage::OnInitDialog() {
	CDialogEx::OnInitDialog();
	btn_UpdateData.ShowWindow(SW_HIDE);
	return TRUE;
}

void ClipboardPage::OnBnClickedButtonCliprun()
{
	if (cmb_ClipMode.GetWindowTextLengthW()) {
		if (cmb_ClipWordVersion.GetWindowTextLengthW()) {
			CString WordVersion, Mode, Text, 
					ModeText = L"设置获取清除", VersionText = L"ANSIUnicode";
			cmb_ClipMode.GetWindowTextW(Mode);
			cmb_ClipWordVersion.GetWindowTextW(WordVersion);
			switch (ModeText.Find(Mode)) {
			case 0:
				if (txt_ClipText.GetWindowTextLengthW()) {
					txt_ClipText.GetWindowTextW(Text);
					Device.ClipboardSetTextW((std::wstring)Text);
				}else {
					MessageBoxW(L"剪贴板内容为空！", WARINGCAPTION, MB_ICONWARNING);
				}
				break;
			case 2:
				if (VersionText.Find(WordVersion) == 0) {
					Text = Device.ClipboardGetTextA().c_str();
				}else {
					Text = Device.ClipboardGetTextW().c_str();
				}
				txt_ClipText.SetWindowTextW(Text);
				break;
			case 4:
				Device.ClipboardClear();
				txt_ClipText.SetWindowTextW(Device.ClipboardGetTextW().c_str());
			}
		}
		else {
			MessageBoxW(L"未指定字符集版本！", WARINGCAPTION, MB_ICONWARNING);
		}
	}
	else {
		MessageBoxW(L"未指定剪贴板模式！", WARINGCAPTION, MB_ICONWARNING);
	}

}

void ClipboardPage::OnBnClickedButtonCupdatedata()
{
	// TODO: 数据更新操作

	UpdateInfo.IsUpdate = false;
	UpdateInfo.DlgID = 0xFF;
	PageDlg[4]->ShowWindow(SW_SHOW);
	ShowWindow(SW_HIDE);
	btn_UpdateData.ShowWindow(SW_HIDE);
}
