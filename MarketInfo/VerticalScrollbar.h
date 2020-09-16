
#pragma once

#include "UserCommon.h"

/////////////////////////////////////////////////////////////////////////////
// CVerticleScrollbar window

class CVerticalScrollbar : public CStatic
{
private:
	int m_nMode;

// Construction
public:
	CVerticalScrollbar();
	virtual ~CVerticalScrollbar();

// Implementation
public:
	CWnd* m_pListWnd;
	int m_nRows;
	int m_nPageSize;

	bool m_bSignature;
	
	bool m_bMouseDownArrowUp; // 是否在上箭头
	bool m_bMouseDownArrowDown; // 下箭头
	bool m_bDragging; // 是否在拖动
	bool m_bMouseDownThumb; // 是否点中滑块
	bool m_bPageUp; //是否点中滑块与上箭头中间
	bool m_bPageDown; // 滑块与下箭头中间

	bool m_bSuspendTimer1;
	bool m_bSuspendTimer2;
	bool m_bSuspendTimer3;
	bool m_bSuspendTimer4;

	int m_nThumbTop;  // 滑块距离客户区顶部距离
	int	m_nCXVScrollNew; // 箭头宽
	int	m_nCYVScrollNew; // 箭头高
	int m_nWidth;    // 客户区宽
	int m_nHeight; // 高
	int m_vThumbLength; // 滑块长度
	int m_nCurrentPosition;
	
	CPoint m_1stPointOnMove; // 鼠标第一次点中滑块位置
	CPoint m_1stClickInPageUp; // 点击滑块上箭头之间 没有放开 鼠标位置
	CPoint m_1stClickInPageDown; 

	static CBitmap *m_pUpArrow;
	static CBitmap *m_pDownArrow;
	static CBitmap *m_pSpan;
	static CBitmap *m_pThumb1;
	static CBitmap *m_pThumb2;

	int m_nOffsetLeftMargin;
	BOOL m_bTracking;

	void LimitThumbPosition();
	void Draw(CDC* pDC);
	void UpdateThumbPosition(int currentPosition);
	void SetWidth(int width);
	int GetWidth();
	void SetHeight(int height);
	int GetHeight();

	CWnd *m_pParentWnd;
	int m_nIdEvent;
	int m_nElapse;
	
	void SetThumbLength(int length);
	void SetPageSize(int pageSize);
	void SetRowNumber(int rows);

	void SetTimerEvent(CWnd *pWnd, int nIdEvent, int nElapse);

protected:
	void InitSkinVerticalScrollbarNew();
	// Generated message map functions
protected:
	//{{AFX_MSG(CSkinVerticleScrollbar)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
};
