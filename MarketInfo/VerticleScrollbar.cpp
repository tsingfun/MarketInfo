// VerticleScrollbar.cpp : implementation file
//

#include "stdafx.h"
#include "VerticalScrollbar.h"
#include "MemDC.h"
#include "CommonFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVerticleScrollbar

CBitmap * CVerticalScrollbar::m_pUpArrow = NULL;
CBitmap * CVerticalScrollbar::m_pDownArrow = NULL;
CBitmap * CVerticalScrollbar::m_pSpan = NULL;
CBitmap * CVerticalScrollbar::m_pThumb1 = NULL;
CBitmap * CVerticalScrollbar::m_pThumb2 = NULL;

CVerticalScrollbar::CVerticalScrollbar()
{
	m_nCXVScrollNew = 12;
	m_nCYVScrollNew = 12;

	m_nRows = 0;
	m_nPageSize = 0;
	m_nCurrentPosition = 0;

	m_bSignature = false;

	m_vThumbLength = 100;

	m_bMouseDownArrowUp = false;
	m_bMouseDownArrowDown = false;
	m_bDragging = false;
	m_bMouseDownThumb = false;
	m_bPageUp = false;
	m_bPageDown = false;

	m_bSuspendTimer1 = false;
	m_bSuspendTimer2 = false;
	m_bSuspendTimer3 = false;
	m_bSuspendTimer4 = false;

	m_nWidth = 12;
	m_nHeight = 100;

	m_pListWnd = NULL;

	m_pParentWnd = NULL;
	m_nIdEvent = -1;
	m_nElapse = 1000;

	m_bTracking = FALSE;

	// margin-left
	m_nOffsetLeftMargin = 0;
}

CVerticalScrollbar::~CVerticalScrollbar()
{
	
}

BEGIN_MESSAGE_MAP(CVerticalScrollbar, CStatic)
	//{{AFX_MSG_MAP(CSkinVerticleScrollbar)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_MESSAGE( WM_MOUSELEAVE, OnMouseLeave )
	//}}AFX_MSG_MAP
	ON_WM_MOUSEACTIVATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkinVerticleScrollbar message handlers

int CVerticalScrollbar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	InitSkinVerticalScrollbarNew();
	m_nThumbTop = m_nCYVScrollNew;

	return 0;
}

void CVerticalScrollbar::InitSkinVerticalScrollbarNew()
{
	BITMAP Bitmap;
	CBitmap *pBitmap = NULL;

	pBitmap = ::GetBitmap(_T("scroll_bar/DownArrow_window_big.bmp"));
	if (m_pDownArrow != pBitmap)
		m_pDownArrow = pBitmap;

	pBitmap = ::GetBitmap(_T("scroll_bar/UpArrow_window_big.bmp"));
	if (m_pUpArrow != pBitmap)
	{
		m_pUpArrow = pBitmap;
		m_pUpArrow->GetBitmap (&Bitmap);
		m_nCXVScrollNew = Bitmap.bmWidth;
		m_nCYVScrollNew = Bitmap.bmHeight;
	}

	pBitmap = ::GetBitmap(_T("scroll_bar/VSPAN_window_big.bmp"));
	if (m_pSpan != pBitmap)
		m_pSpan = pBitmap;

	pBitmap = ::GetBitmap(_T("scroll_bar/VThumb1_window_big.bmp"));
	if (m_pThumb1 != pBitmap)
		m_pThumb1 = pBitmap;

	pBitmap = ::GetBitmap(_T("scroll_bar/VThumb2_window_big.bmp"));
	if (m_pThumb2 != pBitmap)
		m_pThumb2 = pBitmap;
}

BOOL CVerticalScrollbar::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE; //CStatic::OnEraseBkgnd(pDC);
}

