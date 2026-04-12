#pragma once
#include "afxdialogex.h"



// MousePage 对话框

class MousePage : public CDialogEx
{
	DECLARE_DYNAMIC(MousePage)

public:
	MousePage(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~MousePage();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MOUSEPAGE };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:

public:
	UINT Index = 0;
	CEdit txt_MousePos;
	CEdit txt_MouseWheel;
	CButton chk_Absolute;
	CButton chk_IsGetPos;
	CComboBox cmb_MouseKey;
	CComboBox cmb_Row;
	CStatic lbl_DesktopSize;
	CListCtrl lvw_MouseInfo;
	void InsertMouseInfo(UINT Line);
	void GetMouseInfoForEdit(UINT Line);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonMInsert();
	afx_msg void OnBnClickedButtonMDeleteCurSelList();
	afx_msg void OnBnClickedButtonMClearList();
	afx_msg void OnBnClickedButtonMmodifyline();
	afx_msg void OnCbnSelchangeComboMrow();
	afx_msg void OnBnClickedButtonMousedisplay();
	afx_msg void OnLvnItemchangedListMouseinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonUpdatedata();
	CButton btn_UpdateData;
	afx_msg void OnBnClickedButtonMupdatedata();
};

