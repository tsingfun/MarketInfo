// MainDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MarketInfo.h"
#include "MainDlg.h"
#include "afxdialogex.h"
#include "CommonFunc.h"
#include "MemDC.h"
#include "LightCanvas.hpp"
#include <GdiPlusInit.h>

#include "ZhubiContainer.h"
#include "FenshiContainer.h"
#include "QuoteContainer.h"
#include "OverviewContainer.h"
#include "TabNavContainer.h"
#include "JpbDlg.h"
#include "UserCommon.h"

static const int BMP_BTN_WIDTH = 32;
static const int BMP_BTN_HEIGHT = 18;
// 工具栏高度
static const int TOOLBAR_HEIGHT = 25;
// 左侧导航宽度
static const int NAVBAR_WIDTH = 20;
// 最新报价一览大小
static const int OVERVIEW_WIDTH = 200;
static const int OVERVIEW_HEIGHT = 180;
// 10档行情高度
static const int QUOTE10_HEIGHT = 400;

// CMainDlg 对话框

IMPLEMENT_DYNAMIC(CMainDlg, CDialogEx)

CThostFtdcMdApi *CMainDlg::m_pApi = NULL;
MdSpi *CMainDlg::m_pUserSpi = NULL;

CSecurityFtdcMdApi *CMainDlg::m_pStockApi = NULL;
StockMdSpi *CMainDlg::m_pStockUserSpi = NULL;


CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CMainDlg::IDD, pParent), m_nHoverIndex(-1), m_bMouseDown(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_strLastCode = (char*)DEFAULT_CODE;

	// 初始化GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

CMainDlg::~CMainDlg()
{
	for(int i = 0; i < TOTAL_BMP_BTN_COUNT; i++)
	{
		FREE_P(m_pAllNormalBmpBtn[i])
		FREE_P(m_pAllHoverBmpBtn[i])
	}

	FREE_P(m_pfTitle)
	FREE_P(m_pZhubiContainer)
	FREE_P(m_pFenshiContainer)
	FREE_P(m_pQuoteContainer)
	FREE_P(m_pOverviewContainer)
	FREE_P(m_pTabNavContainer)
	FREE_P(m_pJpbDlg)
	FREE_P(m_pCaptionLogo)
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMainDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	// 设置键盘宝搜索框焦点
	ON_MESSAGE(WM_USER_SETFOCUS_MSG, OnUserSetFocus)
	// 键盘宝搜索框输入事件
	ON_EN_CHANGE(IDC_EDIT_SEARCH, OnSearchEditCommand)
	// 键盘宝切换代码
	ON_MESSAGE(WM_JPB_EXCHANGE, OnJpbExChange)
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()


BOOL CMainDlg::OnInitDialog()
{
	__super::OnInitDialog();

	// 初始化代码
	m_pAllNormalBmpBtn[BMP_BTN_CLOSE] = GetBitmapPlus(_T("toolbar_close.bmp"));
	m_pAllNormalBmpBtn[BMP_BTN_MAX] = GetBitmapPlus(_T("toolbar_max.bmp"));
	m_pAllNormalBmpBtn[BMP_BTN_MIN] = GetBitmapPlus(_T("toolbar_min.bmp"));
	
	m_pAllHoverBmpBtn[BMP_BTN_CLOSE] = GetBitmapPlus(_T("toolbar_close_hover.bmp"));
	m_pAllHoverBmpBtn[BMP_BTN_MAX] = GetBitmapPlus(_T("toolbar_max_hover.bmp"));
	m_pAllHoverBmpBtn[BMP_BTN_MIN] = GetBitmapPlus(_T("toolbar_min_hover.bmp"));

	m_pCaptionLogo = ::GetBitmap(_T("caption_logo.bmp"));

	m_pfTitle = ::GetFont(_T("宋体"), 10, FW_BOLD);


	// 布局
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);
	int w = 400;
	int h = 705;
	int dx = (cx - w) / 2;
	int dy = (cy - h) / 2 - 12;

	// 恢复上次关闭时窗口位置
    int x = theApp.GetProfileInt(SECTION_DROPLOCATION, INI_KEY_MAIN_X, dx);
    int y = theApp.GetProfileInt(SECTION_DROPLOCATION, INI_KEY_MAIN_Y, dy);

    // Work Area
    RECT rectWorkArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rectWorkArea, 0);
    // Left
    if (x < 0)
        x = 0;

    // Right
    int tmp = rectWorkArea.right - w;
    if (x > tmp)
        x = tmp;

    // Bottom
    tmp = rectWorkArea.bottom - h;
    if (y > tmp)
        y = tmp;
	
	MoveWindow(x, y, w, h, TRUE);

	// Toolbar
	CRect rect;
	GetClientRect(&rect);
	m_rectToolbar.SetRect(0, 0, rect.Width(), TOOLBAR_HEIGHT);

	// 左侧导航栏
	m_rectTabNav.SetRect(0, TOOLBAR_HEIGHT, NAVBAR_WIDTH, rect.Height());

	// 最新报价一览
	m_rectOverview.SetRect(NAVBAR_WIDTH, TOOLBAR_HEIGHT, 
						OVERVIEW_WIDTH + NAVBAR_WIDTH, OVERVIEW_HEIGHT + TOOLBAR_HEIGHT);

	// 逐笔成交
	m_rectZhubi.SetRect(NAVBAR_WIDTH, TOOLBAR_HEIGHT + OVERVIEW_HEIGHT, 
						OVERVIEW_WIDTH + NAVBAR_WIDTH, rect.Height());

	// 10档行情
	m_rectQuote10.SetRect(NAVBAR_WIDTH + OVERVIEW_WIDTH, TOOLBAR_HEIGHT, 
						rect.Width(), QUOTE10_HEIGHT + TOOLBAR_HEIGHT);

	// 分时成交
	m_rectFenshi.SetRect(NAVBAR_WIDTH + OVERVIEW_WIDTH, TOOLBAR_HEIGHT + QUOTE10_HEIGHT, 
						rect.Width(), rect.Height());

	// ---------------------------------------------------------
	m_pZhubiContainer = new CZhubiContainer();
	m_pZhubiContainer->MoveWindow(m_rectZhubi);
	m_pZhubiContainer->ShowWindow(SW_SHOW);

	m_pFenshiContainer = new CFenshiContainer();
	m_pFenshiContainer->MoveWindow(m_rectFenshi);
	m_pFenshiContainer->ShowWindow(SW_SHOW);

	m_pQuoteContainer = new CQuoteContainer();
	m_pQuoteContainer->MoveWindow(m_rectQuote10);
	m_pQuoteContainer->ShowWindow(SW_SHOW);

	m_pOverviewContainer = new COverviewContainer();
	m_pOverviewContainer->MoveWindow(m_rectOverview);
	m_pOverviewContainer->ShowWindow(SW_SHOW);

	m_pTabNavContainer = new CTabNavContainer();
	m_pTabNavContainer->MoveWindow(m_rectTabNav);
	m_pTabNavContainer->ShowWindow(SW_SHOW);

	// 创建EditBox
	m_wndEdit = CreateWindow(_T("EDIT"), 0, WS_CHILD | WS_VISIBLE | ES_WANTRETURN, 
		CAPTION_LOGO_WIDTH, 4, 60, 16, m_hWnd, (HMENU)IDC_EDIT_SEARCH, (HINSTANCE)GetWindowLong(m_hWnd, GWL_HINSTANCE), 0);
	//::SetWindowLong(m_wndEdit, GWL_ID, IDC_EDIT_SEARCH);
	::ShowWindow(m_wndEdit, SW_SHOW);
	//::SetFocus(m_wndEdit);
	PostMessage(WM_USER_SETFOCUS_MSG, 0, 0);

	// 显示键盘宝对话框
	m_pJpbDlg = new CJpbDlg();
	m_pJpbDlg->SetParent(this);

	// 请求行情
	DWORD dwThread;
	HANDLE hThread = CreateThread(NULL, 0, CMainDlg::ThreadCallback, 0, 0, &dwThread);
	if (NULL == hThread)
		TRACE("创建期货线程出错！\n");

	HANDLE hThreadStock = CreateThread(NULL, 0, CMainDlg::ThreadCallback_Stock, 0, 0, &dwThread);
	if (NULL == hThreadStock)
		TRACE("创建股票线程出错！\n");

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

DWORD WINAPI CMainDlg::ThreadCallback(LPVOID code)
{
	// ----------------------------------------
	// 创建UserApi
	m_pApi = CThostFtdcMdApi::CreateFtdcMdApi();
	m_pUserSpi = new MdSpi(m_pApi, AfxGetApp()->m_pMainWnd->m_hWnd);

	// 默认订阅
	char *data[] = { (char*)DEFAULT_CODE };
	m_pUserSpi->m_ppInstrumentID = data;
	m_pUserSpi->m_nInstrumentIDNum = _countof(data);

	// 注册事件类
	m_pApi->RegisterSpi(m_pUserSpi);
	TRACE("当前默认服务器为永安--电信实盘交易服务端：%s.\n", FRONT_ADDR);
	TRACE("当前默认Broker ID为：%s\n.", BROKER_ID);
	// connect
	m_pApi->RegisterFront(FRONT_ADDR);
	
	m_pApi->Init();
	TRACE("Step 1: 客户端准备与交易后台进行连接......\n.");

	m_pApi->Join();

	return 0;
}

DWORD WINAPI CMainDlg::ThreadCallback_Stock(LPVOID code)
{
	// ----------------------------------------
	// 股票UserApi
	m_pStockApi = CSecurityFtdcMdApi::CreateFtdcMdApi();
	m_pStockUserSpi = new StockMdSpi(m_pStockApi);	/// 创建回调处理类对象MdSpi
	m_pStockApi->RegisterSpi(m_pStockUserSpi);		/// 回调对象注入接口类
	m_pStockApi->RegisterFront(STOCK_FRONT_ADDR);	/// 注册行情前置地址
	m_pStockApi->Init();								/// 接口线程启动, 开始工作

	m_pStockApi->Join();		///等待接口线程退出
	m_pStockApi->Release();	///接口对象释放

	return 0;
}

// 键盘宝切换品种
LRESULT CMainDlg::OnJpbExChange(WPARAM wParam, LPARAM lParam)
{
	// 关键盘宝对话框
	m_pJpbDlg->Show(FALSE);
	// 清理搜索框
	SetDlgItemText(IDC_EDIT_SEARCH, _T(""));

	// code不变，没有切换
	if (m_strLastCode.compare(string((char*)wParam)) == 0)
		return S_OK;

	// 清理各个模块
	m_pZhubiContainer->ClearData();		// 清理逐笔列表数据	

	// 取消订阅上次的
	if (!IsStock(m_strLastCode))
	{
		char *udata[] = { (char*)( m_strLastCode.empty() ? DEFAULT_CODE : m_strLastCode.data() ) };
		m_pUserSpi->m_ppInstrumentID = udata;
		m_pUserSpi->m_nInstrumentIDNum = _countof(udata);
		m_pUserSpi->UnSubscribeMarketData();
	}
	else
	{
		CJpbDlg *pJpbDlg = dynamic_cast<CMainDlg *>(AfxGetApp()->GetMainWnd())->m_pJpbDlg;
		ASSERT(pJpbDlg);

		char *udata = (char*)( m_strLastCode.empty() ? STOCK_DEFAULT_CODE : m_strLastCode.data() );
		char instIdList[MAX_PATH] = {0};
		strcpy_s(instIdList, udata);
		m_pStockUserSpi->UnSubscribeMarketData(instIdList, (char*)pJpbDlg->m_mapData[string(udata)].marketcode.data());
	}

	// 订阅新的
	if (!IsStock(std::string((char*)wParam)))
	{		
		char *data[] = { (char*)wParam };
		m_pUserSpi->m_ppInstrumentID = data;
		m_pUserSpi->m_nInstrumentIDNum = _countof(data);
		m_pUserSpi->SubscribeMarketData();
	}
	else
	{
		CJpbDlg *pJpbDlg = dynamic_cast<CMainDlg *>(AfxGetApp()->GetMainWnd())->m_pJpbDlg;
		ASSERT(pJpbDlg);

		string code = string((char*)wParam);
		char instIdList[MAX_PATH] = {0};
		strcpy_s(instIdList, code.data());

		string marketcode = string("SSE");	// 码表找不到的默认“上证”
		if (pJpbDlg->m_mapData.find(code) != pJpbDlg->m_mapData.end())
			marketcode = pJpbDlg->m_mapData[code].marketcode;
		
		m_pStockUserSpi->SubscribeMarketData(instIdList, (char*)marketcode.data());
	}

	// Save last code
	m_strLastCode = (char*)wParam;

	return S_OK;
}

LRESULT CMainDlg::OnUserSetFocus(WPARAM wParam, LPARAM lParam)
{
	::SetFocus(m_wndEdit);

	return S_OK;
}

void CMainDlg::OnSearchEditCommand()
{
	CString strSearch;
	GetDlgItemText(IDC_EDIT_SEARCH, strSearch);
	//AfxMessageBox(strSearch);

	if (!strSearch.IsEmpty())
		m_pJpbDlg->Show(TRUE);
	else
		m_pJpbDlg->Show(FALSE);

	m_pJpbDlg->DoSearch(strSearch);
}

BOOL CMainDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// nFlags:指示虚拟按键是否按下,MK_CONTROL...

	// 发送消息给子窗口
	if (m_pJpbDlg->m_bShow)
		::PostMessage(m_pJpbDlg->GetSafeHwnd(), WM_JPB_MOUSEWHEEL, nFlags, zDelta);

	return __super::OnMouseWheel(nFlags, zDelta, pt);
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN
			|| pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		{
			::PostMessage(m_pJpbDlg->GetSafeHwnd(), WM_JPB_KEYDOWN, pMsg->wParam, pMsg->lParam);
			return TRUE;
		}
	}

	return __super::PreTranslateMessage(pMsg);
}

