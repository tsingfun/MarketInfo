#pragma once
#include <map>
#include <vector>
using namespace std;

typedef struct _JPB_DATA {
	// 期货代码
	string code;
	// 期货名称
	string name;
	// 名称简拼
	string name_jp;
	// 市场代码
	string marketcode;
	// 市场名称
	string market;
	// 保留几位小数
	int xiaoshu;

	_JPB_DATA(string c, string n, string m, int x)
	{
		code = c;
		name = n;
		name_jp = "";//TODO
		marketcode = "";
		market = m;
		xiaoshu = x;
	}
	_JPB_DATA(string c, string n, string m, string mc, int x)
	{
		code = c;
		name = n;
		name_jp = "";//TODO
		marketcode = "";
		market = m;
		marketcode = mc;
		xiaoshu = x;
	}

	void Reset()
	{
		code = "";
		name = "";
		name_jp = "";
		marketcode = "";
		market = "";
		xiaoshu = 2;
	}

	_JPB_DATA()
	{
		Reset();
	}

} JPB_DATA, *PJPB_DATA;


// 列表项高度
#define JPB_ITEM_HEIGHT 22
// 列表项显示条数
#define JPB_ITEM_NUM	8

// CJpbDlg

class CJpbDlg : public CWnd
{
	DECLARE_DYNAMIC(CJpbDlg)

public:
	CJpbDlg();
	virtual ~CJpbDlg();	

	// 显示/隐藏键盘宝对话框
	void Show(BOOL bShow);
	BOOL	m_bShow;

	void DoSearch(CString strSearch);

	// 品种数据
	map<string, JPB_DATA> m_mapData;
	
	int m_curXiaoshu;

protected:

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	LRESULT OnJpbKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnJpbKeyMouseWheel(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

private:
	// 点击是否选中某Tab项，返回其Index，否则返回-1
	int HitTest(CPoint point);

	void SelectedItemUp();
	void SelectedItemDown();

	static DWORD WINAPI ThreadCallback(LPVOID);

	// bHasMarketCode:期货没有市场代码，股票、ETF有上证、深证之分
	void LoadConfig(LPCTSTR dat, int nXiaoshu, bool bHasMarketCode);

private:
	CFont *m_pfMarket, *m_pfName;

	// 选中项索引
	int m_nTotalCount;
	int	m_nSelectedIndex;
	int m_nTopIndex;
	string m_curCode;
	CString m_strSearch;	// 键盘宝输入文字

	vector<JPB_DATA> m_vecDisp;
};


