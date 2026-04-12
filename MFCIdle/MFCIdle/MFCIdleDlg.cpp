
// MFCIdleDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCIdle.h"
#include "MFCIdleDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

WinSysDevice Device;

// 鼠标，键盘，截图，剪贴板，录制 对话框类
MousePage Page_Mouse;
KeyboardPage Page_Keyboard;
CapturePage Page_Capture;
ClipboardPage Page_Clipboard;
RecordPage Page_Record;

PageMap PageDlg = {
	{0, &Page_Mouse},
	{1, &Page_Keyboard},
	{2, &Page_Clipboard},
	{3, &Page_Capture},
	{4, &Page_Record},
};

std::map<CString, char> SwitchKeyMap = {
	{L"F1", VK_F1},
	{L"F2", VK_F2},
	{L"F3", VK_F3},
	{L"F4", VK_F4},
	{L"F5", VK_F5},
	{L"F6", VK_F6},
	{L"F7", VK_F7},
	{L"F8", VK_F8},
	{L"F9", VK_F9},
	{L"F10", VK_F10},
	{L"F11", VK_F11},
	{L"F12", VK_F12},
};

char RunKey = NULL; // 默认录制键为无

bool IsRunning = false; // 本文件全局运行状态
int TabIndex = 0;
// CMFCIdleDlg 对话框
CMFCIdleDlg::CMFCIdleDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCIDLE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCIdleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_IdleFunc, tab_idlefunc);
	DDX_Control(pDX, IDC_EDIT_LOG, txt_Log);
	DDX_Control(pDX, IDC_COMBO_RUNKEY, cmb_RunKey);
}

BEGIN_MESSAGE_MAP(CMFCIdleDlg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_IdleFunc, &CMFCIdleDlg::OnTcnSelchangeTabIdlefunc)
	ON_CBN_SELCHANGE(IDC_COMBO_RUNKEY, &CMFCIdleDlg::OnCbnSelchangeComboRunkey)
END_MESSAGE_MAP()


// CMFCIdleDlg 消息处理程序

