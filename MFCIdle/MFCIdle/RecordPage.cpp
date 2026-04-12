// RecordPage.cpp: 实现文件
//

#include "pch.h"
#include "MFCIdle.h"
#include "afxdialogex.h"
#include "InputBox.h"
#include "MFCIdleDlg.h"
#include "RecordPage.h"

// RecordPage 对话框

/*
std::map<CString, int> PageID = {
	{L"鼠标", 0},
	{L"键盘", 1},
	{L"剪贴板", 2},
	{L"截图", 3}
};
*/

/*
CString MouseTranscribeKey; BYTE IsTranscribe = 0;


if (GetKeyState(MouseTranscribeKey[0]) == -127) IsTranscribe = 1;
if (GetKeyState(MouseTranscribeKey[0]) == -128) IsTranscribe = 2;
if (IsTranscribe == 1) {
	if (!hookMouse.HookHandle) hookMouse.IntsallHook(MouseHook::HOOK_MOUSE_LL, 0);
	hookMouse.DispatchMSG();
	CString MouseInfo[5];
	MouseInfo[0] = std::to_wstring(MouseKey).c_str();
	MouseInfo[1] = chk_Absolute.GetCheck() == 0 ? L"绝对" : L"相对";
	MouseInfo[2] = std::to_wstring(Pos.x).c_str();
	MouseInfo[3] = std::to_wstring(Pos.y).c_str();
	MouseInfo[4] = std::to_wstring(MouseWheel).c_str();
	MouseWheel = 0;
	//if(MouseKey == WinSysDevice::LeftClick || MouseKey == WinSysDevice::RightClick || MouseKey == WinSysDevice::MiddleClick ||
	//   MouseKey == WinSysDevice::LeftUp || MouseKey == WinSysDevice::RightUp || MouseKey == WinSysDevice::MiddleUp )
	//   MouseKey = WinSysDevice::MouseNULL;
	int LineIndex = lvw_MouseInfo.InsertItem(Index, std::to_wstring(Index + 1).c_str());
	cmb_Row.AddString(std::to_wstring(LineIndex + 1).c_str());
	for (int i = 0; i < 5; i++) {
		lvw_MouseInfo.SetItemText(LineIndex, i + 1, MouseInfo[i]);
	}
	Index++;
}
else if (IsTranscribe == 2) {
	hookMouse.UnInstallHook();
	IsTranscribe = 0;
}
*/

CMFCIdleDlg* pMain_Idle = nullptr;

UpdateActionDataDlg UpdateInfo;

RecordPage recordPage;
RecordPage* RecordPage::pRecordPage = nullptr;
RecordPage::RecordItemLinkedList* RecordDataPageList = nullptr;

void* ClipboardDataBuffer = nullptr;

WinSysDevice::MouseInfo MouseData;
WinSysDevice::KeyboardInfo KeyboardData;
DWORD PreviousTime = 0;
bool IsAbsolute = false, RecordSwitch = false;

char RecordKey = NULL; // 默认录制键为无
bool MovePass = false, IsRecording = false; // 本文件全局运行状态
class MouseHook : public WinSysHook {
	LRESULT OnMouseLL(int nCode, WPARAM wParam, LPARAM lParam) override {
		if (nCode >= 0) {
			if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN) MovePass = true;
			if (MovePass) {
				MSLLHOOKSTRUCT* mouseInfo = (MSLLHOOKSTRUCT*)lParam;
				RecordPage::RecordItem MouseItem;
				ZeroMemory(&MouseData, sizeof(WinSysDevice::MouseInfo));
				ZeroMemory(&MouseItem, sizeof(RecordPage::RecordItem));
				MouseItem.Type = RecordPage::Action_Mouse;
				MouseItem.StreamID = 0xFF;
				MouseItem.delay = static_cast<USHORT>(mouseInfo->time - PreviousTime);
				PreviousTime = mouseInfo->time;
				if (wParam == WM_MOUSEWHEEL) MouseData.WheelMove = HIWORD(mouseInfo->mouseData);
				MouseData.Key = Device.ExchangeMouseMSG(wParam);
				if (IsAbsolute)
					MouseData.AbsolutePos = Device.GetAbsolutePos();
				else
					MouseData.ScreenPos = Device.GetScreenPos();
				void* pMouseRecord = recordPage.FormatRecordItem(MouseItem, &MouseData);
				recordPage.AppendRecordItem(*RecordDataPageList, pMouseRecord, recordPage.RecordBaseMap);
				recordPage.DisplayRecordItemCallBack((RecordPage::RecordItem*)pMouseRecord, &MouseData);
				free(pMouseRecord);
			}
			if (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || wParam == WM_MBUTTONUP) MovePass = false;
			return CallNextHookEx(HookHandle, nCode, wParam, lParam);
		}
	}
} hookMouse;

std::wstring KeyWord;
class KeyboardHook : public WinSysHook {
	LRESULT OnKeyboardLL(int nCode, WPARAM wParam, LPARAM lParam) override {
		if (nCode >= 0) {
			bool AppendQueue = false;
			KBDLLHOOKSTRUCT* keyboardInfo = (KBDLLHOOKSTRUCT*)lParam;
			RecordPage::RecordItem  KeyboardItem;
			ZeroMemory(&KeyboardData, sizeof(WinSysDevice::KeyboardInfo));
			ZeroMemory(&KeyboardItem, sizeof(RecordPage::RecordItem));
			KeyboardItem.Type = RecordPage::Action_Keyboard;
			KeyboardItem.StreamID = 0xFF;
			KeyboardItem.delay = static_cast<USHORT>(keyboardInfo->time - PreviousTime);
			PreviousTime = keyboardInfo->time;
			AppendQueue = KeyboardItem.delay > 2000;
			if (wParam == WM_KEYDOWN) {
				if (GetKeyState(VK_CONTROL) & 0x8000) {
					char HotKey = keyboardInfo->vkCode;
					memcpy(&KeyboardData.CtrlHotKey, &HotKey, 1);
				}
				if (GetKeyState(VK_SHIFT) & 0x8000) {
				
				}
				KeyWord.append(1, keyboardInfo->vkCode);
			}
			if (AppendQueue) {
				KeyboardData.WordData.pData = (void*)KeyWord.data();
				KeyboardData.WordData.Size = KeyWord.size() * sizeof(wchar_t);
				void* pKeyboardRecord = recordPage.FormatRecordItem(KeyboardItem, &KeyboardData);
				recordPage.AppendRecordItem(*RecordDataPageList, pKeyboardRecord, recordPage.RecordBaseMap);
				recordPage.DisplayRecordItemCallBack((RecordPage::RecordItem*)pKeyboardRecord, &KeyboardData);
				free(pKeyboardRecord);
			}
			return CallNextHookEx(HookHandle, nCode, wParam, lParam);
		}
	}
} hookKeyboard;

IMPLEMENT_DYNAMIC(RecordPage, CDialogEx)

RecordPage::RecordPage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RECORDPAGE, pParent)
{
}

RecordPage::~RecordPage()
{
}

void RecordPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_RECORDQUEUE, lvw_RecordQueue);
	DDX_Control(pDX, IDC_COMBO_RROW, cmb_Row);
	DDX_Control(pDX, IDC_EDIT_RECORDFILENAME, txt_RecordFilePath);
	DDX_Control(pDX, IDC_STATIC_RECORDFILESIZE, lbl_RecordFileSize);
	DDX_Control(pDX, IDC_EDIT_DELAY, txt_Delay);
	DDX_Control(pDX, IDC_COMBO_ACTION, cmb_Action);
	DDX_Control(pDX, IDC_STATIC_USEDSTATE, lbl_UsedState);
	DDX_Control(pDX, IDC_COMBO_RECORDKEY, cmb_RecordKey);
}


