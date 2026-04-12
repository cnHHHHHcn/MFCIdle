#pragma once
#include "afxdialogex.h"


// CapturePage 对话框

class CapturePage : public CDialogEx
{
	DECLARE_DYNAMIC(CapturePage)

public:
	CapturePage(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CapturePage();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CAPTUREPAGE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CEdit txt_Path;
	CEdit txt_WNDTitle;
	CEdit txt_HWND;
	CEdit txt_WNDLeft;
	CEdit txt_WNDTop;
	CEdit txt_WNDWidth;
	CEdit txt_WNDHeight;
	CStatic pic_Display;
	CImage img_Picture;
	CComboBox cmb_CaptureMethod;
	CMFCShellListCtrl shell_Dir;
	CButton btn_UpdateData;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnEnChangeEditWndtitle();
	afx_msg void OnNMClickMfcshelllistDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditPath();
	afx_msg void OnEnChangeEditHwnd();
	afx_msg void OnBnClickedButtonPupdatedata();
};
