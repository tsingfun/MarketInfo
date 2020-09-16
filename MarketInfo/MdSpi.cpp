#include "StdAfx.h"
#include "MdSpi.h"
#include<afx.h>
#include"resource.h"
#include "afxdialogex.h"
#include "MainDlg.h"

using namespace std;
ostream& operator << (ostream& out, string& str)
{
	return out<<str.c_str();
}; 

//char FRONT_ADDR[] = "tcp://180.169.124.109:41213";		// 前置地址
//TThostFtdcBrokerIDType	BROKER_ID = "20000";			// 经纪公司代码
TThostFtdcInvestorIDType INVESTOR_ID = "00092";			// 投资者代码
TThostFtdcPasswordType  PASSWORD = "888888";			// 用户密码
//char *ppInstrumentID[] = {"cu1501", "cu1503", "a1505", "ag1503","al1506"};		// 行情订阅列表
//char *ppInstrumentID[] = {"cu1503"};
//int iInstrumentID = _countof(ppInstrumentID);									// 行情订阅数量
int iRequestID = 0;


MdSpi::MdSpi(CThostFtdcMdApi *pUserApi, HWND hWnd) : m_pUserApi(pUserApi)
{
	m_hWnd = hWnd;
}

void MdSpi::BeginConnect()
{

}

///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
void MdSpi::OnFrontConnected()
{
	TRACE("Step 1 Success: 客户端与交易后台连接成功！\n");
	TRACE("$ 信息来源： OnFrontConnected\n"); 
	TRACE("----------------------------------------------------------------------------\n");
	///用户登录请求
	TRACE("Step 2: 用户请求登陆......\n");
	ReqUserLogin();
}

///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
void MdSpi::OnFrontDisconnected(int nReason)
	{
		TRACE("$ 信息来源： OnFrontDisconnected\n");
		TRACE("$ Reason = %d\n", nReason);
		TRACE("客户端与交易后台连接断开！\n");
		TRACE("==============================================================================\n");
	}

///错误应答
void MdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	TRACE("$ 错误信息来源： OnRspError\n");
	IsErrorRspInfo(pRspInfo);
}

///心跳超时警告。当长时间未收到报文时，该方法被调用。
///@param nTimeLapse 距离上次接收报文的时间
void MdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	TRACE("$ 错误信息来源： OnHeartBeatWarning\n");
	TRACE("$ nTimerLapse = %d", nTimeLapse);
}

///登录请求响应
void MdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(!IsErrorRspInfo(pRspInfo))
	{
		TRACE("Step 2 Success: 用户请求登陆成功！\n");
	}
	else
	{
		TRACE("Step 2 Failed: 用户请求登陆失败！\n");
	}
	TRACE("$信息来源: OnRspUserLogin\n");
	TRACE("----------------------------------------------------------------------------\n");

	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///获取当前交易日
		TRACE("Step 3: 获取当前交易日......\n");
		TRACE("$ 获取当前交易日 = %s\n", m_pUserApi->GetTradingDay());
		TRACE("Step 3 Success: 获取当前交易日成功\n");
		TRACE("----------------------------------------------------------------------------\n");

		TRACE("Step 4: 用户请求订阅行情，或者绘制数据图：\n");
		TRACE("\tdata: 请求订阅行情；\n");
		TRACE("\tdraw: 绘制数据图。\n");

		// 请求订阅行情
		TRACE("用户请求订阅行情......\n");
		SubscribeMarketData();
	}
}

///订阅行情应答
void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(!pRspInfo->ErrorID)
	{
		TRACE("Step 4 Success: 用户请求订阅行情成功！\n");
	}
	else
	{
		TRACE("Step 4 Failed: 用户请求订阅行情失败！\n");
	}
	TRACE("$信息来源: %s\n.", __FUNCTION__);
	TRACE("----------------------------------------------------------------------------\n");

	////弄清楚行情数据的具体结构
	//string path=pSpecificInstrument->InstrumentID;
	//path.append("_").append(m_pUserApi->GetTradingDay()).append(".txt");
	//fstream out(path,ios::app);
	//out<<"合约代码,\t买一价,\t买一量,\t卖一价,\t卖一量,\t交易量,\t最高价,\t最低价,\t最新价,\t交易日,\t最后修改时间"<<endl;
	//out.close();
}

///取消订阅行情应答
void MdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	TRACE("$信息来源: %s\n.", __FUNCTION__);
}