BEGIN_MESSAGE_MAP(RecordPage, CDialogEx)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_SELECTFILE, &RecordPage::OnBnClickedButtonSelectfile)
	ON_BN_CLICKED(IDC_BUTTON_RINSERT, &RecordPage::OnBnClickedButtonRinsert)
	ON_BN_CLICKED(IDC_BUTTON_RDELETECURSELLIST, &RecordPage::OnBnClickedButtonRdeletecursellist)
	ON_BN_CLICKED(IDC_BUTTON_RCLEARLIST, &RecordPage::OnBnClickedButtonRclearlist)
	ON_BN_CLICKED(IDC_BUTTON_RMODIFYLINE, &RecordPage::OnBnClickedButtonRmodifyline)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_RECORDQUEUE, &RecordPage::OnLvnItemchangedListRecordqueue)
	ON_BN_CLICKED(IDC_BUTTON_MODIFYACTIONDATA, &RecordPage::OnBnClickedButtonModifyactiondata)
	ON_BN_CLICKED(IDC_BUTTON_MEMORYSORT, &RecordPage::OnBnClickedButtonMemorysort)
	ON_STN_CLICKED(IDC_BUTTON_MOUSEPOSMODE, &RecordPage::OnStnClickedStaticMouseposmode)
END_MESSAGE_MAP()


// RecordPage 消息处理程序

BOOL RecordPage::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	pRecordPage = this; // 记录当前页面指针，方便静态函数访问
	pMain_Idle = static_cast<CMFCIdleDlg*>(GetParent()->GetParent());
	LVCOLUMNW col[5]; ZeroMemory(&col, sizeof(LVCOLUMNW) * 5);
	col[0].pszText = L"序号"; col[1].pszText = L"动作"; col[2].pszText = L"详细信息"; col[3].pszText = L"延迟(ms)"; col[4].pszText = L"大小(B)";
	col[0].cx = 50; col[1].cx = 70; col[2].cx = 300; col[3].cx = 80; col[4].cx = 70;
	for (int i = 0; i < 5; i++) {
		col[i].mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
		col[i].fmt = LVCFMT_CENTER;
		lvw_RecordQueue.InsertColumn(i, &col[i]);
	}
	cmb_Action.SetCurSel(0);
	RecordKey = SwitchKeyMap[L"F10"];
	cmb_RecordKey.SetCurSel(9);
	SetTimer(1, 10, NULL);
	RecordDataPageList = (RecordItemLinkedList*)calloc(1, sizeof(RecordItemLinkedList));
	int Count = 0;
	while (true) {
		Count++;
		for (int i = 0; i < 10; i++) {
			WinSysDevice::MouseInfo MInfo; WinSysDevice::KeyboardInfo KInfo;
			WinSysDevice::ClipboardInfo CInfo; WinSysDevice::CaptureInfo PInfo;
			// Mouse
			RecordItem Item = { RecordPage::Action_Mouse, 0xFF, 0, 200 }; void* Data = nullptr;
			MInfo.Key = WinSysDevice::LeftClick;
			MInfo.ScreenPos = { 0xFF, 0xFF };
			MInfo.WheelMove = 120;
			Data = FormatRecordItem(Item, &MInfo);
			if (Data != nullptr) {
				AppendRecordItem(*RecordDataPageList, (RecordItem*)Data, RecordBaseMap);
				free(Data);
			};
			// Keyboard
			Item = { RecordPage::Action_Keyboard, 0xFF, 0, 200 };
			KInfo.CtrlHotKey = WinSysDevice::Copy;
			memcpy(KInfo.LanguageName, L"en_US", 12);
			KInfo.WithShift = false;
			std::wstring KeyData(512, 0x41);
			KInfo.WordData.pData = (void*)KeyData.data();
			KInfo.WordData.Size = (USHORT)(KeyData.size() * sizeof(wchar_t));
			Data = FormatRecordItem(Item, &KInfo);
			if (Data != nullptr) {
				AppendRecordItem(*RecordDataPageList, (RecordItem*)Data, RecordBaseMap);
				free(Data);
			}
			// Clipboard
			Item = { RecordPage::Action_Clipboard, 0xFF, 0, 200 };
			CInfo.Version = WinSysDevice::Unicode;
			CInfo.Operation = 'S';
			std::wstring ClipData(512, 0x42);
			CInfo.ClipData.pData = (void*)ClipData.data();
			CInfo.ClipData.Size = (USHORT)(ClipData.size() * sizeof(wchar_t));
			Data = FormatRecordItem(Item, &CInfo);
			if (Data != nullptr) {
				AppendRecordItem(*RecordDataPageList, (RecordItem*)Data, RecordBaseMap);
				free(Data);
			}
			// Capture
			Item = { RecordPage::Action_Capture, 0xFF, 0, 200 };
			memcpy(PInfo.FilePath, L"C:\\Test\\Capture.png", 38);
			PInfo.Size = 38;
			Data = FormatRecordItem(Item, &PInfo);
			if (Data != nullptr) {
				AppendRecordItem(*RecordDataPageList, (RecordItem*)Data, RecordBaseMap);
				free(Data);
			}
		}
		if (Count == 1) break;
		ClearRecordItem(*RecordDataPageList);
	}
	WriteRecordFile(L"Record.rd", *RecordDataPageList);
	//ReadRecordFile(L"Record.rd", RecordDataPageList);
	ForEachRecordItem(*RecordDataPageList, DisplayRecordItemCallBack); // this->lvw_RecordQueue 安全可用
	//ClearRecordItem(*RecordDataPageList);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void RecordPage::InsertRecordInfo(UINT Line, bool IsActionChange)
{
	CString StrTmp;
	if (IsActionChange) {
		cmb_Action.GetWindowTextW(StrTmp);
		lvw_RecordQueue.SetItemText(Line, 1, StrTmp);
	}
	txt_Delay.GetWindowTextW(StrTmp);
	StrTmp = StrTmp.IsEmpty() ? L"0" : StrTmp;
	lvw_RecordQueue.SetItemText(Line, 3, StrTmp);
}

void RecordPage::GetRecordInfoForEdit(UINT Line)
{
	cmb_Action.SetCurSel(cmb_Action.FindString(0, lvw_RecordQueue.GetItemText(Line, 1)));
	txt_Delay.SetWindowTextW(lvw_RecordQueue.GetItemText(Line, 3));
}

void RecordPage::Record(bool Switch)
{
	if (Switch) {
		//hookMouse.InstallHook(hookMouse.HOOK_MOUSE_LL, NULL);
		hookKeyboard.InstallHook(hookKeyboard.HOOK_KEYBOARD_LL, NULL);
		SetTimer(2, 10, NULL);
	}else {
		KillTimer(2);
		//hookMouse.UnInstallHook();
		hookKeyboard.UnInstallHook();
	}
	CButton* btn_MousePosMode = static_cast<CButton*>(GetDlgItem(IDC_BUTTON_MOUSEPOSMODE));
	btn_MousePosMode->EnableWindow(!Switch);
}

