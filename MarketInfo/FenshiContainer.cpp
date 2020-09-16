// FenshiContainer.cpp : 实现文件
//

#include "stdafx.h"
#include "MarketInfo.h"
#include "FenshiContainer.h"
#include "MemDC.h"
#include "LightCanvas.hpp"

// CFenshiContainer

IMPLEMENT_DYNAMIC(CFenshiContainer, CContainer)

CFenshiContainer::CFenshiContainer()
{

}

CFenshiContainer::~CFenshiContainer()
{
}


BEGIN_MESSAGE_MAP(CFenshiContainer, CContainer)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CFenshiContainer 消息处理程序

void CFenshiContainer::OnPaint()
{
	CPaintDC dc(this);
	CMyMemDC MemDC(&dc);
	CLightCanvas canvas(MemDC);
	canvas.SetBkMode();

	CRect rect;
	GetClientRect(&rect);

	// 填充黑色背景
	canvas.DrawRect(rect, COLOR_BLACK_BG, COLOR_BLACK_BG);
	DrawBorder(&MemDC, BORDER_RIGHT | BORDER_BOTTOM);

	DrawTitle(&MemDC, _T("分时成交"));

	// 填充标题背景缺口
	canvas.DrawLine(0, 0, COLOR_TITLE_BG, 1, 0, TITLE_HEIGHT);
	canvas.DrawLine(0, 0, COLOR_GRAY_BORDER, 1, 1, 1);
	canvas.DrawLine(0, TITLE_HEIGHT - 1, COLOR_GRAY_BORDER, 1, 1, TITLE_HEIGHT - 1);
}
