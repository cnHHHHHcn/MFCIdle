// CapturePage.cpp: 实现文件
//

#include "pch.h"
#include "MFCIdle.h"
#include "afxdialogex.h"
#include "CapturePage.h"
#include "MousePage.h"


// CapturePage 对话框

IMPLEMENT_DYNAMIC(CapturePage, CDialogEx)

CapturePage::CapturePage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CAPTUREPAGE, pParent)
{

}

CapturePage::~CapturePage()
{
}

BOOL CapturePage::OnInitDialog() {
	CDialogEx::OnInitDialog();
	RECT pic_Area = { 200, 10, 610, 260 };
	pic_Display.MoveWindow(&pic_Area);
	btn_UpdateData.ShowWindow(SW_HIDE);
	return TRUE;
}

void CapturePage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_WNDTITLE, txt_WNDTitle);
	DDX_Control(pDX, IDC_EDIT_HWND, txt_HWND);
	DDX_Control(pDX, IDC_EDIT_WNDLEFT, txt_WNDLeft);
	DDX_Control(pDX, IDC_EDIT_WNDTOP, txt_WNDTop);
	DDX_Control(pDX, IDC_EDIT_WNDWIDTH, txt_WNDWidth);
	DDX_Control(pDX, IDC_EDIT_WNDHEIGHT, txt_WNDHeight);
	DDX_Control(pDX, IDC_COMBO_CAPTUREMETHOD, cmb_CaptureMethod);
	DDX_Control(pDX, IDC_MFCSHELLLIST_DIR, shell_Dir);
	DDX_Control(pDX, IDC_EDIT_PATH, txt_Path);
	DDX_Control(pDX, IDC_STATIC_PICTURE, pic_Display);
	DDX_Control(pDX, IDC_BUTTON_PUPDATEDATA, btn_UpdateData);
}


BEGIN_MESSAGE_MAP(CapturePage, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_WNDTITLE, &CapturePage::OnEnChangeEditWndtitle)
	ON_BN_CLICKED(IDC_BUTTON1, &CapturePage::OnBnClickedButton1)
	ON_NOTIFY(NM_CLICK, IDC_MFCSHELLLIST_DIR, &CapturePage::OnNMClickMfcshelllistDir)
	ON_EN_CHANGE(IDC_EDIT_PATH, &CapturePage::OnEnChangeEditPath)
	ON_EN_CHANGE(IDC_EDIT_HWND, &CapturePage::OnEnChangeEditHwnd)
	ON_BN_CLICKED(IDC_BUTTON_PUPDATEDATA, &CapturePage::OnBnClickedButtonPupdatedata)
END_MESSAGE_MAP()


// CapturePage 消息处理程序

void CapturePage::OnEnChangeEditWndtitle()
{
	CString Titletmp;
	txt_WNDTitle.GetWindowTextW(Titletmp);
	HWND hwnd = ::FindWindowW(NULL, Titletmp);
	Titletmp.Format(L"0x%x", hwnd);
	txt_HWND.SetWindowTextW(Titletmp);
}
void CapturePage::OnEnChangeEditHwnd()
{
	CString Shwnd, WNDTitle;
	txt_HWND.GetWindowTextW(Shwnd);
	HWND hwnd = (HWND)std::wcstol(Shwnd, nullptr, 16);
	wchar_t Titletmp[MAX_PATH] = { 0 };
	::GetWindowTextW(hwnd, Titletmp, MAX_PATH);
	WNDTitle.Format(L"%s", Titletmp);
	txt_WNDTitle.SetWindowTextW(WNDTitle);
}
void CapturePage::OnBnClickedButton1()
{
	CString Method, SavePath = L"D:\\1.png";
	cmb_CaptureMethod.GetLBText(cmb_CaptureMethod.GetCurSel(), Method);
	std::map<CString, int> MethodMap = { {L"桌面", 0}, {L"窗口", 1}, {L"区域", 2}};

	switch (MethodMap[Method]) {
	case 0:
	{
		Device.CaptureDesktop(SavePath);
		break;
	}
	case 1:
	{
		CString WNDTitle, Shwnd;
		txt_WNDTitle.GetWindowTextW(WNDTitle);
		txt_HWND.GetWindowTextW(Shwnd);
		if (WNDTitle.IsEmpty() && Shwnd.IsEmpty()) MessageBoxW(L"窗口标题或句柄任选一个填写。", L"警告", MB_ICONWARNING);
		Device.CaptureWindow((HWND)std::wcstol(Shwnd, nullptr, 16), SavePath);
		break;
	}
	case 2:
	{
		CString tmp;
		int PosX, PosY, Width, Height;
		txt_WNDLeft.GetWindowTextW(tmp); PosX = _wtoi(tmp);
		txt_WNDTop.GetWindowTextW(tmp);	PosY = _wtoi(tmp);
		txt_WNDWidth.GetWindowTextW(tmp); Width = _wtoi(tmp);
		txt_WNDHeight.GetWindowTextW(tmp); Height = _wtoi(tmp);
		Device.CaptureScreenArea(PosX, PosY, Width, Height, SavePath);
		break;
	}
	}
}

void CapturePage::OnNMClickMfcshelllistDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CString Pathtmp; std::vector<CString> Suffix = { L"bmp", L"jpg", L"jpeg", L"png", L"webp"};
	POSITION pos = shell_Dir.GetFirstSelectedItemPosition();
	int nItem = shell_Dir.GetNextSelectedItem(pos);
	if (nItem != -1) {
		shell_Dir.GetItemPath(Pathtmp, nItem);
		txt_Path.SetWindowTextW(Pathtmp);
		CString PicturePath = Pathtmp;
		Pathtmp = Pathtmp.Right(Pathtmp.GetLength() - Pathtmp.ReverseFind(L'\\') - 1);
		Pathtmp = Pathtmp.Right(Pathtmp.GetLength() - Pathtmp.ReverseFind(L'.') - 1);
		for (int i = 0; i < Suffix.size(); i++) {
			if (Pathtmp.MakeLower() == Suffix[i]) {
				//定义变量存储图片信息  
				BITMAPINFO* pBmpInfo;       //记录图像细节  
				BYTE* pBmpData;             //图像数据  
				BITMAPFILEHEADER bmpHeader; //文件头  
				BITMAPINFOHEADER bmpInfo;   //信息头  
				CFile bmpFile;              //记录打开文件  
				//以只读的方式打开文件 读取bmp图片各部分 bmp文件头 信息 数据  
				if (!bmpFile.Open(PicturePath, CFile::modeRead | CFile::typeBinary))
					return;
				if (bmpFile.Read(&bmpHeader, sizeof(BITMAPFILEHEADER)) != sizeof(BITMAPFILEHEADER))
					return;
				if (bmpFile.Read(&bmpInfo, sizeof(BITMAPINFOHEADER)) != sizeof(BITMAPINFOHEADER))
					return;
				pBmpInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)];
				//为图像数据申请空间  
				memcpy(pBmpInfo, &bmpInfo, sizeof(BITMAPINFOHEADER));
				DWORD dataBytes = bmpHeader.bfSize - bmpHeader.bfOffBits;
				pBmpData = (BYTE*)new char[dataBytes];
				bmpFile.Read(pBmpData, dataBytes);
				bmpFile.Close();
				//显示图像   
				CRect rect;
				pic_Display.GetClientRect(&rect); //获得pictrue控件所在的矩形区域  
				CDC* pDC = pic_Display.GetDC(); //获得pictrue控件的DC  
				pDC->SetStretchBltMode(COLORONCOLOR);
				StretchDIBits(pDC->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0,
					bmpInfo.biWidth, bmpInfo.biHeight, pBmpData, pBmpInfo, DIB_RGB_COLORS, SRCCOPY);
				break;
			}
		}
	}
	*pResult = 0;
}

void CapturePage::OnEnChangeEditPath()
{
	CString Pathtmp;
	txt_Path.GetWindowTextW(Pathtmp);
	shell_Dir.DisplayFolder(Pathtmp);
}


void CapturePage::OnBnClickedButtonPupdatedata()
{
	// TODO: 数据更新操作

	UpdateInfo.IsUpdate = false;
	UpdateInfo.DlgID = 0xFF;
	PageDlg[4]->ShowWindow(SW_SHOW);
	ShowWindow(SW_HIDE);
	btn_UpdateData.ShowWindow(SW_HIDE);
}