void RecordPage::OnTimer(UINT_PTR nIDEvent) {
	CDialogEx::OnTimer(nIDEvent);
	switch (nIDEvent) {
	case 1:
		{
			static UCHAR OutLogCount = 0; // 输出日志计数器，防止日志过多时频繁输出导致界面卡顿
			if (GetKeyState(RecordKey) == -127) {
				pMain_Idle->sbc_Status.SetText(L"状态: 录制中", 1, 0); IsRecording = true;
				if (!OutLogCount) pMain_Idle->UpdateLog(Logger::LOG_DEBUG, L"正在录制中...");
			}
			if (GetKeyState(RecordKey) == -128) {
				pMain_Idle->sbc_Status.SetText(L"状态: 暂停中", 1, 0); IsRecording = false;
				if (!OutLogCount) pMain_Idle->UpdateLog(Logger::LOG_DEBUG, L"录制已暂停");
			}
			Record(IsRecording);
			if (GetKeyState(RecordKey) == 0 || GetKeyState(RecordKey) == 1) OutLogCount = 0; else OutLogCount++;
		}
		break;
	case 2:
		{
			//hookMouse.SetHookObject(hookMouse.HookHandle);
			//hookMouse.DispatchMSG();
			hookKeyboard.SetHookObject(hookKeyboard.HookHandle);
			hookKeyboard.DispatchMSG();
		}
		break;
	}
}

void RecordPage::OnBnClickedButtonRinsert()
{
	int LineIndex = lvw_RecordQueue.InsertItem(lvw_RecordQueue.GetItemCount(), std::to_wstring(lvw_RecordQueue.GetItemCount() + 1).c_str());
	cmb_Row.AddString(std::to_wstring(LineIndex + 1).c_str()); InsertRecordInfo(LineIndex, true);
}

void RecordPage::OnBnClickedButtonRdeletecursellist()
{
	int DeleteCount = 0, SelectCount = lvw_RecordQueue.GetSelectedCount();
	for (int i = lvw_RecordQueue.GetItemCount() - 1; i > -1; i--) {
		if (lvw_RecordQueue.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED) {
			lvw_RecordQueue.DeleteItem(i);
			DeleteRecordItem(*RecordDataPageList, RecordBaseMap, i);
			if (++DeleteCount == SelectCount) break;
		}
	}
	cmb_Row.ResetContent();
	for (int i = 0; i < lvw_RecordQueue.GetItemCount(); i++) {
		cmb_Row.AddString(std::to_wstring(i + 1).c_str());
		lvw_RecordQueue.SetItemText(i, 0, std::to_wstring(i + 1).c_str());
	}
}

void RecordPage::OnBnClickedButtonRclearlist()
{
	lvw_RecordQueue.DeleteAllItems();
	cmb_Row.ResetContent();
	ClearRecordItem(*RecordDataPageList);
}

/*
void RecordPage::OnBnClickedButtonRmodifyline()
{
	CString tmp; 
	UINT Row = cmb_Row.GetCurSel();
	tmp = lvw_RecordQueue.GetItemText(Row, 1);
	int DlgIndex = cmb_Action.FindString(0, tmp);
	if (Row != 0) {
		tmp = lvw_RecordQueue.GetItemText(Row, 1);
		if (tmp.IsEmpty() || DlgIndex == CB_ERR) 
			MessageBoxW(L"未指定动作类型，请选择一个动作类型！", L"警告", MB_ICONWARNING);
		cmb_Action.GetWindowTextW(tmp);
		if (lvw_RecordQueue.GetItemText(Row, 1).Find(tmp) == -1) {
			int RTNCode = MessageBoxW(L"是否更改记录数据项？", L"询问", MB_ICONINFORMATION | MB_YESNO);
			// 
			if (RTNCode == IDYES) {
				UINT CurrentIndex = Row;
				char StreamID = static_cast<RecordItem*>(RecordBaseMap[CurrentIndex])->StreamID;
				char Action = static_cast<RecordItem*>(RecordBaseMap[CurrentIndex])->Type;
				char RecordDelta = 1; bool IsSameAction = false; int DelEndPos = CurrentIndex;
				if (StreamID == 0xFF) {
					IsSameAction = (Action == static_cast<RecordItem*>(RecordBaseMap[CurrentIndex - 1])->Type);
					if (IsSameAction)
						RecordDelta = static_cast<RecordItem*>(RecordBaseMap[CurrentIndex - 1])->StreamID + 1;
				}else {
					RecordDelta = static_cast<RecordItem*>(RecordBaseMap[DelEndPos++])->StreamID;
					while (static_cast<RecordItem*>(RecordBaseMap[DelEndPos++])->StreamID != 0xFF)
						RecordDelta++;
				}
				for (int Index = DelEndPos; DelEndPos - RecordDelta  < Index; Index--) {
					lvw_RecordQueue.SetItemState(Index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					DeleteRecordItem(*RecordDataPageList, RecordBaseMap, Index);
				}
				lvw_RecordQueue.SetFocus();
				int InsertIndex = lvw_RecordQueue.InsertItem((DelEndPos - RecordDelta + 1), std::to_wstring(DelEndPos - RecordDelta + 2).c_str());
				InsertRecordInfo(InsertIndex, true); 
				OnBnClickedButtonModifyactiondata();
				return;
			}
		}
		InsertRecordInfo(Row, false);
	}else {
		MessageBoxW(L"未找到指定数据行!", L"警告", MB_ICONWARNING);
	}
}
*/

void RecordPage::OnBnClickedButtonRmodifyline()
{
	CString tmp;
	// 1. 获取用户输入的行号
	UINT Row = cmb_Row.GetCurSel();
	// 2. 获取该行当前的动作类型文本，并在下拉框中查找对应的索引
	// 注意: 这里用 _wtoi(tmp) 再次转换，其实可以直接用 Row，但为了保持原逻辑不变
	tmp = lvw_RecordQueue.GetItemText(Row, 1);
	int DlgIndex = cmb_Action.FindString(0, tmp);
	// 3. 核心逻辑: 只有当行号有效时才执行
	if (Row != CB_ERR) {
		// 获取列表中该行的动作类型文本
		tmp = lvw_RecordQueue.GetItemText(Row, 1);
		// 校验: 如果文本为空 或 下拉框没找到对应项，提示警告
		if (tmp.IsEmpty() || DlgIndex == CB_ERR){
			MessageBoxW(L"未指定动作类型，请选择一个动作类型！", L"警告", MB_ICONWARNING);
			return; 
		}
		// 获取下拉框当前选中的动作文本
		cmb_Action.GetWindowTextW(tmp);
		// 4. 判断动作是否真的发生了改变
		// 如果列表中的文本包含下拉框的文本（Find != -1），说明没变，直接更新数据即可
		if (lvw_RecordQueue.GetItemText(Row, 1).Find(tmp) == -1) {
			// 动作变了，询问用户是否确认修改（因为可能涉及删除后续关联记录）
			int RTNCode = MessageBoxW(L"是否更改记录数据项？", L"询问", MB_ICONINFORMATION | MB_YESNO);
			if (RTNCode == IDYES) {
				// --- 开始执行复杂的删除与重插逻辑 ---
				UINT CurrentIndex = Row; // 当前要修改的行索引（0-based）
				// 获取当前记录的流 ID 和 动作类型
				UCHAR StreamID = static_cast<RecordItem*>(RecordBaseMap[CurrentIndex])->StreamID;
				UCHAR Action = static_cast<RecordItem*>(RecordBaseMap[CurrentIndex])->Type;
				char RecordDelta = 1;      // 用于计算需要删除的记录数量偏移量
				bool IsSameAction = false; // 标记前一个动作是否与当前动作相同
				int DelEndPos = CurrentIndex; // 删除范围的结束位置
				// 【核心算法】计算受影响的记录范围
				if (StreamID == 0xFF) {
					// 情况 A: 当前记录是独立流 (0xFF)
					// 检查前一条记录的动作类型是否和当前一样
					if (CurrentIndex != CB_ERR) {
						IsSameAction = (Action == static_cast<RecordItem*>(RecordBaseMap[CurrentIndex - 1])->Type);
						if (IsSameAction) {
							// 如果动作相同，说明它们属于同一个连续操作流
							// 记录流ID差值 是前一个记录的 StreamID + 1
							RecordDelta = static_cast<RecordItem*>(RecordBaseMap[CurrentIndex - 1])->StreamID + 1;
						}
					}
				}else {
					// 情况 B: 当前记录属于某个连续流 (StreamID != 0xFF)
					// 向后遍历，找到这个连续流的最后一个记录结束标志（0xFF）
					RecordDelta = static_cast<RecordItem*>(RecordBaseMap[DelEndPos++])->StreamID;
					// 循环直到遇到下一个独立流 (0xFF) 或者链表结束
					while (DelEndPos < RecordBaseMap.size() &&
						static_cast<RecordItem*>(RecordBaseMap[DelEndPos])->StreamID != 0xFF) {
						RecordDelta++;
						DelEndPos++;
					}
				}
				// 5. 执行删除操作
				// 从后往前删除，避免索引错乱
				// 删除范围: [DelEndPos - RecordDelta, DelEndPos)
				for (int Index = DelEndPos; Index > (DelEndPos - RecordDelta); Index--) {
					// 选中该行（视觉反馈）
					lvw_RecordQueue.DeleteItem(Index);
					// 从数据结构中删除
					if(Index != (DelEndPos - RecordDelta + 1)) 
						DeleteRecordItem(*RecordDataPageList, RecordBaseMap, Index);
				}
				// 让列表控件获得焦点，确保选中状态可见
				lvw_RecordQueue.SetFocus();
				// 6. 插入新记录
				// 在删除后的空位插入新的一项
				int InsertIndex = lvw_RecordQueue.InsertItem((DelEndPos - RecordDelta + 1), std::to_wstring(DelEndPos - RecordDelta + 2).c_str());
				// 填充新记录的数据（true 表示是新增/修改模式）
				InsertRecordInfo(InsertIndex, true);
				return; // 修改完成，退出
			}
		}
		// 如果动作没变，或者用户选择了“否”（原逻辑似乎没处理 No 的情况，这里默认走更新流程）
		// 仅更新当前行的数据，不删除后续关联项
		InsertRecordInfo(Row, false);
	}else {
		// 行号无效
		MessageBoxW(L"未找到指定数据行!", L"警告", MB_ICONWARNING);
	}
}

