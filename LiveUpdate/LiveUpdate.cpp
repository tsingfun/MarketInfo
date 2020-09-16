
// LiveUpdate.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "LiveUpdate.h"
#include "LiveUpdateDlg.h"
#include "Logger.h"
#include "FileVersion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DOWNLOAD_URL		_T("http://futures.tsingfun.com/update.ini")
#define INI_FILENAME		_T("update.ini")
#define MAIN_EXE_FILENAME	_T("MarketInfo.exe")

// CLiveUpdateApp

BEGIN_MESSAGE_MAP(CLiveUpdateApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CLiveUpdateApp 构造

CLiveUpdateApp::CLiveUpdateApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CLiveUpdateApp 对象

CLiveUpdateApp theApp;


// CLiveUpdateApp 初始化

BOOL CLiveUpdateApp::InitInstance()
{
	CWinApp::InitInstance();


	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

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

	// 检查是否需要更新
	if (!CheckUpdate())
		return FALSE;

	CLiveUpdateDlg dlg;
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
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
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

bool CLiveUpdateApp::CheckUpdate()
{
	LOG_INFO(_T("开始下载%s"), INI_FILENAME);
	TCHAR szPath[_MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, szPath);

	TCHAR szFile[_MAX_PATH] = {0};
	_stprintf_s(szFile, _MAX_PATH, _T("%s\\%s"), szPath, INI_FILENAME);
	// 如果本地文件存在，先删除
	if (-1 != _taccess(szFile,NULL))
		::DeleteFile(szFile);

	CString strErrMsg;
	unsigned long nFileSize = 0;
	
	CComEngine *pComEngine = CComEngine::GetComEngine();
	if (!pComEngine)
	{
		LOG_ERROR(_T("GetComEngine失败，退出"));
		return false;
	}

	// 用同步方式下载
	if (!pComEngine->Download(NULL, NULL, DOWNLOAD_URL, 
		INI_FILENAME, strErrMsg, nFileSize))
	{
		LOG_ERROR(strErrMsg);
		// 调试输出
		OutputDebugString(strErrMsg);

		return false;
	}

	// 下载完成，判断版本号
	if (GetLatestVersion().Compare(GetCurrentVersion()) > 0)
		return true;
	else 
		return false;

	return true;
}

CString CLiveUpdateApp::GetCurrentVersion()
{
	CString version;
	CFileVersion fversion;
	TCHAR szPath[_MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, szPath);
	TCHAR szFile[_MAX_PATH] = {0};
	_stprintf_s(szFile, _MAX_PATH, _T("%s\\%s"), szPath, MAIN_EXE_FILENAME);
	if (fversion.Open(szFile))
		version = fversion.GetFileVersion();
	else
		LOG_ERROR(_T("获取主程序：%s 版本号失败！"), szFile);

	return version;
}

CString CLiveUpdateApp::GetLatestVersion()
{
	TCHAR szPath[_MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, szPath);
	TCHAR szFile[_MAX_PATH] = {0};
	_stprintf_s(szFile, _MAX_PATH, _T("%s\\%s"), szPath, INI_FILENAME);

	CString version;
	::GetPrivateProfileString(_T("update"), _T("version"), NULL, version.GetBuffer(MAX_PATH), MAX_PATH, szFile);

	return version;
}