///深度行情通知
void MdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	TRACE("$深度行情: %s\n.", __FUNCTION__);
	//SetEvent(g_hEvent);

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

	/*CThostFtdcDepthMarketDataField dtField;
	memset(&dtField,0,sizeof(dtField));
	dtField = *pDepthMarketData;

	TThostFtdcPriceType openPrice = dtField.OpenPrice;
	TThostFtdcPriceType maxPrize = openPrice * 1.1;
	TThostFtdcPriceType minPrize = openPrice * 0.9;
	
	STOCK_DATA data;
	memcpy(data.Name, dtField.InstrumentID, 30);
	data.Price = dtField.LastPrice;
	data.Buy1 =	dtField.BidPrice1;
	data.Buy2 = dtField.BidPrice2;
	data.Buy3 = dtField.BidPrice3;
	data.Buy4 = dtField.BidPrice4;
	data.Buy5 = dtField.BidPrice5;
	data.Sell1 = dtField.AskPrice1;
	data.Sell2 = dtField.AskPrice2;
	data.Sell3 = dtField.AskPrice3;
	data.Sell4 = dtField.AskPrice4;
	data.Sell5 = dtField.AskPrice5;
	if(data.Buy1 < minPrize || data.Buy1 > maxPrize)
	{
		data.Buy1 =	0;
	}
	if(data.Buy2 < minPrize || data.Buy2 > maxPrize)
	{
		data.Buy2 =	0;
	}
	if(data.Buy3 < minPrize || data.Buy3 > maxPrize)
	{
		data.Buy3 =	0;
	}
	if(data.Buy4 < minPrize || data.Buy4 > maxPrize)
	{
		data.Buy4 =	0;
	}
	if(data.Buy5 < minPrize || data.Buy5 > maxPrize)
	{
		data.Buy5 =	0;
	}
	if(data.Sell1 < minPrize || data.Sell1 > maxPrize)
	{
		data.Sell1 = 0;
	}
	if(data.Sell2 < minPrize || data.Sell2 > maxPrize)
	{
		data.Sell2 = 0;
	}
	if(data.Sell3 < minPrize || data.Sell3 > maxPrize)
	{
		data.Sell3 = 0;
	}
	if(data.Sell4 < minPrize || data.Sell4 > maxPrize)
	{
		data.Sell4 = 0;
	}
	if(data.Sell5 < minPrize || data.Sell5 > maxPrize)
	{
		data.Sell5 = 0;
	}

	CMarketInfoDlg *dlg = dynamic_cast<CMarketInfoDlg *>(AfxGetApp()->GetMainWnd());
	dlg->AddOrUpdateRow2List(&data);*/

	//string path="";
	//path.append(pDepthMarketData->InstrumentID).append("_").append(pDepthMarketData->TradingDay).append(".txt");
	//fstream out;
	//out.open(path,ofstream::app);
	//out<<"合约代码,\t买一价\t买一量\t卖一价\t卖一量\t交易量\t最高价\t最低价\t最新价"<<endl;
	//out<<dtField.InstrumentID<<",\t";
	//out<<dtField.BidPrice1<<",\t"<<dtField.BidVolume1<<",\t";
	//out<<dtField.AskPrice1<<",\t"<<dtField.AskVolume1<<",\t";
	//out<<dtField.Volume<<",\t";
	//out<<dtField.HighestPrice<<","<<dtField.LowestPrice<<",\t"<<dtField.LastPrice<<",\t";
	//out<<dtField.TradingDay<<",\t"<<dtField.UpdateTime<<endl;
	//out.close();
	//cout<<dtField.UpdateMillisec<<endl;

	//string key=dtField.InstrumentID;
	//stringstream ss;
	//ss<<key<<"\t"<<dtField.AskPrice5<<"\t"<<dtField.AskPrice4<<"\t"<<dtField.AskPrice3<<"\t"<<dtField.AskPrice2<<"\t"<<dtField.AskPrice1<<"\t";
	//ss<<dtField.LastPrice<<"\t";
	//ss<<dtField.AskPrice1<<"\t"<<dtField.AskPrice2<<"\t"<<dtField.AskPrice3<<"\t"<<dtField.AskPrice4<<"\t"<<dtField.AskPrice5<<"\r\n";
	//string str1;
	//ss>>str1;
	//ss.clear();
	//
	//ss<<"\t"<<dtField.AskVolume5<<"\t"<<dtField.AskVolume4<<"\t"<<dtField.AskVolume3<<"\t"<<dtField.AskVolume2<<"\t"<<dtField.AskVolume1<<"\t";
	//ss<<dtField.Volume<<"\t";
	//ss<<dtField.BidVolume1<<"\t"<<dtField.BidVolume2<<"\t"<<dtField.BidVolume3<<"\t"<<dtField.BidVolume4<<"\t"<<dtField.BidVolume5<<"\r\n";
	//string str2;
	//ss>>str2;
	//ss.clear();

	//CWnd* pWnd = AfxGetApp()->GetMainWnd();
	//pWnd->SetDlgItemText(IDC_EDIT1 ,(CString)str1.c_str());
	//pWnd->SetDlgItemText(IDC_EDIT2 ,(CString)str2.c_str());
	////CMarketInfoDlg* pWnd = (CMarketInfoDlg*)CWnd::FromHandle(m_hWnd);
	//if(key.compare(ppInstrumentID[0])==0)
	//{
	//	pWnd->GetDlgItem(IDC_EDIT1)->SetWindowText((CString)str1.c_str());  
	//	pWnd->GetDlgItem(IDC_EDIT2)->SetWindowText((CString)str2.c_str());  
	//	pWnd->UpdateData(false);
	//}
	////to do
	//
	////UpdateData(FALSE);
}

void MdSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, BROKER_ID);
	strcpy_s(req.UserID, INVESTOR_ID);
	strcpy_s(req.Password, PASSWORD);
	int iResult = m_pUserApi->ReqUserLogin(&req, ++iRequestID);
	TRACE("$ 发送用户登录请求: %s\n.",((iResult == 0) ? "成功" : "失败"));
}

void MdSpi::SubscribeMarketData()
{
	int iResult = m_pUserApi->SubscribeMarketData(m_ppInstrumentID, m_nInstrumentIDNum);
	TRACE("$ 发送行情订阅请求: \n.", ((iResult == 0) ? "成功" : "失败"));
}

void MdSpi::UnSubscribeMarketData()
{
	int iResult = m_pUserApi->UnSubscribeMarketData(m_ppInstrumentID, m_nInstrumentIDNum);
	TRACE("$ 发送取消行情订阅请求: \n.", ((iResult == 0) ? "成功" : "失败"));
}
	 
bool MdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		TRACE("$ ErrorID= %d , ErrorMsg= %s\n.", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	return bResult;
}
