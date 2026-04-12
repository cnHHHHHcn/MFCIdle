#pragma once

// MFCIdleDlg.h: 头文件

// 子对话框个数
#define PageCount 5

// CMFCIdleDlg 对话框
class CMFCIdleDlg : public CDialogEx
{
// 构造
public:
	CMFCIdleDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCIDLE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//Configuration Idlecfg;
	Logger Idlelog;
	CEdit txt_Log;
	CTabCtrl tab_idlefunc;
	TCITEMW TabCard[PageCount] = { 0 };
	CComboBox cmb_RunKey;
	CImageList ImageList;
	CStatusBarCtrl sbc_Status;
	void UpdateLog(Logger::Level level, CString Info);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnTcnSelchangeTabIdlefunc(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeComboRunkey();
};