void CVerticalScrollbar::SetTimerEvent (CWnd *pWnd, int nIdEvent, int nElapse)
{
	m_pParentWnd = pWnd;
	m_nIdEvent = nIdEvent;
	m_nElapse = nElapse;
	if (m_pParentWnd && ::IsWindow (m_pParentWnd->GetSafeHwnd ()) && m_nIdEvent >= 0)
		m_pParentWnd->KillTimer (m_nIdEvent);
}

void CVerticalScrollbar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetCapture();
	CRect clientRect;
	GetClientRect(&clientRect);
	
	int nHeight = clientRect.Height() - m_nCYVScrollNew;	

	CRect rectUpArrow(0,0,m_nCXVScrollNew,m_nCYVScrollNew); // 上箭头
	CRect rectDownArrow(0,nHeight,m_nCXVScrollNew,nHeight+m_nCYVScrollNew);//下箭头
	CRect rectThumb(0,m_nThumbTop,m_nCXVScrollNew,m_nThumbTop+m_vThumbLength);//滑块
	CRect rectChanelUp(0,m_nCYVScrollNew, m_nCXVScrollNew, m_nThumbTop)/*上箭头与滑块间*/, 
		rectChanelDown(0, m_nThumbTop + m_vThumbLength, m_nCXVScrollNew,nHeight);//下箭头与滑块间
	//TRACE("top is %d", m_nThumbTop);
	//
	//点中滑块
	if(rectThumb.PtInRect(point))
	{
		m_bMouseDownThumb = true;
		m_1stPointOnMove = point;

	} 
	//
	//点下箭头
	else if(rectDownArrow.PtInRect(point))
	{
		m_bMouseDownArrowDown = true;
		SetTimer(5,250,NULL);
		m_bSuspendTimer1 = false;
	}
	//
	//点上箭头
	else if(rectUpArrow.PtInRect(point))
	{
		m_bMouseDownArrowUp = true;
		SetTimer(5,250,NULL);
		m_bSuspendTimer2 = false;
	}
	//
	//点上箭头与滑块之间
	else if (rectChanelUp.PtInRect(point) )
	{
		m_bPageUp = true;
		SetTimer(5, 250, NULL);
		m_bSuspendTimer3 = false;
		m_1stClickInPageUp = point;

	}
	//
	//点滑块与下箭头之间
	else if (rectChanelDown.PtInRect(point))
	{
		m_bPageDown = true;
		SetTimer(5, 250, NULL);
		m_bSuspendTimer4 = false;
		m_1stClickInPageDown = point;
	}

	CStatic::OnLButtonDown(nFlags, point);
}

void CVerticalScrollbar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	//TRACE("top is %d",m_nThumbTop);
	if (m_pParentWnd && ::IsWindow (m_pParentWnd->GetSafeHwnd ()) && m_nIdEvent >= 0)
	{
		m_pParentWnd->SetTimer (m_nIdEvent, m_nElapse, NULL);
	}

	::PostMessage(m_pListWnd->GetSafeHwnd(), WM_WND_HSCROLLBAR_MOUSEUP, 0, 0);

	KillTimer(1);
	KillTimer(2);
	KillTimer(3);
	KillTimer(4);

	ReleaseCapture();

	CRect clientRect;
	GetClientRect(&clientRect);
	int nHeight = clientRect.Height() - m_nCYVScrollNew;

	CRect rectUpArrow(0,0,m_nCXVScrollNew,m_nCYVScrollNew);//上箭头
	CRect rectDownArrow(0,nHeight,m_nCXVScrollNew,nHeight+m_nCYVScrollNew);//下箭头
	CRect rectThumb(0,m_nThumbTop,m_nCXVScrollNew,m_nThumbTop+m_vThumbLength);//滑块
	CRect rectChanelUp(0,m_nCYVScrollNew, m_nCXVScrollNew, m_nThumbTop),//上箭头与滑块间
		rectChanelDown(0, m_nThumbTop + m_vThumbLength, m_nCXVScrollNew,nHeight);//下箭头与滑块间

	if(rectUpArrow.PtInRect(point))
	{
		if (m_bMouseDownArrowUp)
			::SendMessage(m_pListWnd->GetSafeHwnd(), WM_WND_VSCROLLBAR_CHANGE, V_WND_UP,ONE_WND_ROW);
	}
	else if (rectDownArrow.PtInRect(point))
	{
		if (m_bMouseDownArrowDown)
			::SendMessage(m_pListWnd->GetSafeHwnd(), WM_WND_VSCROLLBAR_CHANGE, V_WND_DOWN, ONE_WND_ROW);
	}

	if (m_bPageDown && rectChanelDown.PtInRect(point))
	{
		::SendMessage(m_pListWnd->GetSafeHwnd(), WM_WND_VSCROLLBAR_CHANGE, V_WND_DOWN, ONE_WND_PAGE);
	}
	else if (m_bPageUp && rectChanelUp.PtInRect(point))
	{
		::SendMessage(m_pListWnd->GetSafeHwnd(), WM_WND_VSCROLLBAR_CHANGE,V_WND_UP, ONE_WND_PAGE);
	}

	m_bMouseDownThumb = false;
	m_bDragging = false;
	m_bMouseDownArrowUp = false;
	m_bMouseDownArrowDown = false;
	m_bPageUp = false;
	m_bPageDown = false;

	CStatic::OnLButtonUp(nFlags, point);
}

