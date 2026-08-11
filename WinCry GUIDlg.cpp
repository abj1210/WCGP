
// WinCry GUIDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "WinCry GUI.h"
#include "WinCry GUIDlg.h"
#include "afxdialogex.h"
#include <iostream>
#include <string>

#define WCGP_VERSION "1.1.0"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CWinCryGUIDlg 对话框



CWinCryGUIDlg::CWinCryGUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WINCRY_GUI_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pWorkerThread = nullptr;
	m_taskRunning.store(false);
}

void CWinCryGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, filepath);
	DDX_Control(pDX, IDC_CHECK1, randomkey);
	DDX_Control(pDX, IDC_EDIT4, keyinput);
	DDX_Control(pDX, IDC_PROGRESS1, m_progressCtrl);
	DDX_Control(pDX, IDC_COMBO1, ctype);
	DDX_Control(pDX, IDC_RADIO1, encrypt);
	DDX_Control(pDX, IDC_RADIO2, decrypt);
	DDX_Control(pDX, IDC_RADIO3, verify);
	DDX_Control(pDX, IDC_EDIT3, outputpath);
	DDX_Control(pDX, IDC_BUTTON3, save);
	DDX_Control(pDX, IDOK, run);
	DDX_Control(pDX, IDC_COMBO2, htype);
	DDX_Control(pDX, IDC_EDIT5, rbuf);
	DDX_Control(pDX, IDCANCEL, over);
	DDX_Control(pDX, IDC_TEXT, ver);
	DDX_Control(pDX, IDC_STATIC1, infile);
	DDX_Control(pDX, IDC_STATIC2, outfile);
	DDX_Control(pDX, IDC_STATIC3, keyres);
	DDX_Control(pDX, IDC_STATIC5, Res);
	DDX_Control(pDX, IDC_STATIC10, result_box);
	DDX_Control(pDX, IDC_EDIT1, file_size);
	DDX_Control(pDX, IDC_EDIT6, time_cnt);
	DDX_Control(pDX, IDC_EDIT7, speed);
}

BEGIN_MESSAGE_MAP(CWinCryGUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, &CWinCryGUIDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO1, &CWinCryGUIDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CWinCryGUIDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CWinCryGUIDlg::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_BUTTON1, &CWinCryGUIDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK1, &CWinCryGUIDlg::OnBnClickedCheck1)
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_COMBO1, &CWinCryGUIDlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON3, &CWinCryGUIDlg::OnBnClickedButton3)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CWinCryGUIDlg::OnCbnSelchangeCombo2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PROGRESS1, &CWinCryGUIDlg::OnNMCustomdrawProgress1)
	ON_MESSAGE(WM_UI_SET_TEXT, &CWinCryGUIDlg::OnUiSetText)
	ON_MESSAGE(WM_UI_ENABLE, &CWinCryGUIDlg::OnUiEnable)
	ON_MESSAGE(WM_UI_START, &CWinCryGUIDlg::OnUiStart)
	ON_MESSAGE(WM_UI_DONE, &CWinCryGUIDlg::OnUiDone)
END_MESSAGE_MAP()


CString printFileSize(ULONGLONG size)
{
	CString str;
	if (size < 1024)
		str.Format(_T("%llu B\r\n"), size);
	else if (size < 1024 * 1024)
		str.Format(_T("%.2f KB\r\n"), size / 1024.0);
	else if (size < 1024 * 1024 * 1024)
		str.Format(_T("%.2f MB\r\n"), size / (1024.0 * 1024.0));
	else
		str.Format(_T("%.2f GB\r\n"), size / (1024.0 * 1024.0 * 1024.0));
	return str;
}


// CWinCryGUIDlg 消息处理程序

