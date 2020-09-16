//####################################################################
//	Comments:	透明Static控件
//
//	UpdateLogs:	
//####################################################################
#pragma once 
// CTransparentStatic

class CTransparentStatic : public CStatic
{
	DECLARE_DYNAMIC(CTransparentStatic)

public:
	CTransparentStatic();
	virtual ~CTransparentStatic();

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();
	afx_msg LRESULT OnSetText(WPARAM,LPARAM);
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
	void SetTextColor(COLORREF crText){m_crText = crText;}
	void SetBkColor(COLORREF crBk)
	{
		m_crBk = crBk;
		if (m_hbkbr)
		{
			DeleteObject(m_hbkbr);
		}
		m_hbkbr = CreateSolidBrush(m_crBk);
	}

private:
	COLORREF m_crText;
	COLORREF m_crBk;
	HBRUSH m_hbkbr;
};