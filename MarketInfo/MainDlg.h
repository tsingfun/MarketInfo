#pragma once
#include "BaseContainer.h"
#include "JpbDlg.h"
#include "resource.h"

#include "MdSpi.h"
#include "stock\StockMdSpi.h"
#include "stock\StockTraderSpi.h"
#include "stock\StockDefine.h"

// CMainDlg 对话框

class CMainDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMainDlg)

public:
	CMainDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMainDlg();

// 对话框数据
	enum { IDD = IDD_MAINDLG };

	// 模块
	CContainer	*m_pZhubiContainer;
	CContainer	*m_pFenshiContainer;
	CContainer	*m_pQuoteContainer;
	CContainer	*m_pOverviewContainer;
	CContainer	*m_pTabNavContainer;

	// 键盘宝窗口
	CJpbDlg	*m_pJpbDlg;

	// 行情API
	static CThostFtdcMdApi *m_pApi;
	static MdSpi		   *m_pUserSpi;

	// 股票行情API
	static CSecurityFtdcMdApi *m_pStockApi;
	static StockMdSpi		  *m_pStockUserSpi;

protected:
	HICON m_hIcon;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	afx_msg LRESULT OnUserSetFocus(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSearchEditCommand();
	afx_msg LRESULT OnJpbExChange(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

private:
	// 绘制图标按钮，OnPaint里调用
	void OnDrawBmpBtn( CDC* pMemDC );

	// 点击是否选中某Tab项，返回其Index，否则返回-1
	int HitTest( CPoint point );

	// 图标按钮单击处理
	void OnBmpBtnClicked(int nIndex);

private:

	static DWORD WINAPI ThreadCallback(LPVOID);
	static DWORD WINAPI ThreadCallback_Stock(LPVOID);

	// Toolbar相关
	enum AllBmpBtn
	{
		BMP_BTN_CLOSE,		// 关闭		
		BMP_BTN_MAX,		// 最大化
		BMP_BTN_MIN,		// 最小化		
		TOTAL_BMP_BTN_COUNT
	};

	Gdiplus::Bitmap* m_pAllNormalBmpBtn[TOTAL_BMP_BTN_COUNT];
	Gdiplus::Bitmap* m_pAllHoverBmpBtn[TOTAL_BMP_BTN_COUNT];
	CBitmap *m_pCaptionLogo;

	CRect   m_rectToolbar;
	CFont	*m_pfTitle;
	int		m_nHoverIndex;
	BOOL	m_bMouseDown;

	// 主界面五个板块
	CRect	m_rectTabNav;	// 左侧导航栏
	CRect	m_rectOverview;	// 最新报价一览
	CRect	m_rectZhubi;	// 逐笔成交
	CRect	m_rectQuote10;	// 10档行情
	CRect	m_rectFenshi;	// 分时成交

	// 搜索框
	HWND	m_wndEdit;

	// 上次品种代码
	string	m_strLastCode;
};
