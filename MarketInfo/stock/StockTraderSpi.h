#ifndef _STOCK_TRADERSPI_H_
#define _STOCK_TRADERSPI_H_

#include "SecurityMdApi/SecurityFtdcTraderApi.h"

class StockTraderSpi : public CSecurityFtdcTraderSpi
{
public:

	StockTraderSpi(CSecurityFtdcTraderApi* api):pUserApi(api){};

	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected();

	///登录请求响应
	virtual void OnRspUserLogin(CSecurityFtdcRspUserLoginField *pRspUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询合约响应
	virtual void OnRspQryInstrument(CSecurityFtdcInstrumentField *pInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询资金账户响应
	virtual void OnRspQryTradingAccount(CSecurityFtdcTradingAccountField *pTradingAccount, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询投资者持仓响应
	virtual void OnRspQryInvestorPosition(CSecurityFtdcInvestorPositionField *pInvestorPosition, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///报单录入请求响应
	virtual void OnRspOrderInsert(CSecurityFtdcInputOrderField *pInputOrder, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///报单操作请求响应
	virtual void OnRspOrderAction(CSecurityFtdcInputOrderActionField *pInputOrderAction, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);	

	///错误应答
	virtual void OnRspError(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	virtual void OnFrontDisconnected(int nReason);

	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	virtual void OnHeartBeatWarning(int nTimeLapse);	

	///报单通知
	virtual void OnRtnOrder(CSecurityFtdcOrderField *pOrder);

	///成交通知
	virtual void OnRtnTrade(CSecurityFtdcTradeField *pTrade);

public:

	///用户登录请求
	void ReqUserLogin(TSecurityFtdcBrokerIDType	appId,
		TSecurityFtdcUserIDType	userId,TSecurityFtdcPasswordType	passwd);
	
	///请求查询合约
	void ReqQryInstrument(TSecurityFtdcInstrumentIDType instId);
	
	///请求查询资金账户
	void ReqQryTradingAccount();
	
	///请求查询投资者持仓
	void ReqQryInvestorPosition(TSecurityFtdcInstrumentIDType instId);
	
	///报单录入请求
	void ReqOrderInsert(TSecurityFtdcInstrumentIDType instId,
		TSecurityFtdcDirectionType dir, TSecurityFtdcCombOffsetFlagType kpp,
		TSecurityFtdcPriceType price,   TSecurityFtdcVolumeType vol, 
		TSecurityFtdcExchangeIDType exchangID);
	
	///报单操作请求
	void ReqOrderAction(TSecurityFtdcSequenceNoType orderSeq);

	// 是否收到成功的响应
	bool IsErrorRspInfo(CSecurityFtdcRspInfoField *pRspInfo);

	void PrintOrders();
	void PrintTrades();

private:
	CSecurityFtdcTraderApi* pUserApi;
};

#endif	// _STOCK_TRADERSPI_H_