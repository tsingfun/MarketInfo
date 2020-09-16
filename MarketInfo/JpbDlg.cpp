// JpbDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MarketInfo.h"
#include "JpbDlg.h"
#include "MemDC.h"
#include "LightCanvas.hpp"
#include "Logger.h"
#include "CommonFunc.h"
#include "UserCommon.h"

// CJpbDlg

IMPLEMENT_DYNAMIC(CJpbDlg, CWnd)

static HANDLE hThread = NULL;

// 默认几位小数
#define DEFAULT_DECIMAL	1

CJpbDlg::CJpbDlg()
:m_bShow(FALSE), m_nSelectedIndex(0), m_nTotalCount(0), m_nTopIndex(0), m_curXiaoshu(DEFAULT_DECIMAL)
{
	// 注册窗口
	CString strWndClass = AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW), 
		GetSysColorBrush(COLOR_WINDOW), NULL);

	// 创建窗口
	CreateEx(NULL, strWndClass, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, AfxGetMainWnd()->m_hWnd, NULL);

	m_pfMarket = ::GetFont(_T("宋体"), 9);
	m_pfName = ::GetFont(_T("宋体"), 11);

	// 后台线程加载码表，以免阻塞UI线程
	DWORD dwThread;
	hThread = CreateThread(NULL, 0, CJpbDlg::ThreadCallback, (LPVOID)this, 0, &dwThread);
	if (NULL == hThread)
		LOG_ERROR(_T("创建载码加载线程出错！"));
}

DWORD WINAPI CJpbDlg::ThreadCallback(LPVOID param)
{
	CJpbDlg *pJpbDlg = reinterpret_cast<CJpbDlg *>(param);

	pJpbDlg->LoadConfig(_T("futures.dat"), 1, false);
	pJpbDlg->LoadConfig(_T("ETFs.dat"), 3, true);
	pJpbDlg->LoadConfig(_T("stocks.dat"), 2, true);

	pJpbDlg->Invalidate();

	hThread = NULL;

	return TRUE;
}

void CJpbDlg::LoadConfig(LPCTSTR dat, int nXiaoshu, bool bHasMarketCode)
{
	// 读配置文件
	TCHAR szFile[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, szFile);
	_stprintf_s(szFile, _T("%s\\config\\%s"), szFile, dat);

	ifstream in(szFile);
    string line;

    if(in) // 有该文件
    {
        while (getline(in, line)) // line中不包括每行的换行符
        {
			if (bHasMarketCode)
			{
				string code, name, market, marketcode;

				size_t index1 = line.find_first_of(',', 0);
				code = line.substr(0, index1);

				size_t index2 = line.find_first_of(',', index1 + 1);
				marketcode = line.substr(index1 + 1, index2 - index1 - 1);

				size_t index3 = line.find_first_of(',', index2 + 1);
				name = line.substr(index2 + 1, index3 - index2 - 1);
				market = line.substr(index3 + 1, line.size() - 1);

				JPB_DATA data(code, name, market, marketcode, nXiaoshu);
				m_mapData[code] = data;
			}
			else
			{
				string code, name, market;

				size_t index1 = line.find_first_of(',', 0);
				code = line.substr(0, index1);

				size_t index2 = line.find_first_of(',', index1 + 1);
				name = line.substr(index1 + 1, index2 - index1 - 1);
				market = line.substr(index2 + 1, line.size() - 1);

				JPB_DATA data(code, name, market, nXiaoshu);
				m_mapData[code] = data;
			}
        }
    }
    else // 没有该文件
	{ 
		LOG_WARN(_T("期货品种配置文件%s不存在！"), szFile);
    }
}

CJpbDlg::~CJpbDlg()
{
	FREE_P(m_pfMarket)
	FREE_P(m_pfName)
}


BEGIN_MESSAGE_MAP(CJpbDlg, CWnd)
	ON_WM_PAINT()
	ON_WM_MOUSEWHEEL()
	ON_WM_ERASEBKGND()
	//ON_WM_KEYDOWN()
	ON_MESSAGE(WM_JPB_KEYDOWN, OnJpbKeyDown)
	ON_MESSAGE(WM_JPB_MOUSEWHEEL, OnJpbKeyMouseWheel)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


void CJpbDlg::Show(BOOL bShow)
{
	m_bShow = bShow;
	if (m_bShow)
	{
		MoveWindow(CAPTION_LOGO_WIDTH - 42, 22, 157, 176);
	}
	else
	{
		MoveWindow(0, 0, 0, 0);
	}
}


