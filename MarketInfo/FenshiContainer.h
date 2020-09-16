#pragma once
#include "BaseContainer.h"

// CFenshiContainer

class CFenshiContainer : public CContainer
{
	DECLARE_DYNAMIC(CFenshiContainer)

public:
	CFenshiContainer();
	virtual ~CFenshiContainer();

protected:

	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()
};


