//####################################################################
//	Comments:	一些新定义的宏和数据结构
//
//	UpdateLogs:	
//####################################################################
#pragma once

#include <tchar.h>
#include <atlconv.h>
#include <iostream>
#include <string>
using namespace std;

/************************************************************************/
/*与Main.bin进程消息通讯的相关定义*/
///Check Main.bin进程是否正常运行的消息（在查找到Main.bin进程时发送此消息）
#define WM_MAIN_RUN_OK (WM_USER + 0xC000)
///等待main.bin回执的最长时间（单位为：ms）
#define MAX_TIME_WAIT_MAIN_BIN_REPLY 15000
///检测到main.bin在运行，提示用户在等待响应的最长时间
#define MAX_TIP_USER_WAIT_TIME 1000
/************************************************************************/

#define WM_PROGRESS_SET_RANGE	(WM_USER + 101)
#define WM_PROGRESS_SET_POS		(WM_USER + 102)
#define WM_PROGRESS_STEP_IT		(WM_USER + 103)
#define WM_PROGRESS_ERROR		(WM_USER + 104)

#define WM_DOWNLOAD_START		(WM_USER + 105)
#define WM_DOWNLOAD_END			(WM_USER + 106)

#define WM_MONITERFILE			(WM_USER + 334)
#define WM_MONITERPROCESS		(WM_USER + 335)
#define WM_NOTIFYMAINTHREAD		(WM_USER + 336)
#define WM_INTERNET_REQUEST_END (WM_USER + 337)
