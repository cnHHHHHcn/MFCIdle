#pragma once
#include "afxdialogex.h"


// ClipboardPage 对话框

class ClipboardPage : public CDialogEx
{
	DECLARE_DYNAMIC(ClipboardPage)

public:
	ClipboardPage(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ClipboardPage();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIPBOARDPAGE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CEdit txt_ClipText;
	CComboBox cmb_ClipMode;
	CComboBox cmb_ClipWordVersion;
	afx_msg void OnBnClickedButtonCliprun();

	CButton btn_UpdateData;
	afx_msg void OnBnClickedButtonCupdatedata();
};
