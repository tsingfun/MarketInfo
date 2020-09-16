#pragma once
#include "BaseContainer.h"

typedef struct _OV_DATA {
	// 最新
	TThostFtdcPriceType LastPrice;
	// 最新均价
	TThostFtdcPriceType AvgPrice;
	// 涨跌
	TThostFtdcPriceType ZhangDie;
	// 换手率
	TThostFtdcPriceType HuanShouRate;
	// 涨幅
	TThostFtdcPriceType ZhangDieFu;
	// 今开
	TThostFtdcPriceType OpenPrice;
	// 总手
	TThostFtdcLargeVolumeType ZongShou;
	// 最高
	TThostFtdcPriceType HighestPrice;
	// 现手
	TThostFtdcVolumeType XianShou;
	// 最低
	TThostFtdcPriceType LowestPrice;
	// 总额
	TThostFtdcPriceType Amount;
	// 量比
	TThostFtdcPriceType LiangBi;
	// 涨停
	TThostFtdcPriceType ZhangTing;
	// 跌停
	TThostFtdcPriceType DieTing;
	// 总卖
	TThostFtdcVolumeType TotalSell;
	// 总卖均价
	TThostFtdcPriceType AvgSellPrice;
	// 总买
	TThostFtdcVolumeType TotalBuy;
	// 总买均价
	TThostFtdcPriceType AvgBuyPrice;

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}

	_OV_DATA()
	{
		Reset();
	}

} OV_DATA, *POV_DATA;

// COverviewContainer

class COverviewContainer : public CContainer
{
	DECLARE_DYNAMIC(COverviewContainer)

public:
	COverviewContainer();
	virtual ~COverviewContainer();

	virtual void OnRecvData(CThostFtdcDepthMarketDataField *pData);
	virtual void OnRecvData(CSecurityFtdcDepthMarketDataField *pData);

protected:

	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()

private:
	void CompareOvData(const OV_DATA &data);

private:
	OV_DATA	m_ovData;
	
	CFont *m_pfText;
};