static CPoint pt (0, 0);
void CVerticalScrollbar::OnMouseMove(UINT nFlags, CPoint point) 
{

	static CPoint prepoint = point;
	if( prepoint.x != point.x || prepoint.y != point.y)
	{
		prepoint = point;
		if ( point.x > 0 && point.y >0 /*&& point.x < m_rectTip.Width() && point.y < m_rectTip.Height()*/ )
		{
			if( !m_bTracking )
			{
				TRACKMOUSEEVENT tme; 
				tme.cbSize = sizeof( tme );
				tme.hwndTrack = m_hWnd;
				tme.dwFlags = TME_LEAVE;
				m_bTracking = _TrackMouseEvent(&tme); 
				TRACE("CYYBTipWnd::OnMouseMove---TRACKMOUSEEVENT \n");
			}
		}
	}

	if (GetCapture() != this)
	{
		CWnd* pParent = GetParent();
		if (pParent)
		{
			CPoint pos = point;
			ClientToScreen(&pos);
			::ScreenToClient(pParent->m_hWnd, &pos);
			pParent->PostMessage(WM_MOUSEMOVE, nFlags, MAKELONG(pos.x, pos.y));
		}
	}

	CRect clientRect;
	GetClientRect(&clientRect);
	
	int nHeight = clientRect.Height() - m_nCYVScrollNew;
	CRect rectUpArrow(0,0,m_nCXVScrollNew,m_nCYVScrollNew);//上箭头
	CRect rectDownArrow(0,nHeight,m_nCXVScrollNew,nHeight+m_nCYVScrollNew);//下箭头
	CRect rectThumb(0,m_nThumbTop,m_nCXVScrollNew,m_nThumbTop+m_vThumbLength);//滑块
	CRect rectChanelUp(0,m_nCYVScrollNew, m_nCXVScrollNew, m_nThumbTop),//上箭头与滑块间
		rectChanelDown(0, m_nThumbTop + m_vThumbLength, m_nCXVScrollNew,nHeight);//下箭头与滑块间

	if (m_bMouseDownThumb)
		m_bDragging = true;

	if (m_bMouseDownArrowDown)
	{
		if (!rectDownArrow.PtInRect(point))
		{
			m_bSuspendTimer1 = true;
		}
		else
		{
			m_bSuspendTimer1 = false;
		}
	}
	
	if (m_bMouseDownArrowUp)
	{
		if (!rectUpArrow.PtInRect(point))
		{
			m_bSuspendTimer2 = true;
		}
		else 
		{
			m_bSuspendTimer2 = false;
		}
	}

	if (m_bPageUp)
	{
		if (!rectChanelUp.PtInRect(point))
			m_bSuspendTimer3 = true;
		else 
			m_bSuspendTimer3 = false;
	}

	if (m_bPageDown)
	{
		if (!rectChanelDown.PtInRect(point))
			m_bSuspendTimer4 = true;
		else 
			m_bSuspendTimer4 = false;
	}

	if(m_bDragging)
	{
		int offset = point.y - m_1stPointOnMove.y;
		m_1stPointOnMove = point;
		m_nThumbTop += offset;
		
		LimitThumbPosition();
		double nHeight = clientRect.Height()-m_nCYVScrollNew*2;
				
		double dTopIndex = (m_nThumbTop - m_nCYVScrollNew) * 
				(m_nRows - m_nPageSize + 1) / (nHeight - m_vThumbLength);
		int nTopIndex = max(min(max((int)dTopIndex, 0), (m_nRows - m_nPageSize + 1)), 0);

		InvalidateRect(NULL,FALSE);
		::SendMessage(m_pListWnd->GetSafeHwnd(), WM_WND_VSCROLLBAR_CHANGE, V_WND_SETROW, nTopIndex);
	}
	CStatic::OnMouseMove(nFlags, point);
}

