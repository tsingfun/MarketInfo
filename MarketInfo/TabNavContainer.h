#pragma once
#include "BaseContainer.h"

// CTabNavContainer

class CTabNavContainer : public CContainer
{
	DECLARE_DYNAMIC(CTabNavContainer)

public:
	CTabNavContainer();
	virtual ~CTabNavContainer();

protected:

	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()
};


