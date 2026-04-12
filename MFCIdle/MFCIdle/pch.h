// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include "framework.h"

#include <Map>
// 功能模块
#include "WinSysDevice.h"
#include "WinSysHook.h"
#include "Configuration.h"
#include "Logger.h"
#include "opencv2/opencv.hpp"

// 对话框类
#include "MFCIdleDlg.h"
#include "MousePage.h"
#include "KeyboardPage.h"
#include "ClipboardPage.h"
#include "CapturePage.h"
#include "RecordPage.h"


#endif //PCH_H

#define IDLE_Version 0x01000000      // V1.0.0

typedef std::map<int, CDialogEx*> PageMap;

struct UpdateActionDataDlg {
	bool IsUpdate;
	char DlgID;
};

extern WinSysDevice Device;    // 全局系统设备对象

//extern CMFCIdleDlg Main_Idle;
extern MousePage Page_Mouse;
extern KeyboardPage Page_Keyboard;
extern ClipboardPage Page_Clipboard;
extern CapturePage Page_Capture;
extern RecordPage Page_Record;
extern PageMap PageDlg;
extern std::map<CString, char> SwitchKeyMap;

extern UpdateActionDataDlg UpdateInfo;