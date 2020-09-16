// ZhubiContainer.cpp : 实现文件
//

#include "stdafx.h"
#include "MarketInfo.h"
#include "ZhubiContainer.h"
#include "MemDC.h"
#include "LightCanvas.hpp"
#include "CommonFunc.h"
#include "CommUtil.h"
#include "MainDlg.h"

// CZhubiContainer

IMPLEMENT_DYNAMIC(CZhubiContainer, CContainer)

CZhubiContainer::CZhubiContainer():m_bScrollBottom(true)
{
	m_rScrollBar.SetRectEmpty();
	m_bDrawScrollBar = false;
	m_pVerticleScrollbar = NULL;
	m_nMaxLine = 0;

	InitScrollBar();

	SetTimer(TIMERID_SPLASH, INTERVAL_SPLASH, NULL);
}

CZhubiContainer::~CZhubiContainer()
{
	FREE_P(m_pVerticleScrollbar)
}

BEGIN_MESSAGE_MAP(CZhubiContainer, CContainer)
	ON_WM_PAINT()
	ON_WM_MOUSEWHEEL()
	ON_MESSAGE(WM_WND_VSCROLLBAR_CHANGE, OnVScrollBarChange)
END_MESSAGE_MAP()


// CZhubiContainer 消息处理程序

BOOL CZhubiContainer::InitScrollBar()
{
	if (NULL == m_pVerticleScrollbar)
	{
		m_pVerticleScrollbar = new CVerticalScrollbar();
		if (NULL == m_pVerticleScrollbar)
			return FALSE;

		m_pVerticleScrollbar->Create(NULL, WS_CHILD|WS_VISIBLE|SS_LEFT|SS_NOTIFY|WS_GROUP, CRect (0, 0, 0, 0), this);
		m_pVerticleScrollbar->m_pListWnd = this;
	}

	return TRUE;
}

void CZhubiContainer::InitScrollBarPosition()
{
	if (NULL == m_pVerticleScrollbar)	
		return;

	CRect rect;
	GetClientRect(&rect);
	// 计算最大行数
	m_nMaxLine = (rect.Height() - TITLE_HEIGHT) / LIST_ITEM_HEIGHT;

	// 判断滚动条是否处于底部(处于底部，刷新自动滚动；不处于底部，固定显示)
	if (m_bDrawScrollBar && (int)m_vecDataList.size() > m_nMaxLine && m_nTopIndex < (int)m_vecDataList.size() - m_nMaxLine - 1)
		m_bScrollBottom = false;
	else
		m_bScrollBottom = true;

	// 是否显示滚动条
	if ((int)m_vecDataList.size() > m_nMaxLine)
		m_bDrawScrollBar = true;

	if (m_bScrollBottom)
		m_nTopIndex = ((int)m_vecDataList.size() > m_nMaxLine) ? ((int)m_vecDataList.size() - m_nMaxLine) : 0;

	int nLeft = 0, nTop = 0, nRight = 0, nBottom = 0;
	if (m_bDrawScrollBar)
	{
		UINT uWindowPosType = SWP_SHOWWINDOW;
		nLeft = rect.right - SCROLLBAR_WIDTH - 2;
		nRight = SCROLLBAR_WIDTH;
		nTop = rect.top + TITLE_HEIGHT;
		nBottom = rect.Height() - TITLE_HEIGHT - 1;

		m_pVerticleScrollbar->SetWindowPos(&wndTop, nLeft, nTop, nRight, nBottom, uWindowPosType);
		m_rScrollBar.SetRect(nLeft, nTop, nLeft+nRight, nTop+nBottom);
	}
	else
	{
		UINT uWindowPosType = SWP_HIDEWINDOW;
		m_pVerticleScrollbar->SetWidth (0);
		m_pVerticleScrollbar->SetThumbLength (0);
		m_pVerticleScrollbar->SetPageSize(0);
		m_pVerticleScrollbar->SetRowNumber(0);

		m_pVerticleScrollbar->SetWindowPos (&wndTop, nLeft, nTop, nRight, nBottom, uWindowPosType);
		m_rScrollBar.SetRect(0, 0, 0, 0);
	}
}