BOOL CWinCryGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	for (int i = 0;i < wif.get_cname_num();i++)
		ctype.AddString(CString(wif.get_cname(i).c_str()));
	for (int i = 0;i < wif.get_hname_num();i++)
		htype.AddString(CString(wif.get_hname(i).c_str()));

	ctype.SetCurSel(0);
	htype.SetCurSel(0);
	encrypt.SetCheck(BST_CHECKED);
	CFont mf;
	mf.CreatePointFont(180, L"宋体");
	ver.SetFont(&mf);
	CString versionStr = CString(L"WCGP version: ") + CString(WCGP_VERSION) + CString(L"\r\n") + CString(L"Kernel version: ") + CString(wif.get_version().c_str()) + CString(L"\r\n") + CString(L"Build time: ") + CString(wif.get_buildtime().c_str()) + CString(L"\r\n");
	ver.SetWindowTextW(versionStr);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CWinCryGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWinCryGUIDlg::OnPaint()
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
HCURSOR CWinCryGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

ULONGLONG GetFileSizeNoOpen(const CString& filePath) {
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;

	// 获取文件属性信息
	if (GetFileAttributesEx(filePath, GetFileExInfoStandard, &fileInfo)) {
		// 计算文件大小（高 32 位和低 32 位）
		ULONGLONG fileSize = (static_cast<ULONGLONG>(fileInfo.nFileSizeHigh) << 32) | fileInfo.nFileSizeLow;
		return fileSize;
	}
	else {
		std::cerr << "Failed to get file attributes." << std::endl;
		return 0;
	}
}

// 工作线程参数:在主线程上读取控件后传入,避免跨线程访问 UI 控件
struct CryParams
{
	CWinCryGUIDlg* pDlg;
	CString inputPath;
	CString outputPath;
	CString keyB64;
	CString rbuf;
	int ctypeSel;
	int htypeSel;
	BOOL isEncrypt;
	BOOL isDecrypt;
	BOOL isVerify;
};//output

// 恢复"运行/加密/解密/验证"四个按钮的状态(通过主线程消息)
static void UiEnableAll(CWinCryGUIDlg* dlg, BOOL en)
{
	dlg->PostMessage(WM_UI_ENABLE, UI_EN_RUN, en);
	dlg->PostMessage(WM_UI_ENABLE, UI_EN_ENCRYPT, en);
	dlg->PostMessage(WM_UI_ENABLE, UI_EN_DECRYPT, en);
	dlg->PostMessage(WM_UI_ENABLE, UI_EN_VERIFY, en);
}