void CVerticalScrollbar::OnPaint() 
{
	CPaintDC dc(this);
	
	// 获取重画区域
	CRect rClip,rClient;
	dc.GetClipBox(rClip);
	GetClientRect(rClient);

	// 检查该区域是否需要重画
	if ((rClip & rClient) == CRect(0,0,0,0))
		return;

	//InitSkinVerticalScrollbarNew();
	Draw(&dc);
}

void CVerticalScrollbar::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent)
	{
	case 1: 
		if (!m_bSuspendTimer1) 
			::SendMessage(m_pListWnd->GetSafeHwnd(), WM_WND_VSCROLLBAR_CHANGE, V_WND_DOWN, ONE_WND_ROW); 
		break;
	case 2: 
		if (!m_bSuspendTimer2)
			::SendMessage(m_pListWnd->GetSafeHwnd(), WM_WND_VSCROLLBAR_CHANGE, V_WND_UP, ONE_WND_ROW); 
		break;
	case 3: 
		if (!m_bSuspendTimer3)
			::SendMessage(m_pListWnd->GetSafeHwnd(), WM_WND_VSCROLLBAR_CHANGE, V_WND_UP, ONE_WND_PAGE); 
		break;
	case 4: 
		if (!m_bSuspendTimer4)
		{
			::SendMessage(m_pListWnd->GetSafeHwnd(), WM_WND_VSCROLLBAR_CHANGE, V_WND_DOWN, ONE_WND_PAGE);
		}
		break;
	case 5:
		if (m_bMouseDownArrowDown)
		{
			KillTimer(5);
			SetTimer(1, 50, NULL);
		}
		else if (m_bMouseDownArrowUp)
		{
			KillTimer(5);
			SetTimer(2, 50, NULL);
		}
		else if (m_bPageUp)
		{
			KillTimer(5);
			SetTimer(3, 50, NULL);
		}
		else if (m_bPageDown)
		{
			KillTimer(5);
			SetTimer(4, 50, NULL);
		}
		break;
	}

	CStatic::OnTimer(nIDEvent);
}


void CVerticalScrollbar::OnSize(UINT nType, int cx, int cy)
{
	CStatic::OnSize(nType, cx, cy);
	
	UpdateThumbPosition(m_nCurrentPosition);
}