void CZhubiContainer::OnPaint()
{
	CPaintDC dc(this);
	// 排除滚动条，不刷新
	if (m_bDrawScrollBar)
		ExcludeClipRect(dc, m_rScrollBar.left, m_rScrollBar.top, m_rScrollBar.right, m_rScrollBar.bottom);

	CMyMemDC MemDC(&dc);
	CLightCanvas canvas(MemDC);
	canvas.SetBkMode();

	CRect rect;
	GetClientRect(&rect);

	// 填充黑色背景
	canvas.DrawRect(rect, COLOR_BLACK_BG, COLOR_BLACK_BG);
	DrawBorder(&MemDC, BORDER_LEFT | BORDER_RIGHT | BORDER_BOTTOM);

	DrawTitle(&MemDC, _T("逐笔成交"));

	// 画列表
	DrawList(&MemDC);
}

BOOL CZhubiContainer::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_bDrawScrollBar)
	{
		if (zDelta < 0)
		{
			ScrollDown();
		}
		else
		{
			ScrollUp();
		}
	}

	return __super::OnMouseWheel(nFlags, zDelta, pt);
}

BOOL CZhubiContainer::ScrollUp()
{
	if (NULL == m_pVerticleScrollbar)
		return FALSE;
	int nCount = static_cast<int>(m_vecDataList.size());
	if (nCount <= 0)
		return FALSE;

	m_nTopIndex = max(m_nTopIndex--, 0);

	Invalidate();
	m_pVerticleScrollbar->UpdateThumbPosition(m_nTopIndex);

	return TRUE;
}

BOOL CZhubiContainer::ScrollDown()
{
	if (NULL == m_pVerticleScrollbar)
		return FALSE;
	int nCount = static_cast<int>(m_vecDataList.size());
	if (nCount <= 0)
		return FALSE;

	if ((m_nTopIndex + m_nMaxLine - 1) >= (nCount - 1))
		return TRUE;

	m_nTopIndex ++;

	Invalidate();
	m_pVerticleScrollbar->UpdateThumbPosition(m_nTopIndex);

	return TRUE;
}

BOOL CZhubiContainer::PageUp()
{
	if (NULL == m_pVerticleScrollbar)
		return FALSE;
	int nCount = static_cast<int>(m_vecDataList.size());
	if (nCount <= 0)
		return FALSE;

	m_nTopIndex = max(m_nTopIndex - m_nMaxLine + 1, 0);

	Invalidate();
	m_pVerticleScrollbar->UpdateThumbPosition(m_nTopIndex);

	return TRUE;
}

BOOL CZhubiContainer::PageDown()
{
	if (NULL == m_pVerticleScrollbar)
		return FALSE;
	int nCount = static_cast<int>(m_vecDataList.size());
	if (nCount <= 0)
		return FALSE;

	int nMaxIndex = max ((int)nCount - 1, 0);
	m_nTopIndex = min (m_nTopIndex + m_nMaxLine - 1, nMaxIndex);
	if (m_nTopIndex + m_nMaxLine > nCount)
	{
		m_nTopIndex = nCount - m_nMaxLine;
	}

	Invalidate();
	m_pVerticleScrollbar->UpdateThumbPosition(m_nTopIndex);

	return TRUE;
}

BOOL CZhubiContainer::ScrollRowTo(int nRow)
{
	if (NULL == m_pVerticleScrollbar)
		return FALSE;
	int nCount = static_cast<int>(m_vecDataList.size());
	if (nCount <= 0)
		return FALSE;

	m_nTopIndex = nRow;
	if (nCount - m_nTopIndex <= m_nMaxLine - 1)
	{
		m_nTopIndex = nCount - m_nMaxLine;
	}

	Invalidate();
	m_pVerticleScrollbar->UpdateThumbPosition(m_nTopIndex);

	return TRUE;
}