UINT MyThreadFunction(LPVOID pParam)
{
	DWORD dwStart = GetTickCount();
	CryParams* prm = (CryParams*)pParam;
	CWinCryGUIDlg* dlg = prm->pDlg;

	default_settings.set_no_echo(true);
	dlg->PostMessage(WM_UI_ENABLE, UI_EN_RUN, FALSE);

	FILE* filein = nullptr;
	FILE* fileout = nullptr;

	if ((prm->ctypeSel == CB_ERR || prm->ctypeSel == 0) && prm->isEncrypt)
	{
		dlg->PostMessage(WM_UI_SET_TEXT, UI_MODE, (LPARAM)L"模式未选择!");
		dlg->PostMessage(WM_UI_SET_TEXT, UI_RES, (LPARAM)L"启动失败");
		UiEnableAll(dlg, TRUE);
		delete prm;
		DWORD dwEnd = GetTickCount();
		dlg->dwElapsed = dwEnd - dwStart;
		return 3;
	}
	else
		dlg->PostMessage(WM_UI_SET_TEXT, UI_MODE, (LPARAM)L"选择成功!");

	if ((prm->htypeSel == CB_ERR || prm->htypeSel == 0) && prm->isEncrypt)
	{
		dlg->PostMessage(WM_UI_SET_TEXT, UI_MODE, (LPARAM)L"模式未选择!");
		dlg->PostMessage(WM_UI_SET_TEXT, UI_RES, (LPARAM)L"启动失败");
		UiEnableAll(dlg, TRUE);
		delete prm;
		DWORD dwEnd = GetTickCount();
		dlg->dwElapsed = dwEnd - dwStart;
		return 3;
	}
	else
		dlg->PostMessage(WM_UI_SET_TEXT, UI_MODE, (LPARAM)L"选择成功!");

	errno_t err = _wfopen_s(&filein, prm->inputPath, L"rb");
	dlg->fileSize = GetFileSizeNoOpen(prm->inputPath);
	if (err == 0)
		dlg->PostMessage(WM_UI_SET_TEXT, UI_INFILE, (LPARAM)L"打开成功!");
	else
	{
		dlg->PostMessage(WM_UI_SET_TEXT, UI_INFILE, (LPARAM)L"打开失败!");
		dlg->PostMessage(WM_UI_SET_TEXT, UI_RES, (LPARAM)L"启动失败");
		UiEnableAll(dlg, TRUE);
		delete prm;
		DWORD dwEnd = GetTickCount();
		dlg->dwElapsed = dwEnd - dwStart;
		return 1;
	}

	if (prm->isVerify)
	{
		fileout = nullptr;
	}
	else
	{
		err = _wfopen_s(&fileout, prm->outputPath, L"wb+");
		if (err == 0)
			dlg->PostMessage(WM_UI_SET_TEXT, UI_OUTFILE, (LPARAM)L"打开成功!");
		else
		{
			dlg->PostMessage(WM_UI_SET_TEXT, UI_OUTFILE, (LPARAM)L"打开失败!");
			dlg->PostMessage(WM_UI_SET_TEXT, UI_RES, (LPARAM)L"启动失败");
			if (filein != nullptr) fclose(filein);
			UiEnableAll(dlg, TRUE);
			delete prm;
			DWORD dwEnd = GetTickCount();
			dlg->dwElapsed = dwEnd - dwStart;
			return 1;
		}
	}

	CStringA cstrA(prm->keyB64); // 转换为 ANSI 字符串
	std::string keyStr = cstrA.GetString(); // 转换为 std::string
	unsigned char key[24] = { 0 };
	if (!checkB64Key((const u8_t*)keyStr.c_str(), key))
	{
		dlg->PostMessage(WM_UI_SET_TEXT, UI_KEYRES, (LPARAM)L"非法密钥!");
		dlg->PostMessage(WM_UI_SET_TEXT, UI_RES, (LPARAM)L"启动失败");
		if (filein != nullptr) fclose(filein);
		if (fileout != nullptr) fclose(fileout);
		UiEnableAll(dlg, TRUE);
		delete prm;
		DWORD dwEnd = GetTickCount();
		dlg->dwElapsed = dwEnd - dwStart;
		return 2;
	}
	dlg->PostMessage(WM_UI_SET_TEXT, UI_KEYRES, (LPARAM)L"合法密钥!");
	CStringA cstrB(prm->rbuf); // 转换为 ANSI 字符串
	std::string rbstr = cstrB.GetString(); // 转换为 std::string

	// runner 仅在消息队列同步点(WM_UI_START)之前由本线程写入,
	// 之后由主线程通过 OnTimer/OnUiDone 访问并负责释放,避免跨线程竞态
	Settings settings(prm->ctypeSel - 1, prm->htypeSel - 1, true);
	dlg->runner = runcrypt_create(filein, fileout, key, settings);
	dlg->PostMessage(WM_UI_START, 0, 0);
	CString resStr = L"运行成功！";
	unsigned short ftype = 0;
	try {
		if (prm->isEncrypt) {
			dlg->runner->execute_encrypt((size_t)dlg->fileSize, (unsigned char*)rbstr.c_str());
			dlg->res.ctype = -1;
			dlg->res.htype = -1;
		}
		else if (prm->isDecrypt) {
			ftype = dlg->runner->execute_decrypt((size_t)dlg->fileSize);
			dlg->res.ctype = ftype & 0xFF;
			dlg->res.htype = (ftype >> 8) & 0xFF;
		}
		else {
			ftype = dlg->runner->execute_verify((size_t)dlg->fileSize);
			dlg->res.ctype = ftype & 0xFF;
			dlg->res.htype = (ftype >> 8) & 0xFF;
		}
	}
	catch (std::string errlog) {
		resStr = CString(errlog.c_str());
		dlg->res.ctype = -1;
		dlg->res.htype = -1;
	}
	dlg->res.resStr = resStr;
	dlg->PostMessage(WM_UI_DONE, 0, 0);
	delete prm;
	DWORD dwEnd = GetTickCount();
	dlg->dwElapsed = dwEnd - dwStart;
	return 0;  // 线程结束
}


void CWinCryGUIDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	encrypt.EnableWindow(false);
	decrypt.EnableWindow(false);
	verify.EnableWindow(false);
	run.EnableWindow(false);

	CryParams* prm = new CryParams;
	prm->pDlg = this;
	filepath.GetWindowText(prm->inputPath);
	outputpath.GetWindowText(prm->outputPath);
	keyinput.GetWindowTextW(prm->keyB64);
	rbuf.GetWindowTextW(prm->rbuf);
	prm->ctypeSel = ctype.GetCurSel();
	prm->htypeSel = htype.GetCurSel();
	prm->isEncrypt = (encrypt.GetState() & BST_CHECKED) != 0;
	prm->isDecrypt = (decrypt.GetState() & BST_CHECKED) != 0;
	prm->isVerify = (verify.GetState() & BST_CHECKED) != 0;

	CWinThread* pThread = AfxBeginThread(MyThreadFunction, (LPVOID)prm);

	if (pThread == nullptr)
	{
		AfxMessageBox(_T("线程创建失败"));
		delete prm;
		run.EnableWindow(true);
		encrypt.EnableWindow(true);
		decrypt.EnableWindow(true);
		verify.EnableWindow(true);
		m_pWorkerThread = nullptr;
		m_taskRunning.store(false);
	}
	else
	{
		// 关闭自动释放:线程退出后由主线程手动 delete,避免悬垂访问
		pThread->m_bAutoDelete = FALSE;
		// 记录线程句柄并标记任务正在运行
		m_pWorkerThread = pThread;
		m_taskRunning.store(true);
	}
	//CDialogEx::OnOK();
}


void CWinCryGUIDlg::OnBnClickedRadio1()
{
	// TODO: 在此添加控件通知处理程序代码
	save.EnableWindow(true);
	rbuf.EnableWindow(ctype.GetCurSel() > 1);
	ctype.EnableWindow(true);
	htype.EnableWindow(true);
}


void CWinCryGUIDlg::OnBnClickedRadio2()
{
	// TODO: 在此添加控件通知处理程序代码
	save.EnableWindow(true);
	rbuf.EnableWindow(false);
	ctype.EnableWindow(false);
	htype.EnableWindow(false);
}


void CWinCryGUIDlg::OnBnClickedRadio3()
{
	// TODO: 在此添加控件通知处理程序代码
	save.EnableWindow(false);
	rbuf.EnableWindow(false);
	ctype.EnableWindow(false);
	htype.EnableWindow(false);
	outputpath.SetWindowTextW(L"");
}


void CWinCryGUIDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	// 创建文件打开对话框
	if (encrypt.GetState() & BST_CHECKED) {
		CFileDialog fileDlg(TRUE, _T(".txt"), nullptr,
			OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
			_T("所有文件 (*.*)|*.*||"));

		// 显示对话框并判断用户是否选择了文件
		if (fileDlg.DoModal() == IDOK)
		{
			CString filePath = fileDlg.GetPathName();  // 获取文件路径
			filepath.SetWindowTextW(filePath);
			outputpath.SetWindowTextW(filePath+L".wenc");
		}
	}
	else {
		CFileDialog fileDlg(TRUE, _T(".txt"), nullptr,
			OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
			_T("Wencry加密文件 (*.wenc)|*.wenc|所有文件 (*.*)|*.*||"));

		// 显示对话框并判断用户是否选择了文件
		if (fileDlg.DoModal() == IDOK)
		{
			CString filePath = fileDlg.GetPathName();  // 获取文件路径
			filepath.SetWindowTextW(filePath);
			outputpath.SetWindowTextW(filePath + L".denc");
		}
	}
}


void CWinCryGUIDlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	UINT nCheck = randomkey.GetState();
	if (nCheck & BST_CHECKED) {
		keyinput.SetReadOnly(true);
		unsigned char* res = getRandomKey();
		keyinput.SetWindowTextW(CString(printkey(res).c_str()));
		delete[] res;
	}
	else {
		keyinput.SetReadOnly(false);
		keyinput.SetSel(0, -1);
		keyinput.ReplaceSel(_T(""));
	}
}



void CWinCryGUIDlg::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
	int nIndex = ctype.GetCurSel();  // 获取当前选择的索引
	if (nIndex != CB_ERR)
	{
		
		default_settings.set_ctype(nIndex - 1);
		rbuf.EnableWindow((encrypt.GetState() & BST_CHECKED) && nIndex > 1);

		CString strText;
		ctype.GetLBText(nIndex, strText);  // 获取选择项的文本
	}
}


void CWinCryGUIDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	if (encrypt.GetState() & BST_CHECKED) {
		CFileDialog saveDlg(FALSE, _T(".wenc"), _T("newfile"),
			OFN_OVERWRITEPROMPT,
			_T("Wencry加密文件 (*.wenc)|*.wenc||"));

		if (saveDlg.DoModal() == IDOK)
		{
			CString savePath = saveDlg.GetPathName();
			outputpath.SetWindowTextW(savePath);
		}
	}
	else if (decrypt.GetState() & BST_CHECKED) {
		CFileDialog saveDlg(FALSE, nullptr, _T("newfile"),
			OFN_OVERWRITEPROMPT,
			_T("所有文件(*.*) | *.* ||"));

		if (saveDlg.DoModal() == IDOK)
		{
			CString savePath = saveDlg.GetPathName();
			outputpath.SetWindowTextW(savePath);
		}
	}
}

void CWinCryGUIDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent != 1)
	{
		CDialogEx::OnTimer(nIDEvent);
		return;
	}

	int new_pos = -1;
	if (runner != nullptr)
		new_pos = runner->get_percentage_gui();
	if (new_pos < 0)
	{
		m_progressCtrl.SetPos(100);
		KillTimer(1);
	}
	else
		m_progressCtrl.SetPos(new_pos);
}

LRESULT CWinCryGUIDlg::OnUiSetText(WPARAM wParam, LPARAM lParam)
{
	const wchar_t* text = reinterpret_cast<const wchar_t*>(lParam);
	switch (wParam)
	{
	case UI_RES:     Res.SetWindowTextW(text);     break;
	case UI_INFILE:  infile.SetWindowTextW(text);  break;
	case UI_OUTFILE: outfile.SetWindowTextW(text); break;
	case UI_KEYRES:  keyres.SetWindowTextW(text);  break;
	}
	return 0;
}

LRESULT CWinCryGUIDlg::OnUiEnable(WPARAM wParam, LPARAM lParam)
{
	BOOL en = (BOOL)lParam;
	switch (wParam)
	{
	case UI_EN_RUN:     run.EnableWindow(en);     break;
	case UI_EN_ENCRYPT: encrypt.EnableWindow(en); break;
	case UI_EN_DECRYPT: decrypt.EnableWindow(en); break;
	case UI_EN_VERIFY:  verify.EnableWindow(en);  break;
	}
	return 0;
}

LRESULT CWinCryGUIDlg::OnUiStart(WPARAM wParam, LPARAM lParam)
{
	SetTimer(1, 20, nullptr);
	return 0;
}



