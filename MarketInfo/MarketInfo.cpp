
// MarketInfo.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "MarketInfo.h"
//#include "MarketInfoDlg.h"
#include "MainDlg.h"
#include "VerticalScrollbar.h"
#include "CommonFunc.h"
#include "MyBugTrap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define LIVE_UPDATE_FILENAME	_T("LiveUpdate.exe")
#define COMM_LIB_FILENAME		_T("commlib.dll")

// CMarketInfoApp

BEGIN_MESSAGE_MAP(CMarketInfoApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMarketInfoApp 构造

CMarketInfoApp::CMarketInfoApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CMarketInfoApp 对象

CMarketInfoApp theApp;


// CMarketInfoApp 初始化

BOOL CMarketInfoApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	// 设置日志级别
#ifndef _DEBUG
	SetLoggerLevel(LEVEL_ERROR);
#endif

	// 开启程序崩溃感知
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

	// 开启一个线程启动升级程序
	DWORD dwThread;
	HANDLE hThread = ::CreateThread(NULL, 0, CheckUpdate, 0, 0, &dwThread);
	if (NULL == hThread)
		TRACE("创建新的线程出错！\n");

	//CMarketInfoDlg dlg;
	CMainDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

static DWORD dwLiveUpdateTrdID = 0;

int CMarketInfoApp::ExitInstance()
{
	FREE_P(CVerticalScrollbar::m_pUpArrow)
	FREE_P(CVerticalScrollbar::m_pDownArrow)
	FREE_P(CVerticalScrollbar::m_pSpan)
	FREE_P(CVerticalScrollbar::m_pThumb1)
	FREE_P(CVerticalScrollbar::m_pThumb2)

	// 释放行情api指针
	CMainDlg::m_pApi->Release();
	FREE_P(CMainDlg::m_pUserSpi);

	// 关闭更新对话框
	::PostThreadMessage(dwLiveUpdateTrdID, WM_QUIT, 0, 0);

	return TRUE;
}

DWORD WINAPI CheckUpdate( LPVOID lparam )
{
	//::Sleep(1000);

	TCHAR szPath[_MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, szPath);

	TCHAR szLiveUpdateFile[_MAX_PATH] = {0};
	_stprintf_s(szLiveUpdateFile, _MAX_PATH, _T("%s\\%s"), szPath, LIVE_UPDATE_FILENAME);
	TCHAR szCommLibFile[_MAX_PATH] = {0};
	_stprintf_s(szCommLibFile, _MAX_PATH, _T("%s\\%s"), szPath, COMM_LIB_FILENAME);

	
	// 防止LiveUpdate.exe无法更新自身程序，将程序拷贝到tmp目录
	CString tmpPath;
	ExpandEnvironmentStrings(_T("%temp%"), tmpPath.GetBuffer(MAX_PATH), MAX_PATH);
	//CreateDirectory(tmpPath, NULL);

	CString tmpLiveUpdate, tmpCommLib;
	tmpLiveUpdate.Format(_T("%s\\%s"), tmpPath, LIVE_UPDATE_FILENAME);
	::CopyFile(szLiveUpdateFile, tmpLiveUpdate, FALSE);
	tmpCommLib.Format(_T("%s\\%s"), tmpPath, COMM_LIB_FILENAME);
	::CopyFile(szCommLibFile, tmpCommLib, FALSE);

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset( &si, 0, sizeof( si ) );
	si.cb = sizeof( si );
	si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW;

	BOOL fRet = CreateProcess( tmpLiveUpdate, NULL, NULL, FALSE, NULL, NULL, NULL, szPath, &si, &pi );

	if (!fRet)
		LOG_ERROR( _T("启动更新程序:%s失败！"), tmpLiveUpdate );

	dwLiveUpdateTrdID = pi.dwThreadId;

	return 0;
}