LRESULT CZhubiContainer::OnVScrollBarChange(WPARAM wParam, LPARAM lParam)
{
	if (wParam == V_WND_UP)
	{
		if (lParam == ONE_WND_ROW)
		{
			ScrollUp ();
		}
		else
		{
			PageUp ();
		}
	}
	else if (wParam == V_WND_DOWN)
	{
		if (lParam == ONE_WND_ROW)
		{
			ScrollDown ();
		}
		else
		{	
			PageDown ();
		}
	}
	else if (wParam == V_WND_SETROW)
	{	
		ScrollRowTo (static_cast<int>(lParam));
	}

	return TRUE;
}


void CZhubiContainer::DrawList( CDC *pDC )
{
	CLightCanvas canvas(pDC);
	canvas.SetBkMode();

	CRect rect;
	GetClientRect(&rect);
	int nIndex = 0;

	for(size_t i = m_nTopIndex; i < m_vecDataList.size(); i++)
	{
		int w = (rect.Width() - SCROLLBAR_WIDTH - ITEM_TIME_WIDTH) / 2 - 2;

		int top = TITLE_HEIGHT + LIST_ITEM_HEIGHT * nIndex;
		int bottom = top + LIST_ITEM_HEIGHT;
	
		CRect timeRect  (0, top, ITEM_TIME_WIDTH, bottom);
		CRect secondRect(0, top, ITEM_TIME_WIDTH + w, bottom);
		CRect thirdRect (0, top, ITEM_TIME_WIDTH + w * 2, bottom);

		// 画渐变蓝色背景
		if (i - m_nTopIndex < m_vecClfSplash.size())
			canvas.DrawRect(CRect(1, top, rect.Width()-2, bottom),
				m_vecClfSplash[i - m_nTopIndex], m_vecClfSplash[i - m_nTopIndex]);

		// 绘制时间字符串
		CString strTime(m_vecDataList[i].Time);
		canvas.SetTextColor(COLOR_WHITE_TEXT);
		canvas.DrawText(strTime, &timeRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		// 绘制成交价
		canvas.SetTextColor(COLOR_GREEN_TEXT);
		CString zhengshu, xiaoshu;
		zhengshu.Format(_T("%d"), (int)m_vecDataList[i].Price);
		CJpbDlg *pJpbDlg = dynamic_cast<CMainDlg *>(AfxGetApp()->GetMainWnd())->m_pJpbDlg;
		ASSERT(pJpbDlg);
		Xiaoshu2String(m_vecDataList[i].Price, pJpbDlg->m_curXiaoshu, xiaoshu);

		// 先画下划线小数部分
		canvas.DrawText(xiaoshu, &secondRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE, m_pfSystemUnderLine);
		// 画整数部分
		CRect tmpRect = secondRect;
		CSize xsSize = pDC->GetTextExtent(xiaoshu);
		tmpRect.DeflateRect(0, 0, xsSize.cx, 0);
		canvas.DrawText(zhengshu, &tmpRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	
		// 绘制成交量
		CString strVol;
		strVol.Format(_T("%d"), m_vecDataList[i].Vol2);
		canvas.DrawText(strVol, &thirdRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		nIndex++;
	}
}

static bool bOnce = true;
void CZhubiContainer::OnRecvData(CThostFtdcDepthMarketDataField *pData)
{
	// 逐笔数据：时间、成交价、成交量
	TThostFtdcVolumeType vol = pData->Volume;
	if (m_vecDataList.size() > 0)
		vol -= m_vecDataList[m_vecDataList.size() - 1].Vol;
	ZHUBI_DATA data(CString(pData->UpdateTime), pData->LastPrice, pData->Volume, vol);
	m_vecDataList.push_back(data);

	InitScrollBarPosition();

	if (bOnce)
	{
		bOnce = false;
		for(int i = 0; i < m_nMaxLine; i++)
			m_vecClfSplash.push_back(COLOR_SPLASH_1);
	}

	// 出现滚动条后，背景颜色往前挪一位
	if (m_nTopIndex > 0)
	{
		for(int i = 0; i < m_nMaxLine - 1; i++)
			m_vecClfSplash[i] = m_vecClfSplash[i + 1];
	}
	// 设置最新的蓝背景
	int nLatestIndex = m_vecDataList.size() - 1 - m_nTopIndex;
	if (nLatestIndex > -1 && nLatestIndex < (int)m_vecClfSplash.size())
		m_vecClfSplash[m_vecDataList.size() - 1 - m_nTopIndex] = COLOR_SPLASH_4;	

	if (m_bDrawScrollBar)
	{
		int nHScrollHeight = m_rScrollBar.Height();
		int nCount = static_cast<int>(m_vecDataList.size());
		int nPageSize = nCount / m_nMaxLine + 1;
		int nThumbLength = (150 - nCount) > 12 ? (150 - nCount) : 12;

		m_pVerticleScrollbar->SetThumbLength(nThumbLength);
		m_pVerticleScrollbar->SetPageSize(m_nMaxLine);
		m_pVerticleScrollbar->SetRowNumber(nCount);
		
		if (m_bScrollBottom)
			m_pVerticleScrollbar->UpdateThumbPosition(m_nTopIndex);
	}

	// 重绘
	Invalidate();
}

// 股票数据
void CZhubiContainer::OnRecvData(CSecurityFtdcDepthMarketDataField *pData)
{
	// 逐笔数据：时间、成交价、成交量
	TThostFtdcVolumeType vol = pData->Volume;
	if (m_vecDataList.size() > 0)
		vol -= m_vecDataList[m_vecDataList.size() - 1].Vol;
	ZHUBI_DATA data(CString(pData->UpdateTime), pData->LastPrice, pData->Volume, vol);
	m_vecDataList.push_back(data);

	InitScrollBarPosition();

	if (bOnce)
	{
		bOnce = false;
		for(int i = 0; i < m_nMaxLine; i++)
			m_vecClfSplash.push_back(COLOR_SPLASH_1);
	}

	// 出现滚动条后，背景颜色往前挪一位
	if (m_nTopIndex > 0)
	{
		for(int i = 0; i < m_nMaxLine - 1; i++)
			m_vecClfSplash[i] = m_vecClfSplash[i + 1];
	}
	// 设置最新的蓝背景
	int nLatestIndex = m_vecDataList.size() - 1 - m_nTopIndex;
	if (nLatestIndex > -1 && nLatestIndex < (int)m_vecClfSplash.size())
		m_vecClfSplash[m_vecDataList.size() - 1 - m_nTopIndex] = COLOR_SPLASH_4;	

	if (m_bDrawScrollBar)
	{
		int nHScrollHeight = m_rScrollBar.Height();
		int nCount = static_cast<int>(m_vecDataList.size());
		int nPageSize = nCount / m_nMaxLine + 1;
		int nThumbLength = (150 - nCount) > 12 ? (150 - nCount) : 12;

		m_pVerticleScrollbar->SetThumbLength(nThumbLength);
		m_pVerticleScrollbar->SetPageSize(nPageSize);
		m_pVerticleScrollbar->SetRowNumber(nCount);
		
		if (m_bScrollBottom)
			m_pVerticleScrollbar->UpdateThumbPosition(m_nTopIndex);
	}

	// 重绘
	Invalidate();
}

void CZhubiContainer::ClearData()
{
	m_vecDataList.clear();
	m_bDrawScrollBar = false;
	m_nMaxLine = 0;
	m_nTopIndex = 0;

	Invalidate();
}