LRESULT CWinCryGUIDlg::OnUiDone(WPARAM wParam, LPARAM lParam)
{
	Res.SetWindowTextW( res.resStr == L"运行成功！" ? L"运行成功！" : L"运行失败");
	result_box.SetWindowTextW(CString(L"运行结果: ") + res.resStr);
	CString strTime;
	strTime.Format(_T(" %lf s"), dwElapsed / (1000.0));
	time_cnt.SetWindowTextW(strTime);
	double speed_num = (dwElapsed > 0) ? (fileSize / (1024.0 * 1024.0)) / (dwElapsed / 1000.0) : 0.0;
	CString strFileSize = printFileSize(fileSize);
	file_size.SetWindowTextW(strFileSize);
	CString strSpeed;
	strSpeed.Format(_T("%.2f MB/s"), speed_num);
	speed.SetWindowTextW(strSpeed);
	m_progressCtrl.SetPos(100);
	KillTimer(1);
	if (runner != nullptr)
	{
		runcrypt_destroy(runner);
		runner = nullptr;
	}
	// 任务结束，等待工作线程完全退出并清理运行状态
	CleanupWorker();
	if (res.ctype >= 0 && res.htype >= 0) {
		ctype.SetCurSel(res.ctype + 1);
		htype.SetCurSel(res.htype + 1);
	}
	run.EnableWindow(TRUE);
	encrypt.EnableWindow(TRUE);
	decrypt.EnableWindow(TRUE);
	verify.EnableWindow(TRUE);
	return 0;
}

// 等待工作线程结束并释放其 CWinThread 对象
// 线程对象已禁用自动删除(m_bAutoDelete=FALSE),必须由主线程在此释放,
// 避免访问已被框架自动销毁的悬垂指针
void CWinCryGUIDlg::CleanupWorker()
{
	if (m_pWorkerThread != nullptr)
	{
		::WaitForSingleObject(m_pWorkerThread->m_hThread, INFINITE);
		delete m_pWorkerThread;
		m_pWorkerThread = nullptr;
	}
	m_taskRunning.store(false);
}


void CWinCryGUIDlg::OnCbnSelchangeCombo2()
{
	// TODO: 在此添加控件通知处理程序代码
	int nIndex = htype.GetCurSel();  // 获取当前选择的索引
	if (nIndex != CB_ERR)
	{
		default_settings.set_htype(nIndex - 1);
		CString strText;
		htype.GetLBText(nIndex, strText);  // 获取选择项的文本
	}
}


void CWinCryGUIDlg::OnNMCustomdrawProgress1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CWinCryGUIDlg::OnCancel()
{
	if (m_taskRunning.load())
	{
		int ret = MessageBox(_T("任务正在运行。选择 '是' 等待任务完成后退出；选择 '否' 取消关闭。"), _T("任务进行中"), MB_YESNO | MB_ICONQUESTION);
		if (ret == IDYES)
		{
			// 等待工作线程结束并释放其 CWinThread 对象
			CleanupWorker();
			// 线程已结束：WM_UI_DONE 可能因对话框即将销毁而不会被派发,
			// 此处手动清理 runner,避免内存泄漏与悬垂指针
			if (runner != nullptr)
			{
				runcrypt_destroy(runner);
				runner = nullptr;
			}
			// 线程完成后继续关闭
		}
		else
		{
			// 用户选择不关闭，忽略关闭请求
			return;
		}
	}
	CDialogEx::OnCancel();
}

void CWinCryGUIDlg::OnDestroy()
{
	// 兜底保护:若对话框在任务运行中被销毁(未经过 OnCancel),
	// 等待工作线程结束,避免其向已销毁窗口发送消息或访问已销毁对象
	if (m_taskRunning.load())
	{
		// 等待工作线程结束并释放其 CWinThread 对象
		CleanupWorker();
		// 线程已结束,清理 runner 与运行状态,防止内存泄漏
		if (runner != nullptr)
		{
			runcrypt_destroy(runner);
			runner = nullptr;
		}
	}
	CDialogEx::OnDestroy();
}
