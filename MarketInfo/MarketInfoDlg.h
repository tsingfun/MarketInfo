
// MarketInfoDlg.h : 头文件
//

#pragma once
#include "../GridCtrl_src/GridCtrl.h"

// CMarketInfoDlg 对话框
class CMarketInfoDlg : public CDialogEx
{
// 构造
public:
	CMarketInfoDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MARKETINFO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();

public:
	void AddOrUpdateRow2List(PSTOCK_DATA pData);
	void SetProgressMsg(CString msg);

private:
	static DWORD WINAPI ThreadCallback(LPVOID);

	void FillGridCtrl(PSTOCK_DATA pData, int row, int col);

private:
	CGridCtrl* m_pGridCtrl;
	CRITICAL_SECTION Critical;
};