void CMainDlg::OnPaint()
{
	if (IsIconic())	// 窗口最小化的绘制
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		// 排除键盘宝对话框，不刷新
		if (m_pJpbDlg->m_bShow)
		{
			CRect jpbRect;
			m_pJpbDlg->GetClientRect(jpbRect);
			ExcludeClipRect(dc, CAPTION_LOGO_WIDTH - 42, jpbRect.top + 22, jpbRect.right + 50, 25);
		}
		// 排除键盘宝输入框，不刷新
		ExcludeClipRect(dc, CAPTION_LOGO_WIDTH, 4, CAPTION_LOGO_WIDTH + 60, 20);

		CMyMemDC MemDC(&dc);
		CLightCanvas canvas(MemDC);
		canvas.SetBkMode();

		// 填充黑色背景
		canvas.DrawRect(m_rectToolbar, COLOR_TOOLBAR_BG, COLOR_TOOLBAR_BG);

		// 软件名称(图片)
		canvas.DrawBitmap(m_rectToolbar.left, m_rectToolbar.top, CAPTION_LOGO_WIDTH, m_rectToolbar.bottom, m_pCaptionLogo);

		// 绘制按钮
		OnDrawBmpBtn(&MemDC);
	}
}

BOOL CMainDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return TRUE;
}

