#ifndef __COMMON_FUNC_H_
#define __COMMON_FUNC_H_
#include "Logger.h"

#define FREE_P(p) if (p) { delete p; p = NULL; }
#define FREE_ARR(p) if (p) { delete [] p; p = NULL; }

// 浮点数是否等于零
#define ZERO_FLOAT(f) ((f) > -1e-5 && (f) < 1e-5)
#define ZERO_DOUBLE(f) ZERO_FLOAT(f)
#define ZERO_MARK	_T("---")

inline CBitmap* GetBitmap(LPCTSTR lpszBitmapPath)
{
	CBitmap *pBitmap = new CBitmap();
	HBITMAP bitmap;
	try
	{
		pBitmap->Detach();
		// 从文件路径加载图片
		TCHAR szFilePath[MAX_PATH] = {0};
		GetCurrentDirectory(MAX_PATH, szFilePath);
		_stprintf_s(szFilePath, _T("%s/config/bitmap/%s"), szFilePath, lpszBitmapPath);

		bitmap = (HBITMAP)::LoadImage(NULL, szFilePath, IMAGE_BITMAP, 0, 0,
			LR_CREATEDIBSECTION | LR_LOADFROMFILE | LR_DEFAULTSIZE);
		if (pBitmap->Attach(bitmap))
			return pBitmap;
	}
	catch (...)
	{
		LOG_ERROR(_T("位图加载失败！%S"), lpszBitmapPath);
	}

	return NULL;
}

typedef Gdiplus::Bitmap CBitmapPlus;
inline CBitmapPlus* GetBitmapPlus(LPCTSTR lpszBitmapPath)
{
	CBitmapPlus *pBitmap = NULL;
	try
	{
		// 从文件路径加载图片
		TCHAR szFilePath[MAX_PATH] = {0};
		GetCurrentDirectory(MAX_PATH, szFilePath);
		_stprintf_s(szFilePath, _T("%s/config/bitmap/%s"), szFilePath, lpszBitmapPath);

		pBitmap = CBitmapPlus::FromFile(szFilePath);
		
		if (pBitmap)
			return pBitmap;
	}
	catch (...)
	{
		LOG_ERROR(_T("位图Plus加载失败！%S"), lpszBitmapPath);
	}

	return NULL;
}

///获取字体(字体名，height,weight,下划线，倾斜角度)
inline CFont* GetFont(LPCTSTR lpszFace, int nHeight, int nWeight = FW_NORMAL, BOOL bUnderline = FALSE, int fEscapement = 0)
{
	HDC hDC = ::GetWindowDC(NULL);

	// 使用 PX 会精确控制字体高度，而不是字号转换字体高度
	//nFontHeight = nPX;
	int nFontHeight = -MulDiv(nHeight, GetDeviceCaps(hDC, LOGPIXELSY), 72);

	CFont* pf = new CFont();
	pf->CreateFont(
		nFontHeight,					// int nHeight
		0,								// int nWidth
		(int)fEscapement,				// int nEscapement
		0,								// int nOrientation
		nWeight,						// int nWeight (FW_NORMAL, FW_REGULAR, FW_BOLD)
		FALSE,							// BYTE bItalic (TRUE, FALSE)
		bUnderline,						// BYTE bUnderline (TRUE, FALSE)
		FALSE,							// BYTE cStrikeOut (TRUE, FALSE)
		DEFAULT_CHARSET,				// BYTE nCharSet (ANSI_CHARSET, DEFAULT_CHARSET)
		OUT_TT_PRECIS,					// BYTE nOutPrecision (OUT_DEVICE_PRECIS, OUT_RASTER_PRECIS, OUT_TT_PRECIS)
		CLIP_TT_ALWAYS,					// BYTE nClipPrecision (CLIP_TT_ALWAYS)
		PROOF_QUALITY,					// BYTE nQuality (DEFAULT_QUALITY, DRAFT_QUALITY, PROOF_QUALITY)
		DEFAULT_PITCH | TMPF_TRUETYPE,	// BYTE nPitchAndFamily (DEFAULT_PITCH, VARIABLE_PITCH, FIXED_PITCH | TMPF_TRUETYPE)
		lpszFace						// LPCTSTR lpszFacename (仿宋_GB2312, 黑体, 楷体_GB2312, 隶书, 宋体, 新宋体, 幼圆)
		);
	
	ReleaseDC(NULL, hDC);

	return pf;
}

inline bool IsStock(std::string szCode)
{
	return szCode[0] > 47 && szCode[0] < 58;
}


inline LPCTSTR GetFloatFormat(int n)
{
	switch (n)
	{
	case 1:
		return _T("%.1f");

	case 2:
		return _T("%.2f");

	case 3:
		return _T("%.3f");

	case 4:
		return _T("%.4f");

	default:
		return _T("%d");
	}
}

#endif __COMMON_FUNC_H_