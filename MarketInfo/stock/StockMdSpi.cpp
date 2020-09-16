#include "stdafx.h"
#include "StockMdSpi.h"
#include <vector>
#include "windows.h"
#include "../MainDlg.h"
#include "Logger.h"

using namespace std;
#pragma warning(disable : 4996)

int requestId = 0;

void StockMdSpi::OnRspError(CSecurityFtdcRspInfoField *pRspInfo,
	int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
}

void StockMdSpi::OnFrontDisconnected(int nReason)
{
	LOG_WARN(_T(" 响应 | 连接中断...(reason=%d)"), nReason);
}

void StockMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	LOG_WARN(_T(" 响应 | 心跳超时警告...(TimerLapse=%d)"), nTimeLapse);
}

void StockMdSpi::OnFrontConnected()
{
	LOG_INFO(_T(" 连接交易前置...成功"));

	ReqUserLogin("2011", "010000047178", "0090380");
}

void StockMdSpi::ReqUserLogin(TSecurityFtdcBrokerIDType	appId,
	TSecurityFtdcUserIDType	userId,	TSecurityFtdcPasswordType	passwd)
{
	CSecurityFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(req.UserID, userId);
	strcpy(req.Password, passwd);
	int ret = pUserApi->ReqUserLogin(&req, ++requestId);

	LOG_INFO(_T(" 请求 | 发送登录... %s"), (ret == 0) ? _T("成功") : _T("失败"));
}

void StockMdSpi::OnRspUserLogin(CSecurityFtdcRspUserLoginField *pRspUserLogin,
	CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
		LOG_INFO(_T(" 响应 | 登录成功...当前交易日:%s"), CA2T(pRspUserLogin->TradingDay));
}

void StockMdSpi::SubscribeMarketData(char* instIdList, char* exchangeID)
{
	vector<char*> list;
	char *token = strtok(instIdList, ",");
	while( token != NULL ){
		list.push_back(token);
		token = strtok(NULL, ",");
	}
	unsigned int len = list.size();
	char** pInstId = new char* [len];
	for(unsigned int i=0; i<len;i++)  pInstId[i]=list[i];
	int ret = pUserApi->SubscribeMarketData(pInstId, len,exchangeID);

	USES_CONVERSION;
	LOG_INFO(_T(" 请求 | 发送行情订阅(%s)... %s"), A2T(instIdList), (ret == 0) ? _T("成功") : _T("失败"));
}

void StockMdSpi::UnSubscribeMarketData(char* instIdList, char* exchangeID)
{
	vector<char*> list;
	char *token = strtok(instIdList, ",");
	while( token != NULL ){
		list.push_back(token);
		token = strtok(NULL, ",");
	}
	unsigned int len = list.size();
	char** pInstId = new char* [len];
	for(unsigned int i=0; i<len;i++)  pInstId[i]=list[i];
	int ret = pUserApi->UnSubscribeMarketData(pInstId, len, exchangeID);

	USES_CONVERSION;
	LOG_INFO(_T(" 请求 | 发送取消行情订阅(%s)... %s"), A2T(instIdList), (ret == 0) ? _T("成功") : _T("失败"));
}

void StockMdSpi::OnRspSubMarketData(
	CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, 
	CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	USES_CONVERSION;
	LOG_INFO(_T(" 响应 | 行情订阅(%s)... 成功"), A2T(pSpecificInstrument->InstrumentID));
}

void StockMdSpi::OnRspUnSubMarketData(
	CSecurityFtdcSpecificInstrumentField *pSpecificInstrument,
	CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	USES_CONVERSION;
	LOG_INFO(_T(" 响应 | 行情取消订阅(%s)... 成功"), A2T(pSpecificInstrument->InstrumentID));
}

void StockMdSpi::OnRtnDepthMarketData(
	CSecurityFtdcDepthMarketDataField *pDepthMarketData)
{
	// 数据分发
	CMainDlg *dlg = dynamic_cast<CMainDlg *>(AfxGetApp()->GetMainWnd());
	if (dlg)
	{
		dlg->m_pZhubiContainer->OnRecvData(pDepthMarketData);
		dlg->m_pFenshiContainer->OnRecvData(pDepthMarketData);
		dlg->m_pQuoteContainer->OnRecvData(pDepthMarketData);
		dlg->m_pOverviewContainer->OnRecvData(pDepthMarketData);
		dlg->m_pTabNavContainer->OnRecvData(pDepthMarketData);
	}
}

bool StockMdSpi::IsErrorRspInfo(CSecurityFtdcRspInfoField *pRspInfo)
{	
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (ret)
		LOG_ERROR(_T(" 响应 | %s"), CA2T(pRspInfo->ErrorMsg));

	return ret;
}