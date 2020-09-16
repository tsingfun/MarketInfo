
// LiveUpdateDlg.h : 头文件
//

#pragma once
#include "BtnST\BtnST.h"
#include "ProgressBar\ProgressCtrlX.h"
#include "ComEngine.h"


// CLiveUpdateDlg 对话框

// CLiveUpdateDlg 对话框
class CLiveUpdateDlg : public CDialogEx
{
// 构造
public:
	CLiveUpdateDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_LIVEUPDATE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

protected:
	afx_msg LRESULT OnProgressSetRange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnProgressSetPos(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnProgressStepIt(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnProgressError(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDownloadStart(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDownloadEnd(WPARAM wParam, LPARAM lParam);

private:
	void UpdateMessage(LPCTSTR lpszMessage, DWORD dwMilliseconds = 0);

private:
	CButtonST m_STBtnUpgrade;
	CButtonST m_STBtnLater;
	CProgressCtrlX m_PRG;

	HANDLE m_hCancel;
	CComEngine* m_pComEngine;


	HANDLE m_hCreatePackage;	// 打开的安装包句柄
};