void RecordPage::OnBnClickedButtonSelectfile()
{
	CString Path; DWORD FileSize;
	txt_RecordFilePath.GetWindowTextW(Path);
	if (!Path.IsEmpty() && !FileExists(Path)) {
		MessageBoxW(L"没用找到指定文件！", L"警告", MB_ICONWARNING);
		return;
	}
	if (Path.IsEmpty()) {
		SelectRecordFile();
		txt_RecordFilePath.GetWindowTextW(Path);
	}
	CString tmp;
	tmp.Format(L"脚本名称: %s", Path.Mid(Path.ReverseFind(L'\\') + 1));
	pMain_Idle->sbc_Status.SetText(tmp, 3, 0);
	pMain_Idle->sbc_Status.SetText(L"文件状态: 解析中...", 4, 0);
	if (ValidRecordFile(Path)) {
		FileSize = GetRecordFileSize(Path);
		pMain_Idle->sbc_Status.SetText(L"文件状态: 读取成功", 4, 0);
		tmp.Format(L"文件大小:%s", FormatBytes(FileSize));
		lbl_RecordFileSize.SetWindowTextW(tmp);
		ClearRecordItem(*RecordDataPageList);
		ReadRecordFile(Path, *RecordDataPageList);
		ForEachRecordItem(*RecordDataPageList, RelocateStringInfoCallBack);
		ForEachRecordItem(*RecordDataPageList, BuildIndexMapCallBack);
		ForEachRecordItem(*RecordDataPageList, DisplayRecordItemCallBack);
	}else {
		pMain_Idle->sbc_Status.SetText(L"文件状态: 数据格式错误", 4, 0);
	}
}

void RecordPage::OnLvnItemchangedListRecordqueue(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	cmb_Row.SetCurSel(pNMLV->iItem);
	GetRecordInfoForEdit(pNMLV->iItem);
	*pResult = 0;
}

void RecordPage::OnBnClickedButtonModifyactiondata()
{
	CString ActionTmp = lvw_RecordQueue.GetItemText(cmb_Row.GetCurSel(), 1);
	ShowWindow(SW_HIDE);
	// 每一次更新，标志为true；更新完成让对应对话框设为false;
	UpdateInfo.IsUpdate = true;
	UpdateInfo.DlgID = cmb_Action.FindString(0, ActionTmp);
	CButton* btn_commonUpdate = static_cast<CButton*>(PageDlg[UpdateInfo.DlgID]->GetDlgItem(IDC_BUTTON_MUPDATEDATA + UpdateInfo.DlgID));
	btn_commonUpdate->ShowWindow(SW_SHOW);
	// 获取 Idle 主窗口对象  Idle -> TAB Card -> RecordPage
	pMain_Idle->tab_idlefunc.SetCurSel(UpdateInfo.DlgID);
	PageDlg[UpdateInfo.DlgID]->ShowWindow(SW_SHOW);
}

void RecordPage::OnBnClickedButtonMemorysort()
{
	//CString Prompt = L"das;f";
	//MessageBoxW(Prompt, L"Tips", MB_ICONINFORMATION);
	CString DisplayStr;
	DisplayStr.Format(L"内存占用[效能]: %s", DisplayUilization(*RecordDataPageList));
	lbl_UsedState.SetWindowTextW(DisplayStr);
}


void RecordPage::OnStnClickedStaticMouseposmode()
{
	CButton* btn_MousePosMode = static_cast<CButton*>(GetDlgItem(IDC_BUTTON_MOUSEPOSMODE));
	CString Context = L"鼠标位置模式: ";
	IsAbsolute = !IsAbsolute;
	Context.AppendFormat(IsAbsolute ? L"相对" : L"绝对");
	btn_MousePosMode->SetWindowTextW(Context);
}

void* RecordPage::FormatRecordItem(RecordItem Item, void* DeviceInfo)
{
	if (!DeviceInfo) return nullptr;
	USHORT TotalSize = 0;
	// 精确计算每种类型所需的真实总大小
	switch (Item.Type) {
	case ActionType::Action_Mouse:
		TotalSize = sizeof(WinSysDevice::MouseInfo);
		break;
	case ActionType::Action_Keyboard: {
		auto* pInfo = static_cast<WinSysDevice::KeyboardInfo*>(DeviceInfo);
		TotalSize = sizeof(WinSysDevice::KeyboardInfo) + pInfo->WordData.Size;
		break;
	}
	case ActionType::Action_Clipboard: {
		auto* pInfo = static_cast<WinSysDevice::ClipboardInfo*>(DeviceInfo);
		TotalSize = sizeof(WinSysDevice::ClipboardInfo) + pInfo->ClipData.Size;
		break;
	}
	case ActionType::Action_Capture: {
		auto* pInfo = static_cast<WinSysDevice::CaptureInfo*>(DeviceInfo);
		TotalSize = sizeof(pInfo->Size) + pInfo->Size;
		break;
	}
	default:
		return nullptr; // 不支持的类型
	}

	// 分配内存并清零（防垃圾数据）
	Item.DataSize = TotalSize;
	void* pItemBlock = calloc(1, Item.DataSize + sizeof(RecordItem));
	if (!pItemBlock) return nullptr;
	// 写入头部和主体
	memcpy(pItemBlock, &Item, sizeof(RecordItem));
	memcpy((char*)pItemBlock + sizeof(RecordItem), DeviceInfo, TotalSize);

	if (Item.Type == ActionType::Action_Keyboard || Item.Type == ActionType::Action_Clipboard) {
		size_t DeviceInfoOffset = (Item.Type == ActionType::Action_Keyboard) ? sizeof(WinSysDevice::KeyboardInfo) : sizeof(WinSysDevice::ClipboardInfo);
		void* pExtraBase = (char*)pItemBlock + sizeof(RecordItem) + DeviceInfoOffset;
		WinSysDevice::StringInfo* SrcInfo = (WinSysDevice::StringInfo*)((char*)pItemBlock + sizeof(RecordItem));
		if (SrcInfo->Size > 0 && SrcInfo->pData) memcpy(pExtraBase, SrcInfo->pData, SrcInfo->Size);
	}
	return pItemBlock;
}