BOOL CMFCIdleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 配置文件初始化
	CString logtmp = L"";
	//Idlecfg.SetFile(L"IdleTool.cfg");
	RunKey = SwitchKeyMap[L"F12"]; cmb_RunKey.SetCurSel(11);
	txt_Log.SetWindowTextW(L"初始化配置文件模块完成\r\n");
	
	// TODO: 日志初始化
	Idlelog.SetFile(L"IdleTool.log");
	txt_Log.GetWindowTextW(logtmp); 
	logtmp.AppendFormat(L"初始化日志模块完成\r\n日志文件位置: %s\r\n", Idlelog.GetFile().c_str());
	txt_Log.SetWindowTextW(logtmp);

	// TODO: 对话框界面初始化
	Page_Mouse.Create(IDD_MOUSEPAGE, &tab_idlefunc);
	Page_Keyboard.Create(IDD_KEYBOARDPAGE, &tab_idlefunc);
	Page_Capture.Create(IDD_CAPTUREPAGE, &tab_idlefunc);
	Page_Clipboard.Create(IDD_CLIPBOARDPAGE, &tab_idlefunc);
	Page_Record.Create(IDD_RECORDPAGE, &tab_idlefunc);
	
	ZeroMemory(&TabCard, sizeof(TCITEMW) * PageCount);
	ZeroMemory(&ImageList, sizeof(CImageList));
	ImageList.Create(32, 32, ILC_COLOR8, 0, PageCount);
	ImageList.Add(AfxGetApp()->LoadIcon(IDI_MOUSEICON));
	ImageList.Add(AfxGetApp()->LoadIcon(IDI_KEYBOARDICON));
	ImageList.Add(AfxGetApp()->LoadIcon(IDI_CLIPBOARDICON));
	ImageList.Add(AfxGetApp()->LoadIcon(IDI_CAPTUREICON));
	ImageList.Add(AfxGetApp()->LoadIcon(IDI_RECORDICON));
	tab_idlefunc.SetImageList(&ImageList);
	TabCard[0].pszText = L"鼠标";
	TabCard[1].pszText = L"键盘";
	TabCard[2].pszText = L"剪贴板";
	TabCard[3].pszText = L"截图";
	TabCard[4].pszText = L"录制";
	RECT TabCardArea = {5, 40, 635, 480};
	for (int i = 0; i < PageCount; i++) {
		PageDlg[i]->MoveWindow(&TabCardArea);
		TabCard[i].mask = TCIF_TEXT | TCIF_STATE | TCIF_IMAGE;
		TabCard[i].iImage = i;
		tab_idlefunc.InsertItem(i, &TabCard[i]);
	}
	PageDlg[0]->ShowWindow(SW_SHOW);
	
	RECT statusRect = { 20, 580, 1290, 610 }; int sbc_Parts[7] = { 165, 275, 445, 750, 1000, 1130, 1290 };
	sbc_Status.Create(WS_CHILD | WS_VISIBLE | CCS_NOPARENTALIGN, statusRect, this, IDC_STATUSBAR_INFO);
	sbc_Status.SetParts(7, sbc_Parts);
	sbc_Status.SetText(L"当前工具栏: 鼠标", 0, 0);
	sbc_Status.SetText(L"状态: 暂停中", 1, 0);
	sbc_Status.SetText(L"运行时长:  00:00:00", 2, 0);
	sbc_Status.SetText(L"脚本名称: ", 3, 0);
	sbc_Status.SetText(L"文件状态: NONE", 4, 0);
	sbc_Status.SetText(L"作者: HHH", 5, 0);
	sbc_Status.SetText(L"版本: v1.0.0", 6, 0);
	SetTimer(1, 1000, NULL); // 运行时间定时器，ID为1，间隔1秒
	SetTimer(2, 10, NULL); // 录制检测定时器，ID为2，间隔10毫秒
	txt_Log.GetWindowTextW(logtmp); 
	logtmp.AppendFormat(L"初始化界面完成\r\n");
	txt_Log.SetWindowTextW(logtmp);
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCIdleDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent){ // 检查定时器ID
	case 1:
		{
			static ULONG64 seconds = 0;
			seconds++;
			char hrs = seconds / 3600;
			char mins = (seconds % 3600) / 60;
			char secs = seconds % 60;
			CString timeStr;
			timeStr.Format(L"运行时长:  %02d:%02d:%02d", hrs, mins, secs);
			sbc_Status.SetText(timeStr, 2, 0);
		}
		break;
	case 2:
		{
			static UCHAR OutLogCount = 0; // 输出日志计数器，防止日志过多时频繁输出导致界面卡顿
			if (GetKeyState(RunKey) == -127) { 
				sbc_Status.SetText(L"状态: 运行中", 1, 0); IsRunning = true;
				if(!OutLogCount) UpdateLog(Logger::LOG_DEBUG, L"正在运行中...");
			}
			if (GetKeyState(RunKey) == -128) { 
				sbc_Status.SetText(L"状态: 暂停中", 1, 0); IsRunning = false;
				if(!OutLogCount) UpdateLog(Logger::LOG_DEBUG, L"运行已暂停"); 
			}
			if (GetKeyState(RunKey) == 0 || GetKeyState(RunKey) == 1) OutLogCount = 0; else OutLogCount++;
			break;
		}
	}
	CDialogEx::OnTimer(nIDEvent);
} 

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCIdleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCIdleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFCIdleDlg::OnTcnSelchangeTabIdlefunc(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (!UpdateInfo.IsUpdate){
		for (int i = 0; i < PageCount; i++)
			PageDlg[i]->ShowWindow(SW_HIDE);
		TabIndex = tab_idlefunc.GetCurSel();
		PageDlg[TabIndex]->ShowWindow(SW_SHOW);
		CString CurrentTool = L"当前工具栏: ", info;
		CurrentTool.AppendFormat(L"%s", TabCard[TabIndex].pszText);
		sbc_Status.SetText(CurrentTool, 0, 0);
		info.AppendFormat(L"工具栏 %s 选中", TabCard[TabIndex].pszText);
		UpdateLog(Logger::LOG_INFO, info);
	}else {
		//RecordPage* pRecordPage = (RecordPage*)PageDlg[4];
		tab_idlefunc.SetCurSel(UpdateInfo.DlgID);
	}
	*pResult = 0;
}


void CMFCIdleDlg::OnCbnSelchangeComboRunkey()
{
	CString strRunKey;
	cmb_RunKey.GetLBText(cmb_RunKey.GetCurSel(), strRunKey);
	RunKey = SwitchKeyMap[strRunKey];
}

void CMFCIdleDlg::UpdateLog(Logger::Level level, CString Info)
{
	CString Log, LogText, LogLevel;
	int psbc_Type = 0;
	switch (level)	
	{
	case Logger::LOG_INFO: LogLevel = L"[INFO,";  break;
	case Logger::LOG_DEBUG: LogLevel = L"[DEBUG,"; break;
	case Logger::LOG_WARN: LogLevel = L"[WARN,"; break;
	case Logger::LOG_ERROR: LogLevel = L"[ERROR,";
	}
	txt_Log.GetWindowTextW(LogText);
	std::wstring CurrentTime = Idlelog.GetTime();
	LogText.AppendFormat(L"%s%s]",LogLevel, CurrentTime.c_str());
	CString sbc_Text = sbc_Status.GetText(2, &psbc_Type);
	Log.AppendFormat(L"{%s}%s\r\n", sbc_Text, Info);
	LogText.AppendFormat(L"%s", Log);
	txt_Log.SetWindowTextW(LogText);
	txt_Log.LineScroll(txt_Log.GetLineCount());
	Idlelog.Info((std::wstring)Log);
}