void CVerticalScrollbar::UpdateThumbPosition(int currentPosition)
{
	CRect clientRect;
	GetClientRect(&clientRect);
	
	m_nCurrentPosition = currentPosition;
	
	int nHeight = clientRect.Height() - m_nCYVScrollNew;

	if (!m_bDragging)
	{
		double nHeight = (clientRect.Height()-(m_nCYVScrollNew*2)-m_vThumbLength);
		double dbThumbInterval = 0;
		if(m_nRows >= m_nPageSize)
			dbThumbInterval = nHeight / (m_nRows - m_nPageSize + 1);
		else
			dbThumbInterval =0;

		//double dbThumbInterval = nHeight/m_nRows;

		double nNewdbValue = (dbThumbInterval * (double)currentPosition);
		int nNewValue = (int)nNewdbValue;

		m_nThumbTop = m_nCYVScrollNew+nNewValue;

		if (currentPosition != 0 && currentPosition >= m_nRows - m_nPageSize)
			m_nThumbTop = clientRect.Height()-m_nCYVScrollNew-m_vThumbLength;
	}

	CRect rectChanelUp(0,m_nCYVScrollNew, m_nCXVScrollNew, m_nThumbTop),//上箭头与滑块间
		rectChanelDown(0, m_nThumbTop + m_vThumbLength, m_nCXVScrollNew,nHeight);//下箭头与滑块间

	if (m_bPageUp)
	{
		if (!rectChanelUp.PtInRect(m_1stClickInPageUp))
			m_bSuspendTimer3 = true;
	}
	if (m_bPageDown)
	{
		if (!rectChanelDown.PtInRect(m_1stClickInPageDown))
			m_bSuspendTimer4 = true;
	}

	LimitThumbPosition();
	InvalidateRect(NULL,FALSE);
}

void CVerticalScrollbar::Draw(CDC* pDC)
{
	//pList->PositionScrollBars();
	if (m_pListWnd != NULL)
	{
		if (!m_pUpArrow || !m_pDownArrow || !m_pSpan || !m_pThumb1 || !m_pThumb2)
			return;

		CRect clientRect;
		GetClientRect(&clientRect);
		CMyMemDC memDC(pDC, clientRect);
		memDC.FillSolidRect(&clientRect,  RGB(255,0,0));
		CDC bitmapDC;
		bitmapDC.CreateCompatibleDC(pDC);
		
		int bitmapWidth;
		int bitmapHeight;

		if (clientRect.Width() != 0)
		{
		//	CBitmap* pOldBitmap = bitmapDC.SelectObject(m_pcbUpArrow[m_nMode]);
			CBitmap* pOldBitmap = bitmapDC.SelectObject(m_pUpArrow);
			memDC.BitBlt(clientRect.left,clientRect.top,m_nCXVScrollNew,m_nCYVScrollNew,&bitmapDC,0,0,SRCCOPY);
			bitmapDC.SelectObject(pOldBitmap);
			pOldBitmap = NULL;
			
			BITMAP BmpProp ={0};   

			//draw the background (span)

		//	pOldBitmap = bitmapDC.SelectObject(m_pcbSpan[m_nMode]);
			pOldBitmap = bitmapDC.SelectObject(m_pSpan);
			int nHeight = clientRect.Height() - (m_nCYVScrollNew*2);
		//	m_pcbSpan[m_nMode]->GetBitmap(&BmpProp);
			m_pSpan->GetBitmap(&BmpProp);

			//bitmap size
			bitmapWidth = BmpProp.bmWidth;
			bitmapHeight = BmpProp.bmHeight;

			// stretch bitmap
			memDC.StretchBlt(clientRect.left, clientRect.top+m_nCYVScrollNew, m_nCXVScrollNew,nHeight,&bitmapDC, 0,0, bitmapWidth, bitmapHeight, SRCCOPY);

			bitmapDC.SelectObject(pOldBitmap);
			pOldBitmap = NULL;
			
			//draw the down arrow of the scrollbar
			if (m_bSignature)
			{
			//	pOldBitmap = bitmapDC.SelectObject(m_pcbDownArrow1[m_nMode]);
			//	pOldBitmap = bitmapDC.SelectObject(m_pDownArrow1);
			}
			else
			{
			//	pOldBitmap = bitmapDC.SelectObject(m_pcbDownArrow[m_nMode]);
				pOldBitmap = bitmapDC.SelectObject(m_pDownArrow);
			}

			memDC.BitBlt(clientRect.left,m_nCYVScrollNew+nHeight,m_nCXVScrollNew,m_nCYVScrollNew,&bitmapDC,0,0,SRCCOPY);
			bitmapDC.SelectObject(pOldBitmap);
			pOldBitmap = NULL;

			//draw the thumb control
	
		//	pOldBitmap = bitmapDC.SelectObject(m_pcbThumb1[m_nMode]);
		//	m_pcbThumb1[m_nMode]->GetBitmap(&BmpProp);
			pOldBitmap = bitmapDC.SelectObject(m_pThumb1);
			m_pThumb1->GetBitmap(&BmpProp);

			bitmapWidth = BmpProp.bmWidth;
			bitmapHeight = BmpProp.bmHeight;

			memDC.BitBlt(clientRect.left + m_nOffsetLeftMargin,clientRect.top+m_nThumbTop,m_nCYVScrollNew,bitmapHeight,&bitmapDC,0,0,SRCCOPY);
			bitmapDC.SelectObject(pOldBitmap);
			pOldBitmap = NULL;

			int oldBitmapWidth = bitmapWidth;
			int oldBitmapHeight = bitmapHeight;

		//	pOldBitmap = bitmapDC.SelectObject(m_pcbThumb2[m_nMode]);
		//	m_pcbThumb2[m_nMode]->GetBitmap(&BmpProp);
			pOldBitmap = bitmapDC.SelectObject(m_pThumb2);
			m_pThumb2->GetBitmap(&BmpProp);
	
			bitmapWidth = BmpProp.bmWidth;
			bitmapHeight = BmpProp.bmHeight;
			
			memDC.StretchBlt(clientRect.left + m_nOffsetLeftMargin,clientRect.top+m_nThumbTop + oldBitmapHeight,bitmapWidth,m_vThumbLength - oldBitmapHeight ,&bitmapDC,0,0,m_nCYVScrollNew,bitmapHeight,SRCCOPY);
			bitmapDC.SelectObject(pOldBitmap);
			pOldBitmap = NULL;
		}// end of if clientRect

		bitmapDC.DeleteDC();
	}// end of if m_pListWnd
}

