#pragma once
#include "afxdialogex.h"


// KeyboardPage 对话框

class KeyboardPage : public CDialogEx
{
	DECLARE_DYNAMIC(KeyboardPage)

public:

	KeyboardPage(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~KeyboardPage();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_KEYBOARDPAGE };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	UINT Index = 0;
	CComboBox cmb_Layout;
	CComboBox cmb_CtrlHotKey;
	CComboBox cmb_Row;
	CButton chk_Shift;
	CEdit txt_KeyWord;
	CEdit txt_lvwKeyboardSet;
	CListCtrl lvw_KeyboardInfo;
	void InsertKeyboardInfo(UINT Line);
	void GetKeyboardInfoForEdit(UINT Line);
	afx_msg void OnBnClickedButtonLayoutnext();
	afx_msg void On_AppointLayout();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonKInsert();
	afx_msg void OnBnClickedButtonKDeleteCurSelList();
	afx_msg void OnBnClickedButtonKClearList();
	afx_msg void OnBnClickedButtonKmodifyline();
	afx_msg void OnLvnItemchangedListKeyboardinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeComboKrow();
	CButton btn_UpdateData;
	afx_msg void OnBnClickedButtonKupdatedata();
};
