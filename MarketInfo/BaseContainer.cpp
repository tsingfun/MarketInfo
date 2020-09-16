// Container.cpp : 实现文件
//

#include "stdafx.h"
#include "MarketInfo.h"
#include "BaseContainer.h"
#include "CommonFunc.h"
#include "MemDC.h"
#include "LightCanvas.hpp"

// CContainer

IMPLEMENT_DYNAMIC(CContainer, CWnd)

CContainer::CContainer()
{
	// 注册窗口
	CString strWndClass = AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW), 
		GetSysColorBrush(COLOR_WINDOW), NULL);

	// 创建窗口
	CreateEx(NULL, strWndClass, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, AfxGetMainWnd()->m_hWnd, NULL);

	m_pfTitle = ::GetFont(_T("宋体"), 10);
	m_pfSystemUnderLine = ::GetFont(_T("system"), 9, FW_NORMAL, 1);
}

CContainer::~CContainer()
{
	FREE_P(m_pfTitle)
	FREE_P(m_pfSystemUnderLine)
}

BEGIN_MESSAGE_MAP(CContainer, CWnd)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CContainer 消息处理程序
int CContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

BOOL CContainer::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return TRUE;
}

void CContainer::DrawTitle( CDC *pDC, CString strTitle )
{
	CLightCanvas canvas(pDC);
	canvas.SetBkMode();

	CRect rect;
	GetClientRect(&rect);

	// 标题背景
	CRect rectTitle;
	rectTitle.SetRect(0, 0, rect.Width(), TITLE_HEIGHT);
	canvas.DrawRect(rectTitle, COLOR_GRAY_BORDER, COLOR_TITLE_BG);

	// 标题
	CSize titleSize = pDC->GetTextExtent(strTitle);
	CRect tmpRect = rect;
	tmpRect.DeflateRect( (rect.Width() - titleSize.cx) / 2, 6, 0, 0);
	canvas.SetTextColor(RGB(255, 255, 255));
	canvas.DrawText(strTitle, &tmpRect, DT_LEFT | DT_TOP | DT_SINGLELINE, m_pfTitle);
}

void CContainer::DrawBorder(CDC *pDC, DWORD dwBorderType)
{
	CLightCanvas canvas(pDC);
	canvas.SetBkMode();

	CRect rect;
	GetClientRect(&rect);

	if (dwBorderType & BORDER_TOP)
		canvas.DrawLine(0, 0, COLOR_GRAY_BORDER, 1, rect.right, 0);

	if (dwBorderType & BORDER_LEFT)
		canvas.DrawLine(0, 0, COLOR_GRAY_BORDER, 1, 0, rect.bottom);

	if (dwBorderType & BORDER_BOTTOM)
		canvas.DrawLine(0, rect.bottom - 1, COLOR_GRAY_BORDER, 1, rect.right, rect.bottom - 1);

	if (dwBorderType & BORDER_RIGHT)
		canvas.DrawLine(rect.right - 1, 0, COLOR_GRAY_BORDER, 1, rect.right - 1, rect.bottom);
}

void CContainer::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMERID_SPLASH:
		{
			for (int i = 0; i < (int)m_vecClfSplash.size(); i++)
			{
				if (m_vecClfSplash[i] == COLOR_SPLASH_4)
					m_vecClfSplash[i] = COLOR_SPLASH_3;
				else if (m_vecClfSplash[i] == COLOR_SPLASH_3)
					m_vecClfSplash[i] = COLOR_SPLASH_2;
				else if (m_vecClfSplash[i] == COLOR_SPLASH_2)
					m_vecClfSplash[i] = COLOR_SPLASH_1;
			}

			Invalidate();
		}
		break;

	default:
		break;
	}

	__super::OnTimer(nIDEvent);
}