// CJpbDlg 消息处理程序
void CJpbDlg::OnPaint()
{
	CPaintDC dc(this);
	CMyMemDC MemDC(&dc);
	CLightCanvas canvas(MemDC);
	canvas.SetBkMode();

	CRect rect;
	GetClientRect(&rect);

	canvas.DrawRect(rect, COLOR_JPB_BORDER, COLOR_JPB_FILL);
	rect.DeflateRect(2, 0, 2, 2);
	canvas.DrawRect(rect, COLOR_JPB_IN_BORDER, COLOR_JPB_FILL);

	// 码表是否加载完毕
	if (hThread && WAIT_OBJECT_0 != WaitForSingleObject(hThread, 0))
	{
		CRect rect;
		GetClientRect(&rect);
		canvas.DrawText(_T("码表加载中,请稍候..."), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE, m_pfName);
		
		m_curCode = string(CT2A((LPCTSTR)m_strSearch));
		return;
	}

	// 查询条件过滤
	m_vecDisp.clear();
	if (!m_strSearch.IsEmpty())
	{
		for (map<string, JPB_DATA>::iterator it = m_mapData.begin(); it != m_mapData.end(); it++)
		{
			CString code(it->first.data());
			CString tmp = m_strSearch;
			if (code.MakeUpper().Find(tmp.MakeUpper()) > -1)
				m_vecDisp.push_back(it->second);
		}
	}
	//else
	//{
	//	vecDisp = m_vecFutures;
	//}
	m_nTotalCount = m_vecDisp.size();

	if (m_nTotalCount > 0)
	{
		CRect rect;
		GetClientRect(&rect);
		int nIndex = 0;

		// 画选中效果
		CRect selectedRect(0, JPB_ITEM_HEIGHT * (m_nSelectedIndex - m_nTopIndex),
			rect.right, JPB_ITEM_HEIGHT * (m_nSelectedIndex - m_nTopIndex + 1));
		canvas.DrawRect(selectedRect, COLOR_JPB_IN_BORDER, COLOR_CYAN_TEXT);

		size_t endIndex = m_vecDisp.size() > JPB_ITEM_NUM ? m_nTopIndex + JPB_ITEM_NUM : m_vecDisp.size();
		for (size_t i = m_nTopIndex; i < endIndex; i++)
		{
			int top = JPB_ITEM_HEIGHT * nIndex;
			int bottom = top + JPB_ITEM_HEIGHT;
	
			CRect firstRect (2,  top, 42, bottom);
			CRect secondRect(42, top, 95, bottom);
			CRect thirdRect (95, top, rect.Width(), bottom);

			// 绘制市场
			CString strMarket(m_vecDisp[i].market.data());
			canvas.SetTextColor(COLOR_JPB_MARKET);
			canvas.DrawText(strMarket, &firstRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE, m_pfMarket);

			// 绘制代码
			string code = m_vecDisp[i].code;
			transform(code.begin(), code.end(), code.begin(), ::toupper);
			CString strCode(code.data());
			canvas.SetTextColor(COLOR_JPB_NAME);
			canvas.DrawText(strCode, &secondRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

			// 绘制名称
			CString strName(m_vecDisp[i].name.data());
			canvas.SetTextColor(COLOR_JPB_NAME);
			canvas.DrawText(strName, &thirdRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE, m_pfName);

			nIndex++;
		}

		m_curCode = m_vecDisp[m_nSelectedIndex].code;
	}
	else
	{
		CRect rect;
		GetClientRect(&rect);
		canvas.DrawText(_T("未找到相关品种！"), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE, m_pfName);

		// 键盘宝列表中没有的话，直接查看该品种
		m_curCode = string(CT2A((LPCTSTR)m_strSearch));
	}
}

void CJpbDlg::DoSearch(CString strSearch)
{
	if (m_strSearch.CompareNoCase(strSearch) == 0)
		return;
	m_strSearch = strSearch;

	m_nSelectedIndex = 0;
	m_nTopIndex = 0;

	Invalidate();
}

BOOL CJpbDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return TRUE;
}

void CJpbDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	__super::OnMouseMove(nFlags, point);

	int nIndex = HitTest(point);
	if (nIndex > -1)
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));

		if ((nIndex + m_nTopIndex) != m_nSelectedIndex)
		{
			m_nSelectedIndex = nIndex + m_nTopIndex;
			Invalidate();
		}
	}
	else
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	}
}

void CJpbDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	__super::OnLButtonDown(nFlags, point);

	int nIndex = HitTest(point);
	if (nIndex > -1)
	{
		if (m_vecDisp.size() > 0)
			m_curXiaoshu = m_vecDisp[m_nSelectedIndex].xiaoshu;
		else
			m_curXiaoshu = DEFAULT_DECIMAL;

		::PostMessage(this->GetParent()->GetSafeHwnd(), WM_JPB_EXCHANGE, (WPARAM)m_curCode.data(), 0);
	}
}

int CJpbDlg::HitTest(CPoint point)
{
	CRect rect;
	GetClientRect(&rect);

	// 鼠标不在图片按钮区域内
	if (point.x < rect.left || point.x > rect.right
		|| point.y < rect.top || point.y > rect.bottom)
	{
		return -1;
	}

	int nIndex = (point.y - rect.top) / JPB_ITEM_HEIGHT;
	if (nIndex > m_nTotalCount - 1)
		return -1;

	return nIndex;
}

LRESULT CJpbDlg::OnJpbKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		case VK_UP:
			SelectedItemUp();
			break;

		case VK_DOWN:
			SelectedItemDown();
			break;

		case VK_ESCAPE:
			Show(FALSE);
			break;

		case VK_RETURN:
			{
				if (m_vecDisp.size() > 0)
					m_curXiaoshu = m_vecDisp[m_nSelectedIndex].xiaoshu;
				else
					m_curXiaoshu = DEFAULT_DECIMAL;
				::PostMessage(this->GetParent()->GetSafeHwnd(), WM_JPB_EXCHANGE, (WPARAM)m_curCode.data(), 0);
			}
			break;
	}

	return S_OK;
}

void CJpbDlg::SelectedItemUp()
{
	if (m_nSelectedIndex > 0)
		m_nSelectedIndex--;
	if (m_nTopIndex == m_nSelectedIndex + 1)
		m_nTopIndex--;

	Invalidate();
}

void CJpbDlg::SelectedItemDown()
{
	if (m_nSelectedIndex < m_nTotalCount - 1)
		m_nSelectedIndex++;
	if (m_nSelectedIndex == m_nTopIndex + JPB_ITEM_NUM)
		m_nTopIndex++;

	Invalidate();
}

LRESULT CJpbDlg::OnJpbKeyMouseWheel(WPARAM wParam, LPARAM lParam)
{
	if (lParam < 0)
	{
		SelectedItemDown();
	}
	else
	{
		SelectedItemUp();
	}

	return S_OK;
}