bool RecordPage::AppendRecordItem(RecordItemLinkedList& MemorySlab, void* Item, RecordIndexMapping& IndexMap)
{
	if (Item == nullptr) return false;
	RecordItemLinkedList* CurrentSlab = &MemorySlab;
	RecordItem* pItem = (RecordItem*)Item;
	// 遍历链表找到最后一个节点
	while (CurrentSlab->Next) CurrentSlab = CurrentSlab->Next;
	size_t TotalSize = sizeof(RecordItem) + pItem->DataSize;
	// 如果当前链表没有分配内存，则分配新的链表节点
	if (!CurrentSlab->pBuffer) {
		CurrentSlab->pBuffer = VirtualAlloc(NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!CurrentSlab->pBuffer) return false;
		CurrentSlab->BufferCursor = 0;
	}
	// 如果当前链表剩余空间不足以存储新的记录项，则分配新的链表节点
	if (CurrentSlab->BufferCursor + TotalSize > 0x1000) {
		CurrentSlab->Next = (RecordItemLinkedList*)calloc(1, sizeof(RecordItemLinkedList));
		if (!CurrentSlab->Next) return false;
		CurrentSlab->Next->pBuffer = VirtualAlloc(NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!CurrentSlab->Next->pBuffer) {
			free(CurrentSlab->Next);
			return false;
		}
		CurrentSlab->Next->BufferCursor = 0;
		CurrentSlab = CurrentSlab->Next;
	}
	if (pItem->Type == ActionType::Action_Keyboard || pItem->Type == ActionType::Action_Clipboard) {
		size_t DeviceInfoOffset = (pItem->Type == ActionType::Action_Keyboard) ? sizeof(WinSysDevice::KeyboardInfo) : sizeof(WinSysDevice::ClipboardInfo);
		WinSysDevice::StringInfo* pStrInfo = (WinSysDevice::StringInfo*)((char*)Item + sizeof(RecordItem));
		pStrInfo->pData = (char*)CurrentSlab->pBuffer + CurrentSlab->BufferCursor + sizeof(RecordItem) + DeviceInfoOffset;
	}
	char* WritePos = static_cast<char*>(CurrentSlab->pBuffer) + CurrentSlab->BufferCursor;
	IndexMap.push_back(WritePos);
	memcpy(WritePos, Item, sizeof(RecordItem));
	memcpy(WritePos + sizeof(RecordItem), (char*)Item + sizeof(RecordItem), pItem->DataSize);
	CurrentSlab->BufferCursor += (WORD)TotalSize;
	return true;
}

bool RecordPage::DeleteRecordItem(RecordItemLinkedList& MemorySlab, RecordIndexMapping& IndexMap, int Index)
{
	if (MemorySlab.pBuffer == nullptr) return false;
	if (IndexMap.empty()) return false;
	if (Index >= IndexMap.size()) return false;
	ULONG64 RecordItemBase = (ULONG64)IndexMap[Index] & PAGE_BASE_MASK;
	RecordItemLinkedList* CurrentSlab = &MemorySlab;
	while (CurrentSlab && (ULONG64)CurrentSlab->pBuffer != RecordItemBase) CurrentSlab = CurrentSlab->Next;
	RecordItem* Item = (RecordItem*)IndexMap[Index]; USHORT RecordItemSize = sizeof(RecordItem) + Item->DataSize;
	// 获取当前数据末尾位置
	WORD RecordNextCursor = ((WORD)IndexMap[Index] & PAGE_OFFSET_MASK) + RecordItemSize;
	void* pClearAddr = IndexMap[Index];
	// 如果当前内存页的数据游标位置 和 当前数据末尾位置 不一致，则需要移动往后的数据到当前数据起始位置
	if (CurrentSlab->BufferCursor != RecordNextCursor) {
		memmove(IndexMap[Index], (char*)IndexMap[Index] + RecordItemSize, CurrentSlab->BufferCursor - RecordNextCursor);
		pClearAddr = (char*)CurrentSlab->pBuffer + CurrentSlab->BufferCursor - RecordItemSize;
	}
	memset(pClearAddr, 0, RecordItemSize); IndexMap.erase(IndexMap.begin() + Index);
	// 如果这个记录也在同一个内存页内，则更新指针: 向前移动 RecordItemSize 字节   
	// Index < IndexMap.size() 防止访问越界  tips : && 短路运算
	while (Index < IndexMap.size() && RecordItemBase == ((ULONG64)IndexMap[Index] & PAGE_BASE_MASK)) {
		IndexMap[Index] = (char*)IndexMap[Index] - RecordItemSize; Index++;
	}
	CurrentSlab->BufferCursor -= RecordItemSize;
	return true;
}

bool RecordPage::ClearRecordItem(RecordItemLinkedList& MemorySlab)
{
	if (MemorySlab.pBuffer == nullptr && MemorySlab.Next == nullptr) return true;
	// 1. 只遍历 Next 链
	RecordItemLinkedList* Current = MemorySlab.Next;
	while (Current != nullptr) {
		RecordItemLinkedList* Next = Current->Next;
		if (Current->pBuffer) {
			VirtualFree(Current->pBuffer, 0, MEM_RELEASE);
			Current->pBuffer = NULL;
		}
		free(Current); // 这里 free 的是 calloc 出来的节点，是安全的
		Current = Next;
	}
	// 2. 单独处理头节点的缓冲区
	if (MemorySlab.pBuffer) {
		VirtualFree(MemorySlab.pBuffer, 0, MEM_RELEASE);
		MemorySlab.pBuffer = nullptr;
	}
	MemorySlab.BufferCursor = 0;
	MemorySlab.Next = nullptr;
	return true;
}