void CVerticalScrollbar::LimitThumbPosition()
{
	CRect clientRect;
	GetClientRect(&clientRect);

	if(m_nThumbTop+m_vThumbLength > (clientRect.Height()-m_nCYVScrollNew))
	{
		m_nThumbTop = clientRect.Height()-m_nCYVScrollNew-m_vThumbLength;
	}

	if(m_nThumbTop < (clientRect.top+m_nCYVScrollNew))
	{
		m_nThumbTop = clientRect.top+m_nCYVScrollNew;
	}
}

void CVerticalScrollbar::SetWidth(int width = 12)
{
	m_nWidth = width;
}

int CVerticalScrollbar::GetWidth()
{
	return m_nWidth;
}

void CVerticalScrollbar::SetHeight(int height = 100)
{
	m_nHeight = height;
}

int CVerticalScrollbar::GetHeight()
{
	return m_nHeight;
}

void CVerticalScrollbar::SetThumbLength(int length)
{
	m_vThumbLength = length;

	if(m_vThumbLength<10)
		m_vThumbLength = 10;
	InvalidateRect(NULL,FALSE);
}

void CVerticalScrollbar::SetRowNumber(int rows)
{
	m_nRows = rows;
	
}

void CVerticalScrollbar::SetPageSize(int pageSize)
{
	m_nPageSize = pageSize;
}

int CVerticalScrollbar::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return CStatic::OnMouseActivate(pDesktopWnd, nHitTest, message);
}


LRESULT  CVerticalScrollbar::OnMouseLeave(WPARAM   wParam,   LPARAM   lParam)
{
	m_bTracking = FALSE;

	::SendMessage(m_pListWnd->GetSafeHwnd(), WM_SCROLL_LEAVE, 0 , 0);
	return 0L;
}