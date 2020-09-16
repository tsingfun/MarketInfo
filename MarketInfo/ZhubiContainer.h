#pragma once
#include "BaseContainer.h"
#include "VerticalScrollbar.h"

#include <vector>
using namespace std;

// 滚动条宽度
#define SCROLLBAR_WIDTH	12
// 滚动条位移
#define SCROLLBAR_OFFSET 24

// 列表显示项的高度
#define LIST_ITEM_HEIGHT 20
// 逐笔成交时间宽度
#define ITEM_TIME_WIDTH  60

typedef struct _ZHUBI_DATA {
	// 时间
	TThostFtdcTimeType Time;
	// 成交价格
	TThostFtdcPriceType Price;
	// 成交量
	TThostFtdcVolumeType Vol;
	// 成交量(增量)
	TThostFtdcVolumeType Vol2;

	_ZHUBI_DATA(CString time, double price, int vol, int vol2)
	{
		sprintf_s(Time, "%S", time);
		Price = price;
		Vol = vol;
		Vol2 = vol2;
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}

	_ZHUBI_DATA()
	{
		Reset();
	}

} ZHUBI_DATA, *PZHUBI_DATA;

// CZhubiContainer

class CZhubiContainer : public CContainer
{
	DECLARE_DYNAMIC(CZhubiContainer)

public:
	CZhubiContainer();
	virtual ~CZhubiContainer();

	virtual void OnRecvData(CThostFtdcDepthMarketDataField *pData);
	virtual void OnRecvData(CSecurityFtdcDepthMarketDataField *pData);

	virtual void ClearData();

protected:

	afx_msg void OnPaint();

	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg LRESULT OnVScrollBarChange(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

private:
	void InitScrollBarPosition();
	BOOL InitScrollBar();

	//响应滚动条的回调
	//上滚一行
	BOOL ScrollUp();
	//上滚一页
	BOOL PageUp();
	//下滚一行
	BOOL ScrollDown();
	//下滚一页
	BOOL PageDown();
	//设置具体行
	BOOL ScrollRowTo(int nRow);

	void DrawList( CDC *pDC );

private:
	CVerticalScrollbar *m_pVerticleScrollbar;	//滚动条指针
	bool m_bDrawScrollBar;
	CRect m_rScrollBar;

	int m_nMaxLine;		// 最大行数
	int m_nTopIndex;	// 最上索引
	vector<ZHUBI_DATA>	m_vecDataList;

	bool m_bScrollBottom; // 滚动条滑块是否处于最底部
};