bool RecordPage::ModifyRecordItem(RecordItemLinkedList& MemorySlab, RecordItem* NewItem, RecordIndexMapping& IndexMap, int Index)
{
	if (MemorySlab.pBuffer == nullptr) return false;
	if (NewItem == nullptr) return false;
	if (IndexMap.empty()) return false;
	if (Index >= IndexMap.size()) return false;
	USHORT OldRecordItemSize = sizeof(RecordItem), NewRecordItemSize = sizeof(RecordItem);
	RecordItem* OldItem = (RecordItem*)IndexMap[Index];
	OldRecordItemSize += OldItem->DataSize;
	NewRecordItemSize += NewItem->DataSize;
	if (OldRecordItemSize == NewRecordItemSize) {
		memcpy(OldItem, NewItem, NewRecordItemSize);
		return true;
	}else {
		RecordItemLinkedList* CurrentSlab = &MemorySlab;
		ULONG64 RecordItemBase = (ULONG64)OldItem & PAGE_BASE_MASK;
		USHORT RecordCurrentCursor = ((USHORT)OldItem & PAGE_OFFSET_MASK);
		while ((ULONG64)CurrentSlab->pBuffer != RecordItemBase)
			CurrentSlab = CurrentSlab->Next;
		RecordItemLinkedList* NewSlab = (RecordItemLinkedList*)calloc(1, sizeof(RecordItemLinkedList));
		NewSlab->pBuffer = VirtualAlloc(NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (NewSlab->pBuffer == nullptr) return false;
		NewSlab->Next = CurrentSlab->Next;
		CurrentSlab->Next = NewSlab;
		SHORT RecordItemSizeDelta = OldRecordItemSize - NewRecordItemSize;
		USHORT AfterDataSize = CurrentSlab->BufferCursor - RecordCurrentCursor - OldRecordItemSize, ChangeSize;
		memcpy(NewSlab->pBuffer, (char*)IndexMap[Index] + OldRecordItemSize, AfterDataSize);
		NewSlab->BufferCursor = AfterDataSize;
		if (RecordItemSizeDelta > 0) {
			memcpy(IndexMap[Index], NewItem, NewRecordItemSize);
			ChangeSize = AfterDataSize + RecordItemSizeDelta;
			memset((char*)IndexMap[Index] + NewRecordItemSize, 0, ChangeSize);
		}else {
			RecordItemLinkedList* NewRecordSlab = (RecordItemLinkedList*)calloc(1, sizeof(RecordItemLinkedList));
			NewRecordSlab->pBuffer = VirtualAlloc(NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			if (NewRecordSlab->pBuffer == nullptr) return false;
			NewRecordSlab->BufferCursor = 0; NewRecordSlab->Next = CurrentSlab->Next;
			CurrentSlab->Next = NewRecordSlab;
			memcpy(NewRecordSlab->pBuffer, NewItem, NewRecordItemSize);
			NewRecordSlab->BufferCursor = NewRecordItemSize;
			ChangeSize = AfterDataSize + OldRecordItemSize;
			memset((char*)IndexMap[Index], 0, ChangeSize);
		}
		CurrentSlab->BufferCursor -= ChangeSize;
		CompactSlab(MemorySlab, IndexMap);
		return true;
	}
	return false;
}

bool RecordPage::ForEachRecordItem(RecordItemLinkedList& MemorySlab, RecordCallBack cbfn)
{
	if (MemorySlab.pBuffer == nullptr) return false;
	if (cbfn == nullptr) return false;
	RecordItemLinkedList* CurrentSlab = &MemorySlab;
	// 遍历链表中的记录项，并调用回调函数进行处理
	while (CurrentSlab) {
		size_t Offset = 0;
		// 如果记录项的大小超过当前链表剩余空间，则需要切换到下一个链表继续读取
		while (Offset + sizeof(RecordItem) <= CurrentSlab->BufferCursor) {
			RecordItem* Item = (RecordItem*)((char*)CurrentSlab->pBuffer + Offset);
			void* DeviceInfo = (char*)CurrentSlab->pBuffer + Offset + sizeof(RecordItem);
			// 调用回调函数进行处理，并根据返回值决定是否继续遍历
			if (!cbfn(Item, DeviceInfo)) return false;
			// 计算下一个记录项的偏移位置
			Offset += Item->DataSize + sizeof(RecordItem);
		}
		// 如果当前链表已经遍历完，则切换到下一个链表
		CurrentSlab = CurrentSlab->Next;
	}
	return true;
}

bool RecordPage::PlayRecordItemCallBack(RecordItem* Item, void* DeviceInfo)
{
	if (Item == nullptr) return false;
	if (DeviceInfo == nullptr) return false;
	switch (Item->Type)
	{
		case RecordPage::Action_Mouse:
			// 模拟鼠标事件
			if (Item->DataSize != sizeof(WinSysDevice::MouseInfo)) return false; // 数据大小不匹配，无法处理
			if (Item->StreamID != 0xFF) return false; // 鼠标事件不应该有StreamID，数据格式错误
			Device.MouseSend(*(WinSysDevice::MouseInfo*)DeviceInfo);
			break;
		case RecordPage::Action_Keyboard:
		{
			// 模拟键盘事件
			WinSysDevice::KeyboardInfo* KInfo = (WinSysDevice::KeyboardInfo*)DeviceInfo;
			if (KInfo->CtrlHotKey) Device.CtrlHotKey(KInfo->CtrlHotKey);
			if (KInfo->LanguageName) {
				WinSysDevice::LayoutMap InputInfo;
				InputInfo = Device.InputMethod(WinSysDevice::List_All);
				for(int i = 0; i < InputInfo.size(); i++) {
					if (StrCmpCW(InputInfo[i].LanguageName.c_str(), KInfo->LanguageName)) {
						Device.InputMethod(WinSysDevice::Set_Appoint, InputInfo[i].LayoutHandle);
						break;
					}
				}
			}
			if (!KInfo->WordData.Size) {
				std::wstring KeyTmp; 
				KeyTmp.resize(KInfo->WordData.Size / sizeof(wchar_t));
				memcpy((void*)KeyTmp.data(), KInfo->WordData.pData, KInfo->WordData.Size);
				Device.KeyboardSend(KeyTmp, KInfo->WithShift);
			}
			break;
		}
		case RecordPage::Action_Clipboard:
		{
			// 处理剪贴板数据
			WinSysDevice::ClipboardInfo* CInfo = (WinSysDevice::ClipboardInfo*)DeviceInfo;
			switch (CInfo->Operation) {
			case 0x53: // S
				if (CInfo->Version == 'A') {
					std::string ClipData((char*)CInfo->ClipData.pData, CInfo->ClipData.Size);
					Device.ClipboardSetTextA(ClipData);
				}
				else {
					std::wstring ClipData((wchar_t*)CInfo->ClipData.pData, CInfo->ClipData.Size / sizeof(wchar_t));
					Device.ClipboardSetTextW(ClipData);
				}
				break;
			case 0x47: // G
				if (CInfo->Version == 'A') {
					std::string ClipData = Device.ClipboardGetTextA();
					size_t DataSize = ClipData.size() * sizeof(char);
					if (DataSize > PAGE_OFFSET_MASK) return false; // 数据过大，无法处理
					ClipboardDataBuffer = calloc(1, DataSize);
					if (!ClipboardDataBuffer) return false;
					memcpy(ClipboardDataBuffer, ClipData.data(), DataSize);
				}else {
					std::wstring ClipData = Device.ClipboardGetTextW();
					size_t DataSize = ClipData.size() * sizeof(wchar_t);
					if (DataSize > PAGE_OFFSET_MASK) return false; // 数据过大，无法处理
					ClipboardDataBuffer = calloc(1, DataSize);
					if (!ClipboardDataBuffer) return false;
					memcpy(ClipboardDataBuffer, ClipData.data(), DataSize);
				}
				break;
			case 0x50: // P
				if (CInfo->Version == 'A') {
					Device.ClipboardSendTextA(std::string((char*)CInfo->ClipData.pData, CInfo->ClipData.Size), 0);
				}else {
					Device.ClipboardSendTextW(std::wstring((wchar_t*)CInfo->ClipData.pData, CInfo->ClipData.Size / sizeof(wchar_t)), 0);
				}
				break;
			case 0x43: // C
				Device.ClipboardClear();
			}
			break;
		}
		case RecordPage::Action_Capture:
			// 处理屏幕截图数据
			// TODO: 需要和 OpenCV 等图像处理库配合使用，解析截图数据并模拟相应的操作
		break;
	}
	// 进行延迟处理
	Sleep(Item->delay); 
	return true;
}

bool RecordPage::DisplayRecordItemCallBack(RecordItem* Item, void* DeviceInfo)
{
	if (Item == nullptr) return false;
	if (DeviceInfo == nullptr) return false;
	pRecordPage->lvw_RecordQueue.InsertItem(pRecordPage->lvw_RecordQueue.GetItemCount(), std::to_wstring(pRecordPage->lvw_RecordQueue.GetItemCount() + 1).c_str());
	pRecordPage->lvw_RecordQueue.SetItemText(pRecordPage->lvw_RecordQueue.GetItemCount() - 1, 3, std::to_wstring(Item->delay).c_str());
	pRecordPage->lvw_RecordQueue.SetItemText(pRecordPage->lvw_RecordQueue.GetItemCount() - 1, 4, std::to_wstring(sizeof(RecordItem) + Item->DataSize).c_str());
	pRecordPage->cmb_Row.AddString(std::to_wstring(pRecordPage->lvw_RecordQueue.GetItemCount()).c_str()); CString ActionDataInfo = L"数据格式错误";
	switch (Item->Type)
	{
	case RecordPage::Action_Mouse:
		// 显示鼠标事件信息
		pRecordPage->lvw_RecordQueue.SetItemText(pRecordPage->lvw_RecordQueue.GetItemCount() - 1, 1, L"鼠标");
		if (Item->DataSize == sizeof(WinSysDevice::MouseInfo)) {
			WinSysDevice::MouseInfo* MInfo = (WinSysDevice::MouseInfo*)DeviceInfo;
			std::wstring PositionType = *(ULONG64*)&MInfo->ScreenPos != 0 ? L"绝对" : L"相对";
			POINT Pos = *(ULONG64*)&MInfo->ScreenPos != 0 ? MInfo->ScreenPos : MInfo->AbsolutePos;
			ActionDataInfo.Format(L"按键:%d, 位置: (%s; %d, %d), 滚轮: %d",MInfo->Key, PositionType.c_str(), Pos.x, Pos.y, MInfo->WheelMove);
		}
		break;
	case RecordPage::Action_Keyboard:
		// 显示键盘事件信息
		pRecordPage->lvw_RecordQueue.SetItemText(pRecordPage->lvw_RecordQueue.GetItemCount() - 1, 1, L"键盘");
		{
			WinSysDevice::KeyboardInfo* KInfo = (WinSysDevice::KeyboardInfo*)DeviceInfo;
			char ShiftState = KInfo->WithShift ? 0x59 : 0x4E; 
			std::wstring KeyContext((wchar_t*)KInfo->WordData.pData, KInfo->WordData.Size / sizeof(wchar_t));
			ActionDataInfo.Format(L"Ctrl热键: %c, 输入法: %s, Shift: %c, 按键字符: %ls", KInfo->CtrlHotKey, (*(ULONG64*)KInfo->LanguageName ? KInfo->LanguageName : L"默认"), ShiftState, KeyContext.c_str());
			break;
		}
		break;
	case RecordPage::Action_Clipboard:
		{
			// 显示剪贴板事件信息
			pRecordPage->lvw_RecordQueue.SetItemText(pRecordPage->lvw_RecordQueue.GetItemCount() - 1, 1, L"剪贴板");
			WinSysDevice::ClipboardInfo* CInfo = (WinSysDevice::ClipboardInfo*)DeviceInfo;
			if (CInfo->ClipData.Size && CInfo->ClipData.pData) {
				if (CInfo->Version == WinSysDevice::ANSI) {
					std::string ClipContext((char*)CInfo->ClipData.pData, CInfo->ClipData.Size);
					ActionDataInfo.Format(L"操作: %c, 数据类型: %c, 内容: %s", CInfo->Operation, CInfo->Version, ClipContext.c_str());
				}else {
					std::wstring ClipContext((wchar_t*)CInfo->ClipData.pData, CInfo->ClipData.Size / sizeof(wchar_t));
					ActionDataInfo.Format(L"操作: %c, 数据类型: %c, 内容: %ls", CInfo->Operation, CInfo->Version, ClipContext.c_str());
				}
			}
			break;
		}
	case RecordPage::Action_Capture:
		// 显示屏幕截图事件信息
		pRecordPage->lvw_RecordQueue.SetItemText(pRecordPage->lvw_RecordQueue.GetItemCount() - 1, 1, L"截图");
		WinSysDevice::CaptureInfo* PInfo = (WinSysDevice::CaptureInfo*)DeviceInfo;
		if (PInfo->Size) ActionDataInfo.Format(L"文件位置: %ls", std::wstring(PInfo->FilePath, PInfo->Size / sizeof(wchar_t)).c_str());
		break;
	}
	pRecordPage->lvw_RecordQueue.SetItemText(pRecordPage->lvw_RecordQueue.GetItemCount() - 1, 2, ActionDataInfo);
	return true;
}

bool RecordPage::RelocateStringInfoCallBack(RecordItem* Item, void* DeviceInfo)
{
	if (Item == nullptr) return false;
	if (DeviceInfo == nullptr) return false;
	if (Item->Type == ActionType::Action_Keyboard || Item->Type == ActionType::Action_Clipboard) {
		USHORT Offset = NULL; WinSysDevice::StringInfo* pStrInfo = (WinSysDevice::StringInfo*)DeviceInfo;
		void* pBaseTable = nullptr, * pDeviceBaseTable = nullptr;
		// 获取当前数据页的基址和字符串结构体里的基址
		pBaseTable = (void*)((ULONG64)Item & PAGE_BASE_MASK);
		pDeviceBaseTable = (void*)((ULONG64)pStrInfo->pData & PAGE_BASE_MASK);
		// 数据页的基址和字符串结构体里的基址判定字符指针是有需要重定位
		if (pBaseTable != pDeviceBaseTable) {
			Offset = (USHORT)pStrInfo->pData & PAGE_OFFSET_MASK;
			pStrInfo->pData = (char*)pBaseTable + Offset;
		}
	}
	return true;
}

bool RecordPage::BuildIndexMapCallBack(RecordItem* Item, void* DeviceInfo)
{
	pRecordPage->RecordBaseMap.clear();
	pRecordPage->RecordBaseMap.push_back(Item);
	return true;
}

bool RecordPage::ValidRecordFile(const wchar_t* Path)
{
	if (!Path) return false;
	DWORD BytesReaded = 0; BOOL RTN = FALSE;
	FileHead FileHeader; PageHead PageHeader;
	HANDLE hFile = CreateFileW(Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		RTN = ReadFile(hFile, &FileHeader, sizeof(FileHead), &BytesReaded, NULL);
		if (!RTN || FileHeader.Magic != FileMagic || BytesReaded != sizeof(FileHead)) {
			CloseHandle(hFile); 
			return false;
		}
		DWORD FileSize = NULL;
		LARGE_INTEGER Offset, CurrentFilePos;
		FileSize = GetFileSize(hFile, NULL);
		CurrentFilePos.QuadPart = sizeof(FileHead);
		while (CurrentFilePos.QuadPart < FileSize) {
			RTN = ReadFile(hFile, &PageHeader, sizeof(PageHead), &BytesReaded, NULL);
			if (!RTN || PageHeader.Magic != PageMagic || BytesReaded != sizeof(PageHead)) {
				CloseHandle(hFile);
				return false;
			}
			Offset.QuadPart = PageHeader.Size;
			SetFilePointerEx(hFile, Offset, &CurrentFilePos, FILE_CURRENT);
		}
		CloseHandle(hFile);
		return CurrentFilePos.QuadPart == FileSize;
	}
	return false;
}

bool RecordPage::WriteRecordFile(const wchar_t* Path, RecordItemLinkedList Buffer)
{
	FileHead FileHeader = {FileMagic, IDLE_Version};
	DWORD BytesWrited = 0;	BOOL RTN = FALSE;
	if (!Path) return false;
	if (Buffer.pBuffer == nullptr) return false;
	HANDLE hFile = CreateFileW(Path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return false;
	// 先写入文件签名和块标志
	RTN = WriteFile(hFile, &FileHeader, sizeof(FileHead), &BytesWrited, NULL);
	if (!RTN || BytesWrited != sizeof(FileHead)) {
		CloseHandle(hFile);
		return false;
	}
	RecordItemLinkedList* CurrentSlab = &Buffer;
	while (CurrentSlab && CurrentSlab->pBuffer) {
		PageHead PageHeader;
		PageHeader.Magic = PageMagic;
		PageHeader.Size = CurrentSlab->BufferCursor;
		RTN = WriteFile(hFile, &PageHeader, sizeof(PageHead), &BytesWrited, NULL);
		if (!RTN || BytesWrited != sizeof(PageHead)) {
			CloseHandle(hFile);
			return false;
		}	
		RTN = WriteFile(hFile, CurrentSlab->pBuffer, CurrentSlab->BufferCursor, &BytesWrited, NULL);
		if (!RTN || BytesWrited != CurrentSlab->BufferCursor) {
			CloseHandle(hFile);
			return false;
		}
		CurrentSlab = CurrentSlab->Next;
	}
	CloseHandle(hFile);
	return true;
}

bool RecordPage::ReadRecordFile(const wchar_t* Path, RecordItemLinkedList& Buffer)
{
	if (Path == nullptr) return false;
	if (Buffer.pBuffer) return false;
	HANDLE hFile = CreateFileW(Path, GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		BOOL RTN = FALSE; DWORD BytesReaded = 0; FileHead Signature = { 0 };
		RTN = ReadFile(hFile, &Signature, sizeof(FileHead), &BytesReaded, NULL);
		// 验证文件签名和块标志，确保文件格式正确
		if (RTN && BytesReaded == sizeof(FileHead) && Signature.Magic == FileMagic) {
			/* TODO: 未来在这做每个文件版本读取*/
			// ......
			PageHead PageHeader; RecordItemLinkedList* CurrentSlab = &Buffer;
			// 逐页读取文件内容，并将数据存储到链表中
			while (ReadFile(hFile, &PageHeader, sizeof(PageHead), &BytesReaded, NULL) && BytesReaded == sizeof(PageHead)) {
				// 验证页标志，确保每页数据块的格式正确
				if (PageHeader.Magic != PageMagic) {
					CloseHandle(hFile);
					return false;
				}
				// 分配内存并读取当前页的数据块到链表节点中
				CurrentSlab->pBuffer = VirtualAlloc(NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
				if (!CurrentSlab->pBuffer) {
					CloseHandle(hFile); free(CurrentSlab);
					return false;
				}
				RTN = ReadFile(hFile, CurrentSlab->pBuffer, PageHeader.Size, &BytesReaded, NULL);
				if (!RTN || BytesReaded != PageHeader.Size) return false;
				CurrentSlab->BufferCursor = PageHeader.Size;
				CurrentSlab->Next = (RecordItemLinkedList*)calloc(1, sizeof(RecordItemLinkedList));
				if (!CurrentSlab->Next) return false;
				CurrentSlab = CurrentSlab->Next;
			}
		}
		CloseHandle(hFile);
		return true;
	}
	return false;
}


void RecordPage::SelectRecordFile()
{
	CFileDialog SelectFileDlg(TRUE, NULL, NULL, OFN_NOTESTFILECREATE, L"Record Files(*.rd)|*.rd|All Files (*.*)|*.*||", pRecordPage);
	if (SelectFileDlg.DoModal() == IDOK) {
		CString RecordFileName = SelectFileDlg.GetPathName();
		txt_RecordFilePath.SetWindowTextW(RecordFileName);
	}
	delete SelectFileDlg;
}

bool RecordPage::FileExists(const wchar_t* Path) {
	DWORD attr = GetFileAttributesW(Path);
	return (attr != INVALID_FILE_ATTRIBUTES &&
		!(attr & FILE_ATTRIBUTE_DIRECTORY));
}

DWORD RecordPage::GetRecordFileSize(const wchar_t* Path)
{
	if (Path == nullptr) return false;
	if (wcslen(Path) == NULL) return false;
	HANDLE hFile = CreateFileW(Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD FileSize = 0;
	if (hFile != INVALID_HANDLE_VALUE) {
		FileSize = GetFileSize(hFile, NULL);
		CloseHandle(hFile);
	}
	return FileSize;
}

CString RecordPage::FormatBytes(DWORD BytesSize)
{
	const wchar_t* Units[4] = {L"B", L"KB", L"MB", L"GB"};
	char Index = 0; float Size = BytesSize; CString RTN;
	while (Size >= 1024) {
		Size /= 1024;
		Index++;
	}
	if (Size == (int)BytesSize || Size == (int)Size){
		RTN.Format(L"%d%s", (int)Size, Units[Index]);
	}else{
		RTN.Format(L"%.2f%s", Size, Units[Index]);
	}
	return RTN;
}

bool RecordPage::CompactSlab(RecordItemLinkedList& MemorySlab, RecordIndexMapping& IndexMap)
{
	if (MemorySlab.pBuffer == nullptr) return false;
	if (IndexMap.empty()) return false;
	RecordItemLinkedList* CurrentSlab = &MemorySlab, * NewMemorySlab = (RecordItemLinkedList*)calloc(1, sizeof(RecordItemLinkedList));
	IndexMap.clear();
	while (CurrentSlab != nullptr) {
		short CurrentCursor = NULL;
		while (CurrentCursor < CurrentSlab->BufferCursor) {
			void* pRecordItemPos = (char*)CurrentSlab->pBuffer + CurrentCursor;
			RecordItem* pItem = (RecordItem*)pRecordItemPos;
			AppendRecordItem(*NewMemorySlab, pItem, IndexMap);
			CurrentCursor += sizeof(RecordItem) + pItem->DataSize;
		}
		CurrentSlab = CurrentSlab->Next;
	}
	ClearRecordItem(MemorySlab);
	memcpy(&MemorySlab, NewMemorySlab, sizeof(RecordItemLinkedList));
	free(NewMemorySlab);
	return true;
}

float RecordPage::GetMemorySlabUilization(RecordItemLinkedList MemorySlab)
{
	if (MemorySlab.pBuffer != nullptr) {
		RecordItemLinkedList* CurrentSlab = &MemorySlab;
		UINT UsedBytes = NULL, TotalBytes = NULL;
		while (CurrentSlab && CurrentSlab->pBuffer) {
			UsedBytes += CurrentSlab->BufferCursor;
			TotalBytes += 0x1000;
			CurrentSlab = CurrentSlab->Next;
		}
		if (UsedBytes && TotalBytes) return ((float)UsedBytes / TotalBytes) * 100.0f;
	}
	return 0.0f;
}

CString RecordPage::DisplayUilization(RecordItemLinkedList MemorySlab)
{
	CString Display;
	if (MemorySlab.pBuffer != nullptr) {
		RecordItemLinkedList* CurrentSlab = &MemorySlab;
		UINT UsedBytes = NULL, TotalBytes = NULL;
		while (CurrentSlab && CurrentSlab->pBuffer) {
			UsedBytes += CurrentSlab->BufferCursor;
			TotalBytes += 0x1000;
			CurrentSlab = CurrentSlab->Next;
		}
		Display.Format(L"%s/%s (%.2f%%)", FormatBytes(UsedBytes), FormatBytes(TotalBytes), GetMemorySlabUilization(MemorySlab));
		return Display;
	}
	return L"0B/0B(0.00%)";
}
