#pragma once
#include "BaseContainer.h"
using namespace std;

typedef struct _QUOTE_DATA {
	// 名称
	TThostFtdcInstrumentIDType Name;
	// 现价
	TThostFtdcPriceType Price;
	// 买一~五
	TThostFtdcPriceType Buy1;
	TThostFtdcPriceType Buy2;
	TThostFtdcPriceType Buy3;
	TThostFtdcPriceType Buy4;
	TThostFtdcPriceType Buy5;
	TThostFtdcVolumeType BuyVol1;
	TThostFtdcVolumeType BuyVol2;
	TThostFtdcVolumeType BuyVol3;
	TThostFtdcVolumeType BuyVol4;
	TThostFtdcVolumeType BuyVol5;
	// 卖一~五
	TThostFtdcPriceType Sell1;
	TThostFtdcPriceType Sell2;
	TThostFtdcPriceType Sell3;
	TThostFtdcPriceType Sell4;
	TThostFtdcPriceType Sell5;
	TThostFtdcVolumeType SellVol1;
	TThostFtdcVolumeType SellVol2;
	TThostFtdcVolumeType SellVol3;
	TThostFtdcVolumeType SellVol4;
	TThostFtdcVolumeType SellVol5;

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}

	_QUOTE_DATA()
	{
		Reset();
	}

} QUOTE_DATA, *PQUOTE_DATA;


#define QUOTE_NAME_WIDTH		40
#define BUY_SELL_ITEM_HEIGHT	18.5

// CQuoteContainer

class CQuoteContainer : public CContainer
{
	DECLARE_DYNAMIC(CQuoteContainer)

public:
	CQuoteContainer();
	virtual ~CQuoteContainer();

	virtual void OnRecvData(CThostFtdcDepthMarketDataField *pData);
	virtual void OnRecvData(CSecurityFtdcDepthMarketDataField *pData);

protected:

	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()

private:
	void CompareQuoteData(const QUOTE_DATA &data);

private:
	QUOTE_DATA	m_quoteData;

	CFont *m_pfCodeTitle;
	CFont *m_pfNameTitle;
	CFont *m_pfBuySell;
};


