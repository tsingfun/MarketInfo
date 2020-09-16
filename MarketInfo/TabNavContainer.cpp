// TabNavContainer.cpp : 实现文件
//

#include "stdafx.h"
#include "MarketInfo.h"
#include "TabNavContainer.h"
#include "MemDC.h"
#include "LightCanvas.hpp"

// CTabNavContainer

IMPLEMENT_DYNAMIC(CTabNavContainer, CContainer)

CTabNavContainer::CTabNavContainer()
{

}

CTabNavContainer::~CTabNavContainer()
{
}


BEGIN_MESSAGE_MAP(CTabNavContainer, CContainer)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CTabNavContainer 消息处理程序

void CTabNavContainer::OnPaint()
{
	CPaintDC dc(this);
	CMyMemDC MemDC(&dc);
	CLightCanvas canvas(MemDC);
	canvas.SetBkMode();

	CRect rect;
	GetClientRect(&rect);

	// 填充黑色背景
	canvas.DrawRect(rect, COLOR_BLACK_BG, COLOR_BLACK_BG);
	DrawBorder(&MemDC, BORDER_BOTTOM);
}
