#pragma once


// 设置焦点
#define WM_USER_SETFOCUS_MSG					(WM_USER+1)
// 键盘宝输入框keydown
#define WM_JPB_KEYDOWN							(WM_USER+2)
// 键盘宝切换品种
#define WM_JPB_EXCHANGE							(WM_USER+2)
// 键盘宝对话框mousewheel
#define WM_JPB_MOUSEWHEEL						(WM_USER+3)


// 滚动条相关
#define WM_WND_VSCROLLBAR_CHANGE				(WM_USER+143)				// 垂直滚动条位置变化
#define WM_WND_HSCROLLBAR_CHANGE				(WM_USER+144)				// 水平滚动条位置变化
#define WM_SCROLL_LEAVE							(WM_USER+145)				
#define WM_WND_HSCROLLBAR_MOUSEUP               (WM_USER+156)

#define H_WND_LEFT 1
#define H_WND_RIGHT 2
#define H_WND_SETCOL 3
#define V_WND_UP 4
#define V_WND_DOWN 5
#define V_WND_SETROW 6

#define ONE_WND_ROW 1
#define ONE_WND_PAGE 2