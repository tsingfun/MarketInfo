//####################################################################
//	Comments:	透明Static控件
//
//	UpdateLogs:	
//####################################################################
#include "stdafx.h" 
#include "TransparentStatic.h"
#include "MemDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

// CTransparentStatic

IMPLEMENT_DYNAMIC(CTransparentStatic, CStatic)
CTransparentStatic::CTransparentStatic()
{
	m_crText = RGB(0,0,0);
	m_crBk = RGB(255,255,255);
	m_hbkbr = CreateSolidBrush(m_crBk);
}

CTransparentStatic::~CTransparentStatic()
{
	DeleteObject(m_hbkbr);
}

BEGIN_MESSAGE_MAP(CTransparentStatic, CStatic)
	ON_MESSAGE(WM_SETTEXT,OnSetText)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()
// CTransparentStatic message handlers

void CTransparentStatic::OnPaint()
{
	CPaintDC dc(this); // 控件的设备文件
	CMyMemDC memDC(&dc);

	// 取得位置
	CRect client_rect;
	GetClientRect(client_rect);

	// 取得文本
	CString szText;
	GetWindowText(szText);

	// 取得字体，并选入设备文件
	CFont *pFont, *pOldFont;
	pFont = GetFont();
	pOldFont = memDC.SelectObject(pFont);

	CRgn rgn;
	rgn.CreateRectRgn(client_rect.left, client_rect.top, client_rect.right, client_rect.bottom);
	memDC.SelectClipRgn(&rgn);

	// 用透明背景填充设备文件
	memDC.SetBkMode(TRANSPARENT);

	// 显示文字
	//memDC.DrawText(szText, client_rect, 0);
	memDC.TextOut (0, 0, szText);

	// 清除字体
	memDC.SelectObject(pOldFont);
}

LRESULT CTransparentStatic::OnSetText(WPARAM wParam,LPARAM lParam)
{
	LRESULT Result = Default();
	CRect Rect;
	GetWindowRect(&Rect);
	GetParent()->ScreenToClient(&Rect);
	GetParent()->InvalidateRect(&Rect);
	GetParent()->UpdateWindow();
	return Result;
}

HBRUSH CTransparentStatic::CtlColor(CDC* pDC, UINT nCtlColor)
{
	pDC->SetBkMode(TRANSPARENT);
	HBRUSH hb = (HBRUSH)GetStockObject(NULL_BRUSH);
	if(CTLCOLOR_STATIC == nCtlColor)   
	{     
		pDC->SetTextColor(m_crText);
		pDC->SetBkColor(m_crBk);
		hb = m_hbkbr;
	}   

	return hb;
}
