#include "stdafx.h"
#include "BugTrap\BugTrap.h"
#include "MyBugTrap.h"

static void SetupExceptionHandler()
{
	LOG_TRACER();
	
	BT_InstallSehFilter();

	// 配置信息
    BT_SetAppName(_T("MarketInfo"));
	BT_SetDialogMessage(BTDM_INTRO1, _T("We're so Sorry, program crashed because of our mistakes !"));
	BT_SetDialogMessage(BTDM_INTRO2, _T("本程序会尝试收集仅与此次异常崩溃相关的数据，请将此报告发送给我们以便帮助我们开发体验更好的产品，给您造成的不便之处敬请谅解！"));
    BT_SetSupportEMail(_T("futures_bugreport@tsingfun.com"));
	// BTF_DETAILEDMODE：崩溃时记录dump文件，不设置的话报告中没有dump文件
	// BTF_ATTACHREPORT：点”发送到“可以通过带附件的邮件发送报告
	// BTF_SCREENCAPTURE：错误报告中附带一张崩溃时屏幕截图

	// BTF_EDITMAIL：点”发送到“可以发送邮件到指定地址(没有附件，不可更改目的地址)
	// BTF_LISTPROCESSES：列出崩溃时所有的进程信息(速度较慢)
	// BTF_SHOWADVANCEDUI：崩溃后默认显示详细对话框(不设置的话，先显示简单对话框，有查看详细的按钮)	
	// BTF_DESCRIBEERROR：发送报告之前弹出问题描述对话框，让用户输入错误描述信息
	BT_SetFlags(BTF_DETAILEDMODE | BTF_ATTACHREPORT | BTF_SCREENCAPTURE);
    BT_SetSupportServer(_T("futures.tsingfun.com"), 9999);
    BT_SetSupportURL(_T("http://futures.tsingfun.com"));

	// Add custom log file using default name
	/*INT_PTR iLogHandle = BT_OpenLogFile(NULL, BTLF_TEXT);
	BT_SetLogSizeInEntries(iLogHandle, 100);
	BT_SetLogFlags(iLogHandle, BTLF_SHOWTIMESTAMP);
	BT_SetLogEchoMode(iLogHandle, BTLE_STDERR | BTLE_DBGOUT);

	LPCTSTR lpszFileName = BT_GetLogFileName(iLogHandle);
	BT_AddLogFile(lpszFileName);	
	
	//BT_InsLogEntry(iLogHandle, BTLL_INFO, _T("custom log message"));
	//BT_InsLogEntryF(iLogHandle, BTLL_WARNING, _T("numeric output: %d"), 123);
	
	BT_CloseLogFile(iLogHandle);*/

	// 最新的Log文件附上
	TCHAR szLogFile[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, szLogFile);
	SYSTEMTIME sys;
	GetLocalTime(&sys);

	_stprintf_s(szLogFile, _T("%s\\logs\\%4d%02d%02d.log"), szLogFile, sys.wYear, sys.wMonth, sys.wDay);
	BT_AddLogFile(szLogFile);
	
	//BTTrace trace(_T("c:\\test.log"), BTLF_TEXT);
	//trace.Append(BTLL_INFO, _T("custom log message trace"));
}

// Windows下感知程序崩溃的方法有3个核心的函数，分别如下：
//	SetUnhandledExceptionFilter   (HandleException)确定出现没有控制的异常发生时调用HandleException.
//	_set_invalid_parameter_handler(HandleInvalidParameter)确定出现无效参数调用发生时调用HandleInvalidParameter.
//	_set_purecall_handler         (HandlePureVirtualCall)确定纯虚函数调用发生时调用HandlePureVirtualCall.

// Example: SetUnhandledExceptionFilter(TFUnhandledExceptionFilter); *((int*)0x0) = 0;

LONG WINAPI MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionPointers)
{
	LOG_TRACER();

	SetupExceptionHandler();
	return EXCEPTION_CONTINUE_EXECUTION;
}