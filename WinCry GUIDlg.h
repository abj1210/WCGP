
// WinCry GUIDlg.h: 头文件
//

#pragma once

#include "cry.h"
#include "base64.h"
#include <atomic>

// 工作线程 -> 主线程 的自定义消息
#define WM_UI_SET_TEXT (WM_APP + 0x100)
#define WM_UI_ENABLE   (WM_APP + 0x101)
#define WM_UI_START    (WM_APP + 0x102)
#define WM_UI_DONE     (WM_APP + 0x103)

// WM_UI_SET_TEXT 的目标静态文本控件
enum UI_TEXT_FIELD
{
	UI_MODE = 1,
	UI_RES,
	UI_INFILE,
	UI_OUTFILE,
	UI_KEYRES
};

// WM_UI_ENABLE 的目标按钮
enum UI_ENABLE
{
	UI_EN_RUN = 1,
	UI_EN_ENCRYPT,
	UI_EN_DECRYPT,
	UI_EN_VERIFY
};


// CWinCryGUIDlg 对话框
class CWinCryGUIDlg : public CDialogEx
{
// 构造
public:
	CWinCryGUIDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WINCRY_GUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	// 工作线程句柄与运行状态
	CWinThread* m_pWorkerThread;
	std::atomic<bool> m_taskRunning;

	// 重写以安全处理对话框在任务运行时被关闭的情况
	virtual void OnCancel();
	afx_msg void OnDestroy();

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	runcrypt* runner = nullptr;
	DWORD dwElapsed = 0;
	ULONGLONG fileSize = 0;
	afx_msg void OnBnClickedOk();
	CStatic text_1;
	CEdit output;
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedButton1();
	CEdit filepath;
	afx_msg void OnBnClickedCheck1();
	CButton randomkey;
	CEdit keyinput;
	CProgressCtrl m_progressCtrl;
	CComboBox ctype;
	afx_msg void OnCbnSelchangeCombo1();
	CButton encrypt;
	CButton decrypt;
	CButton verify;
	CEdit outputpath;
	afx_msg void OnBnClickedButton3();
	CButton save;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CButton run;
	afx_msg void OnCbnSelchangeCombo2();
	CComboBox htype;
	afx_msg void OnNMCustomdrawProgress1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnUiSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUiEnable(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUiStart(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUiDone(WPARAM wParam, LPARAM lParam);
	// 等待工作线程结束并释放其 CWinThread 对象
	void CleanupWorker();
	CEdit rbuf;
	CButton over;
	CStatic ver;
	CStatic infile;
	CStatic outfile;
	CStatic keyres;
	CStatic mode;
	CStatic Res;
};