void CMainDlg::OnDrawBmpBtn( CDC* pMemDC )
{
	Gdiplus::Graphics graphics(pMemDC->m_hDC);
	int nX = m_rectToolbar.right;
	int nY = 0;
	Gdiplus::Bitmap* pBmpItem = NULL;
	for (int i = 0; i < TOTAL_BMP_BTN_COUNT; i++)
	{
		pBmpItem = (i == m_nHoverIndex) ? m_pAllHoverBmpBtn[i] : m_pAllNormalBmpBtn[i];
		if (pBmpItem)
		{
			nX -= pBmpItem->GetWidth();
			Gdiplus::Rect itemRect(nX, nY, pBmpItem->GetWidth(), pBmpItem->GetHeight());
			graphics.DrawImage(pBmpItem, itemRect);			
			pBmpItem = NULL;
		}
	}
}


HBRUSH CMainDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CMainDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	__super::OnMouseMove(nFlags, point);

	int nIndex = HitTest(point);
	if ( nIndex > -1 )
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));

		if (nIndex != m_nHoverIndex)
		{
			m_nHoverIndex = nIndex;
			InvalidateRect(m_rectToolbar);
		}		
	}
	else
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

		if (nIndex != m_nHoverIndex)
		{
			m_nHoverIndex = -1;
			InvalidateRect(m_rectToolbar);
		}		
	}

	// 拖动窗口
	if (m_bMouseDown && point.y < m_rectToolbar.bottom)
	{
		PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
	}
}

void CMainDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	__super::OnLButtonDown(nFlags, point);

	int nIndex = HitTest(point);
	if ( nIndex > -1 )
	{
		OnBmpBtnClicked(nIndex);
	}
	else
	{
		m_bMouseDown = TRUE;
	}
}
void CMainDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	__super::OnLButtonUp(nFlags, point);

	m_bMouseDown = FALSE;
}

int CMainDlg::HitTest( CPoint point )
{
	// 鼠标不在图片按钮区域内
	if (point.x < m_rectToolbar.left || point.x > m_rectToolbar.right 
		|| point.y < m_rectToolbar.top || point.y > m_rectToolbar.bottom)
	{
		return -1;
	}

	int nIndex = (m_rectToolbar.right - point.x) / BMP_BTN_WIDTH;
	if (nIndex > TOTAL_BMP_BTN_COUNT - 1 || point.y > BMP_BTN_HEIGHT)
		return -1;

	return nIndex;
}
void TestStockMdSpi();
void CMainDlg::OnBmpBtnClicked(int nIndex)
{
	switch (nIndex)
	{
	case BMP_BTN_MIN:
		{
			m_nHoverIndex = -1;

			WINDOWPLACEMENT lwndpl;
			WINDOWPLACEMENT *lpwndpl;
			lpwndpl = &lwndpl;
			GetWindowPlacement(lpwndpl);
			lpwndpl->showCmd = SW_SHOWMINIMIZED;
			SetWindowPlacement(lpwndpl);
		}
		break;

	case BMP_BTN_MAX:
		break;

	case BMP_BTN_CLOSE:
		{
			CRect mainRect;
			this->GetWindowRect(&mainRect);
    
			// Write INI:记录关闭时窗口位置
			theApp.WriteProfileInt(SECTION_DROPLOCATION, INI_KEY_MAIN_X, mainRect.left);
			theApp.WriteProfileInt(SECTION_DROPLOCATION, INI_KEY_MAIN_Y, mainRect.top );

			PostQuitMessage(0);
			//OnCancel();
		}
		break;

	default:
		break;
	}
}

void TestStockTradeSpi()
{
	//初始化UserApi
	CSecurityFtdcTraderApi* pUserApi = CSecurityFtdcTraderApi::CreateFtdcTraderApi();
	StockTraderSpi* pUserSpi = new StockTraderSpi(pUserApi);
	pUserApi->RegisterSpi((CSecurityFtdcTraderSpi*)pUserSpi);		// 注册事件类
	pUserApi->SubscribePublicTopic(SECURITY_TERT_RESTART);			// 注册公有流
	pUserApi->SubscribePrivateTopic(SECURITY_TERT_RESTART);			// 注册私有流
	pUserApi->RegisterFront(STOCK_TRADE_FRONT_ADDR);					// 注册交易前置地址

	pUserApi->Init();

	// Test...
	pUserSpi->PrintOrders();
	pUserSpi->PrintTrades();

	pUserApi->Join();  
	pUserApi->Release();
}