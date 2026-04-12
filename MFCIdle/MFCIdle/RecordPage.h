#pragma once
#include "afxdialogex.h"
#include <windows.h>
#include <functional>

// RecordPage 对话框

#define FileMagic			0x50434B4D			// MKCP
#define PageMagic			0x4750				// PG
#define PAGE_BASE_MASK		0xFFFFFFFFFFFF0000	// 内存页基址掩码
#define PAGE_OFFSET_MASK    0xFFFF              // 内存页偏移掩码

#define DefualtFileSize  L"文件大小:0B"

class RecordPage : public CDialogEx
{
	DECLARE_DYNAMIC(RecordPage)

public:
	RecordPage(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~RecordPage();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RECORDPAGE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	static RecordPage* pRecordPage;
public:
	virtual BOOL OnInitDialog();

	CListCtrl lvw_RecordQueue;
	CComboBox cmb_Row;
	CEdit txt_RecordFilePath;
	CStatic lbl_RecordFileSize;
	CEdit txt_Delay;
	CComboBox cmb_Action;
	CStatic lbl_UsedState;
	CComboBox cmb_RecordKey;
	void InsertRecordInfo(UINT Line, bool IsActionChange);
	void GetRecordInfoForEdit(UINT Line);
	void Record(bool Switch);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonRinsert();
	afx_msg void OnBnClickedButtonRdeletecursellist();
	afx_msg void OnBnClickedButtonRclearlist();
	afx_msg void OnBnClickedButtonRmodifyline();
	afx_msg void OnBnClickedButtonSelectfile();
	afx_msg void OnLvnItemchangedListRecordqueue(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonModifyactiondata();
	afx_msg void OnBnClickedButtonMemorysort();
	afx_msg void OnStnClickedStaticMouseposmode();


	// LPS-Core 链表页式内存池 (Linked-List + Page-based + Storage)
	
	// FileHead 结构体设计为记录数据文件头部标志，包含一个标志和一个版本字段，用于校验文件的有效性
	struct FileHead {
		DWORD Magic;			// MKCP
		DWORD Version;			// 版本
	};

	// PageHead 结构体设计为紧跟在链表节点前面，包含一个标志和一个大小字段，用于标识当前页的数据块和其大小
	struct PageHead {
		WORD Magic = PageMagic;	// PG
		WORD Size = 0;			// 紧跟在结构体后面
	};
	
	// RecordItemLinkedList 设计为一个链表结构，每个节点包含一个指向数据缓冲区的指针、一个记录当前缓冲区使用情况的游标，以及一个指向下一个链表节点的指针
	struct RecordItemLinkedList {
		LPVOID pBuffer = NULL;					// 指向当前链表节点的数据缓冲区，大小为4KB
		WORD BufferCursor = NULL;				// 记录当前链表节点缓冲区的使用情况，表示已经存储了多少字节的数据
		RecordItemLinkedList* Next = nullptr;	// 指向下一个链表节点的指针，如果没有下一个节点则为nullptr
	};
	// 注: PageHead 和 RecordItemLinkedList 的设计是为了支持文件中记录项的分页存储，避免单个链表节点过大导致内存分配失败的问题
	// 每个链表节点存储一个或多个记录项，直到达到一定大小（如4KB），然后创建新的链表节点继续存储后续的记录项
	// 这样设计的好处是可以更高效地管理内存，避免一次性分配过大的内存块，同时也方便在文件中按页存储记录项，便于读取和写入操作
	// RecordItemLinkedLIst 的 BufferCursor 是代表当前内存中链表节点已经使用了多少字节的数据
	// PageHead 的 Size 则是代表当前文件中存储的所有记录项的总大小，紧跟在 PageHead 结构体后面的是实际的记录项数据

	typedef std::vector<void*> RecordIndexMapping;

	RecordIndexMapping RecordBaseMap;

	using ActionType = enum : char {
		Action_Mouse = 'M',
		Action_Keyboard = 'K',
		Action_Clipboard = 'C',
		Action_Capture = 'P',
	};
	// RecordItem 结构体设计为紧凑存储，记录动作类型、数据流ID、数据块大小和时间间隔等信息，紧跟在结构体后面的是实际的数据内容
	struct RecordItem {
		ActionType Type;		// 记录动作类型
		UCHAR StreamID = 0xFF;	// 用于数据切片，记录同一数据流ID 最大支持64kb数据，末尾用0xFF标记结束
		USHORT DataSize;		// 记录数据块大小，紧跟在结构体后面
		USHORT delay;			// 记录动作发生时与上一个动作的时间间隔，单位为毫秒
	};
	using RecordCallBack = std::function<bool(RecordItem* Item, void* DeviceInfo)>;
	// 前提条件:传入的 RecordItemLinkedList 结构体必须通过动态内存分配（即位于堆中，函数 malloc, calloc, realloc），不可为栈上局部变量。（不听劝计算机就哭给你看！）
	void* FormatRecordItem(RecordItem Item, void* DeviceInfo);																// 记录数据项格式化
	bool AppendRecordItem(RecordItemLinkedList& MemorySlab, void* Item, RecordIndexMapping& IndexMap);						// 追加记录数据项
	bool DeleteRecordItem(RecordItemLinkedList& MemorySlab, RecordIndexMapping& IndexMap, int Index);						// 删除记录数据项(指定)
	bool ClearRecordItem(RecordItemLinkedList& MemorySlab);																	// 清除记录数据项(全部)
	bool ModifyRecordItem(RecordItemLinkedList& MemorySlab, RecordItem* NewItem, RecordIndexMapping& IndexMap, int Index);	// 修改记录数据项
	bool ForEachRecordItem(RecordItemLinkedList& MemorySlab, RecordCallBack cbfn);											// 历遍记录数据项
	static bool PlayRecordItemCallBack(RecordItem* Item, void* DeviceInfo);													// 播放记录数据项(回调)
	static bool DisplayRecordItemCallBack(RecordItem* Item, void* DeviceInfo);												// 显示记录数据项(回调)
	static bool RelocateStringInfoCallBack(RecordItem* Item, void* DeviceInfo);												// 重定位额外数据(回调)
	static bool BuildIndexMapCallBack(RecordItem* Item, void* DeviceInfo);													// 创建指针索引表(回调)
	bool ValidRecordFile(const wchar_t* Path);																				// 验证记录数据文件有效性
	bool WriteRecordFile(const wchar_t* Path, RecordItemLinkedList Buffer);													// 记录数据项文件写入
	bool ReadRecordFile(const wchar_t* Path, RecordItemLinkedList& Buffer);													// 记录数据项文件读取
	void SelectRecordFile();																								// 选取记录数据文件
	bool FileExists(const wchar_t* Path);																					// 确定文件是否存在
	DWORD GetRecordFileSize(const wchar_t* Path);																			// 获取记录数据文件大小
	CString FormatBytes(DWORD Size);																						// 文件大小格式化
	bool CompactSlab(RecordItemLinkedList& MemorySlab, RecordIndexMapping& IndexMap);										// 整理记录数据项（增强内存利用率）
	float GetMemorySlabUilization(RecordItemLinkedList MemorySlab);															// 获取内存利用率（0.00% - 100.00%）
	CString DisplayUilization(RecordItemLinkedList MemorySlab);																// 显示内存利用/占用


};

// --- 内存布局说明 ---
// 1. RecordItemLinkedList 结构体用于管理记录项的链表，每个节点包含一个4KB的数据缓冲区
// 2. RecordItem 结构体用于存储具体的记录项信息，包括动作类型、数据大小和时间间隔等
// 3. 紧跟在 RecordItem 结构体后面的是实际的数据内容，大小由 RecordItem 的 Size 字段指定
// 4. 链表节点中的 BufferCursor 字段用于跟踪当前节点已使用的字节数，便于追加新的记录项
// 即: [RecordItemLinkedList][RecordItem][Data][RecordItem][Data]...[RecordItemLinkedList][RecordItem][Data]...

// --- 文件布局说明 ---
// 1. 文件开头包含一个4字节的签名 "MKCP" 用于标识文件类型
// 2. PageHead 结构体用于标识文件中的每页数据块，每页数据块开头包含一个 "PG"标志 和 页面数据块大小
// 3. 紧跟在 PageHead 结构体后面的是实际的记录项数据(每个记录项:[RecordItme][Data])
// 4. 文件中可以包含多个 PageHead 和对应的记录项数据块，便于分块存储和读取
// 即: ["MKCP"][PageHead][RecordItem][Data]...[PageHead][RecordItem][Data]...