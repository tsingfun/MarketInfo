#pragma once
#include "macro.h"

#define NULL_COLORKEY		0xffffffff
#define COLOR_TRANSPARENT	0xffffffff

//////////////////////////////////////////////////////////////////////////
// for standard GDI only

class CLightCanvas 
	//: public CUnknown
{
public:
	CLightCanvas( CDC *lpDC )
	{
		ASSERT( NULL != lpDC );
		m_lpDC = lpDC;
		m_lpDrawDC = NULL;
		m_lpOriginalPen = NULL;
		m_lpOriginalBrush = NULL;
		m_lpOriginalFont = NULL;
		m_dwOriginalBkMode = 0;
		m_dwOriginalBkColor = COLOR_TRANSPARENT;
		m_dwOriginalTextColor = COLOR_TRANSPARENT;
	}

	virtual ~CLightCanvas(void)
	{
		ResetAll();
	}


	BOOL SetFont( CFont *lpFont )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		LOGFONT logFont;
		lpFont->GetLogFont( &logFont );

		CFont *lpNewFont = new CFont;
		ASSERT_RESULT( NULL != lpNewFont );
		lpNewFont->CreateFontIndirect( &logFont );

		if ( NULL == m_lpOriginalFont )
		{
			// first set
			m_lpOriginalFont = m_lpDC->SelectObject( lpNewFont );
		}
		else
		{
			delete m_lpDC->SelectObject( lpNewFont );
		}

		return TRUE;
	}

	BOOL SetFont( LOGFONT *logFont )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		CFont *lpNewFont = new CFont;
		ASSERT_RESULT( NULL != lpNewFont );
		lpNewFont->CreateFontIndirect( logFont );

		if ( NULL == m_lpOriginalFont )
		{
			// first set
			m_lpOriginalFont = m_lpDC->SelectObject( lpNewFont );
		}
		else
		{
			delete m_lpDC->SelectObject( lpNewFont );
		}

		return TRUE;
	}

	BOOL ResetFont()
	{
		ASSERT_RESULT( NULL != m_lpDC );

		if ( NULL != m_lpOriginalFont )
		{
			delete m_lpDC->SelectObject( m_lpOriginalFont );
			m_lpOriginalFont = NULL;
		}

		return TRUE;
	}

	//SIZE GetTextExtent( LPCWSTR lpszString, CFont *lpFont = NULL )
	//{
	//	ASSERT( NULL != m_lpDC );

	//	SIZE sizeText;
	//	if ( NULL != lpFont )
	//	{
	//		CFont *lpOldFont;
	//		lpOldFont = m_lpDC->SelectObject( lpFont );
	//		sizeText = m_lpDC->GetTextExtent( lpszString );
	//		m_lpDC->SelectObject( lpOldFont );
	//	}
	//	else
	//	{
	//		sizeText = m_lpDC->GetTextExtent( lpszString );
	//	}
	//	return sizeText;
	//}

	SIZE GetTextExtent( LPCWSTR lpszString, CFont *lpFont = NULL , INT nCount = (-1))
	{
		ASSERT( NULL != m_lpDC );

		if ( -1 == nCount )
		{
			nCount = (int)wcslen( lpszString );
		}


		SIZE sizeText;
		if ( NULL != lpFont )
		{
			CFont *lpOldFont;
			lpOldFont = m_lpDC->SelectObject( lpFont );
			::GetTextExtentPointW( m_lpDC->GetSafeHdc(), lpszString, nCount, &sizeText );
			m_lpDC->SelectObject( lpOldFont );
		}
		else
		{
			::GetTextExtentPointW( m_lpDC->GetSafeHdc(), lpszString, nCount, &sizeText );
		}
		return sizeText;
	}

	virtual BOOL ResetAll()
	{
		ResetPen();
		ResetBrush();
		ResetBkMode();
		ResetBkColor();
		ResetTextColor();
		ResetFont();
		delete m_lpDrawDC;
		m_lpDrawDC = NULL;

		return TRUE;
	}

	BOOL SetTextColor( COLORREF crText )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		if ( COLOR_TRANSPARENT == m_dwOriginalTextColor )
		{
			// first set
			m_dwOriginalTextColor = m_lpDC->GetTextColor();
		}

		m_lpDC->SetTextColor( crText );

		return TRUE;
	}

	BOOL ResetTextColor()
	{
		ASSERT_RESULT( NULL != m_lpDC );

		if ( COLOR_TRANSPARENT != m_dwOriginalTextColor )
		{
			m_lpDC->SetTextColor( m_dwOriginalTextColor );
			m_dwOriginalTextColor = COLOR_TRANSPARENT;
		}

		return TRUE;
	}

	BOOL SetBkColor( COLORREF crBack )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		if ( COLOR_TRANSPARENT == m_dwOriginalBkColor )
		{
			// first set
			m_dwOriginalBkColor = m_lpDC->GetBkColor();
		}

		m_lpDC->SetBkColor( crBack );

		return TRUE;
	}

	BOOL ResetBkColor()
	{
		ASSERT_RESULT( NULL != m_lpDC );

		if ( COLOR_TRANSPARENT != m_dwOriginalBkColor )
		{
			m_lpDC->SetBkColor( m_dwOriginalBkColor );
			m_dwOriginalBkColor = COLOR_TRANSPARENT;
		}

		return TRUE;
	}

	BOOL SetBkMode( DWORD dwMode = TRANSPARENT )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		if ( 0 == m_dwOriginalBkMode )
		{
			// first set
			m_dwOriginalBkMode = m_lpDC->GetBkMode();
		}
		
		m_lpDC->SetBkMode( dwMode );

		return TRUE;
	}

	BOOL ResetBkMode()
	{
		ASSERT_RESULT( NULL != m_lpDC );

		if ( 0 != m_dwOriginalBkMode )
		{
			m_lpDC->SetBkMode( m_dwOriginalBkMode );
			m_dwOriginalBkMode = 0;
		}

		return TRUE;
	}

	BOOL SetPen( INT nPanStyle, INT nWidthPen, COLORREF crPen )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		CPen *lpPen = new CPen;
		ASSERT_RESULT( NULL != lpPen );
		lpPen->CreatePen( nPanStyle, nWidthPen, crPen );

		if ( NULL == m_lpOriginalPen )
		{
			// original pen in dc
			m_lpOriginalPen = m_lpDC->SelectObject( lpPen );
		}
		else
		{
			// managed pen in dc, delete old pen
			delete m_lpDC->SelectObject( lpPen );
		}

		return TRUE;
	}

	BOOL SetPen( CPen *lpPen )
	{
		ASSERT_RESULT( NULL != m_lpDC && NULL != lpPen );

		LOGPEN logPen;
		lpPen->GetLogPen( &logPen );

		return SetPen( logPen.lopnStyle, logPen.lopnWidth.x, logPen.lopnColor );
	}

	BOOL ResetPen()
	{
		ASSERT_RESULT( NULL != m_lpDC );

		if ( NULL != m_lpOriginalPen )
		{
			delete m_lpDC->SelectObject( m_lpOriginalPen );
			m_lpOriginalPen = NULL;
		}

		return TRUE;
	}

	BOOL SetBrush( COLORREF crBrush )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		CBrush *lpBrush = new CBrush;
		ASSERT_RESULT( NULL != lpBrush );
		lpBrush->CreateSolidBrush( crBrush );

		if ( NULL == m_lpOriginalBrush )
		{
			m_lpOriginalBrush = m_lpDC->SelectObject( lpBrush );
		}
		else
		{
			delete m_lpDC->SelectObject( lpBrush );
		}

		return TRUE;
	}

	BOOL ResetBrush()
	{
		ASSERT_RESULT( NULL != m_lpDC );

		if ( NULL != m_lpOriginalBrush )
		{
			delete m_lpDC->SelectObject( m_lpOriginalBrush );
			m_lpOriginalBrush = NULL;
		}

		return TRUE;
	}

	BOOL DrawText( LPCWSTR lpszText, LPRECT lprcText, UINT uFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE, CFont *lpFont = NULL )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		if ( NULL == lpFont )
		{
			m_lpDC->DrawText( lpszText, lprcText, uFormat );
		}
		else
		{
			CFont *lpOldFont;
			lpOldFont = m_lpDC->SelectObject( lpFont );
			m_lpDC->DrawText( lpszText, lprcText, uFormat );
			m_lpDC->SelectObject( lpOldFont );
		}

		return TRUE;
	}


	BOOL DrawBitmap( INT x, INT y, CBitmap *lpBitmap, DWORD dwColorKey = NULL_COLORKEY )
	{
		ASSERT_RESULT( NULL != m_lpDC && NULL != lpBitmap && ReadyDrawDC() );

		SIZE sizeBitmap;
		BITMAP bit;
		lpBitmap->GetBitmap ( &bit );

		sizeBitmap.cx = bit.bmWidth;
		sizeBitmap.cy = bit.bmHeight;


		CBitmap *lpOldBitmap = m_lpDrawDC->SelectObject( lpBitmap );


		if ( NULL_COLORKEY == dwColorKey )
		{
			// not to use color key
			m_lpDC->BitBlt( x, y, sizeBitmap.cx, sizeBitmap.cy, m_lpDrawDC, 0, 0, SRCCOPY );
		}
		else
		{
			// use color key
			m_lpDC->TransparentBlt( x, y, sizeBitmap.cx, sizeBitmap.cy, m_lpDrawDC, 0, 0, sizeBitmap.cx, sizeBitmap.cy, dwColorKey );
		}

		m_lpDrawDC->SelectObject( lpOldBitmap );

		return TRUE;
	}

	BOOL DrawBitmap( CBitmap *lpBitmapDes, INT x, INT y, CBitmap *lpBitmapSrc, DWORD dwColorKey = NULL_COLORKEY )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		BOOL bRet;

		CBitmap *lpOldBitmap = m_lpDC->SelectObject( lpBitmapDes );

		bRet = DrawBitmap( x, y, lpBitmapSrc, dwColorKey );

		m_lpDC->SelectObject( lpOldBitmap );

		return bRet;
	}

	BOOL DrawBitmap( INT x, INT y, INT cx, INT cy, CBitmap *lpBitmap,INT xSrc, INT ySrc, INT cSrcWidth, INT cSrcHeight, DWORD dwColorKey = NULL_COLORKEY )
	{
		ASSERT_RESULT( NULL != m_lpDC && NULL != lpBitmap && ReadyDrawDC() );

		CBitmap *lpOldBitmap = m_lpDrawDC->SelectObject( lpBitmap );

		if ( NULL_COLORKEY == dwColorKey )
		{
			// not to use color key
			m_lpDC->StretchBlt( x, y, cx, cy, m_lpDrawDC, xSrc, ySrc, cSrcWidth, cSrcHeight, SRCCOPY );
		}
		else
		{
			m_lpDC->TransparentBlt( x, y, cx, cy, m_lpDrawDC, xSrc, ySrc, cSrcWidth, cSrcHeight, dwColorKey );
		}

		m_lpDrawDC->SelectObject( lpOldBitmap );

		return TRUE;
	}


	BOOL DrawBitmap( INT x, INT y, INT cx, INT cy, CBitmap *lpBitmap, DWORD dwColorKey = NULL_COLORKEY )
	{
		ASSERT_RESULT( NULL != m_lpDC && NULL != lpBitmap && ReadyDrawDC() );

		SIZE sizeBitmap;
		BITMAP bit;
		lpBitmap->GetBitmap ( &bit );

		sizeBitmap.cx = bit.bmWidth;
		sizeBitmap.cy = bit.bmHeight;

		CBitmap *lpOldBitmap = m_lpDrawDC->SelectObject( lpBitmap );

		if ( NULL_COLORKEY == dwColorKey )
		{
			// not to use color key
			m_lpDC->StretchBlt( x, y, cx, cy, m_lpDrawDC, 0, 0, sizeBitmap.cx, sizeBitmap.cy, SRCCOPY );
		}
		else
		{
			m_lpDC->TransparentBlt( x, y, cx, cy, m_lpDrawDC, 0, 0, sizeBitmap.cx, sizeBitmap.cy, dwColorKey );
		}

		m_lpDrawDC->SelectObject( lpOldBitmap );

		return TRUE;
	}

	BOOL DrawBitmap( CBitmap *lpBitmapDes, INT x, INT y, INT cx, INT cy, CBitmap *lpBitmapSrc, DWORD dwColorKey = NULL_COLORKEY )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		BOOL bRet;

		CBitmap *lpOldBitmap = m_lpDC->SelectObject( lpBitmapDes );

		bRet = DrawBitmap( x, y, cx, cy, lpBitmapSrc, dwColorKey );

		m_lpDC->SelectObject( lpOldBitmap );

		return bRet;
	}

	BOOL DrawLine( INT x, INT y, COLORREF crLineColor, INT nWidth = 1, INT x1 = -1, INT y1 = -1 )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		CPen penLine( PS_SOLID, nWidth, crLineColor );
		CPen *lpOldPen = m_lpDC->SelectObject( &penLine );	

		if ( x1 < 0 || y1 < 0 )
		{
			// not move and draw
			m_lpDC->LineTo( x, y );
		}
		else
		{
			m_lpDC->MoveTo( x, y );
			m_lpDC->LineTo( x1, y1 );
		}

		m_lpDC->SelectObject( lpOldPen );

		return TRUE;
	}

	BOOL DrawLine( INT x, INT y, INT x1 = -1, INT y1 = -1 )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		if ( x1 < 0 || y1 < 0 )
		{
			// not move and draw
			m_lpDC->LineTo( x, y );
		}
		else
		{
			m_lpDC->MoveTo( x, y );
			m_lpDC->LineTo( x1, y1 );
		}

		return TRUE;
	}

	BOOL DrawRect( LPRECT lprcRect, DWORD dwColorFrame = COLOR_TRANSPARENT, DWORD dwColorFill = COLOR_TRANSPARENT )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		COLORREF crBackground;
		crBackground = m_lpDC->GetBkColor();

		if ( dwColorFill != COLOR_TRANSPARENT )
		{
			// fill rect
			CBrush brushFill( dwColorFill );
			m_lpDC->FillSolidRect( lprcRect, dwColorFill );
		}

		if ( dwColorFrame != COLOR_TRANSPARENT )
		{
			// draw frame
			m_lpDC->Draw3dRect( lprcRect->left, lprcRect->top, lprcRect->right - lprcRect->left, lprcRect->bottom - lprcRect->top, dwColorFrame, dwColorFrame );
		}

		m_lpDC->SetBkColor( crBackground );

		return TRUE;
	}
	BOOL DrawRoundRect( LPCRECT lprcRect, SIZE sizeRound, DWORD dwColorFrame = COLOR_TRANSPARENT, DWORD dwColorFill = COLOR_TRANSPARENT )
	{
		ASSERT_RESULT( NULL != m_lpDC );

		COLORREF crBackground;
		crBackground = m_lpDC->GetBkColor();

		HRGN hrgnFrame = ::CreateRoundRectRgn( lprcRect->left, lprcRect->top, lprcRect->right + 1, lprcRect->bottom + 1, sizeRound.cx, sizeRound.cy );

		if ( COLOR_TRANSPARENT == dwColorFrame && COLOR_TRANSPARENT != dwColorFill )
		{
			// fill it only			
			CBrush brushFill( dwColorFill );		
			FillRgn(  m_lpDC->GetSafeHdc(), hrgnFrame, brushFill );
		}

		else
		{
			HRGN hrgnFill = ::CreateRoundRectRgn( lprcRect->left + 1, lprcRect->top + 1, lprcRect->right, lprcRect->bottom, sizeRound.cx, sizeRound.cy );
			if ( COLOR_TRANSPARENT != dwColorFrame && COLOR_TRANSPARENT != dwColorFill )
			{
				// fill and frame
				CBrush brushFillFrame( dwColorFrame );	 
				FillRgn(   m_lpDC->GetSafeHdc(),  hrgnFrame, brushFillFrame );

				CBrush brushFill( dwColorFill );
				FillRgn(   m_lpDC->GetSafeHdc(), hrgnFill, brushFill );

			}
			else
			{
				// frame only
				::CombineRgn( hrgnFrame, hrgnFrame, hrgnFill, RGN_DIFF );

				CBrush brushFillFrame( dwColorFrame );	
				FillRgn(   m_lpDC->GetSafeHdc(), hrgnFrame, brushFillFrame );

			}

			::DeleteObject( (HGDIOBJ)hrgnFill );
		}

		::DeleteObject( (HGDIOBJ)hrgnFrame );

		m_lpDC->SetBkColor( crBackground );

		return TRUE;
	}
	// create region with bitmap and color key
	static BOOL CreateValidRgn( OUT CRgn *lpRgn, IN CBitmap *lpBitmap, COLORREF crColorKey )
	{
		ASSERT_RESULT( NULL != lpBitmap && NULL != lpRgn );

		lpRgn->DeleteObject();
		lpRgn->CreateRectRgn( 0, 0, 0, 0 );


		CDC memDC;
		memDC.Attach( ::CreateCompatibleDC( NULL ) );


		CBitmap *pOldMemBmp = NULL;
		pOldMemBmp = memDC.SelectObject( lpBitmap );

		BITMAP bit;
		lpBitmap->GetBitmap ( &bit );

		for( int y = 0; y < bit.bmHeight ; y++ )
		{
			CRgn rgnTemp; //保存临时region

			int nX = 0;
			do
			{
				//跳过透明色找到下一个非透明色的点
				while ( nX < bit.bmWidth && memDC.GetPixel( nX, y ) == crColorKey )
				{
					nX++;
				}

				//记住这个起始点
				int nLeftX = nX;

				//寻找下个透明色的点
				while ( nX < bit.bmWidth && memDC.GetPixel( nX, y ) != crColorKey )
				{
					++nX;
				}

				//创建一个包含起点与重点间高为1像素的临时“region”
				rgnTemp.CreateRectRgn( nLeftX, y, nX, y + 1 );

				//合并到主"region".
				lpRgn->CombineRgn( lpRgn, &rgnTemp, RGN_OR );

				//删除临时"region",否则下次创建时和出错
				rgnTemp.DeleteObject();

			}while( nX < bit.bmWidth );
		}

		memDC.SelectObject( pOldMemBmp );

		return TRUE;
	}

	CDC *GetDC()
	{
		return m_lpDC;
	}


	BOOL SetROP( INT nRop )
	{
		if (  0 == m_nOriginalROP )
		{
			m_nOriginalROP = ::GetROP2( m_lpDC->GetSafeHdc() );
		}

		::SetROP2( m_lpDC->GetSafeHdc(), nRop );

		return TRUE;
	}

	BOOL ResetROP()
	{
		if (  0 != m_nOriginalROP )
		{
			::SetROP2( m_lpDC->GetSafeHdc(), m_nOriginalROP );
		}
		return TRUE;
	}

	BOOL SetClipRect( LPRECT lprcClip )
	{

		HRGN hrgn = ::CreateRectRgn( lprcClip->left, lprcClip->top, lprcClip->right, lprcClip->bottom );
		int ret= ::SelectClipRgn( m_lpDC->GetSafeHdc(), hrgn );
		::DeleteObject( (HGDIOBJ)hrgn );

		return TRUE;
	}

	BOOL SetViewport( INT x, INT y )
	{
		if ( NULL == m_lpptOriginalViewport )
		{
			m_lpptOriginalViewport = new POINT;
			ASSERT_RESULT( NULL != m_lpptOriginalViewport );
			::GetViewportOrgEx( m_lpDC->GetSafeHdc() , m_lpptOriginalViewport );
		}

		return ::SetViewportOrgEx( m_lpDC->GetSafeHdc(), x, y, NULL );
	}

private:
	BOOL ReadyDrawDC()
	{
		ASSERT_RESULT( NULL != m_lpDC );

		if ( NULL != m_lpDrawDC )
		{
			// already created
			return TRUE;
		}

		m_lpDrawDC = new CDC;
		ASSERT_RESULT ( NULL != m_lpDrawDC );
		ASSERT_RESULT( m_lpDrawDC->CreateCompatibleDC( m_lpDC ) );

		return TRUE;
	}

private:
	CDC *m_lpDC;
	CDC *m_lpDrawDC;
	CPen *m_lpOriginalPen;
	CBrush *m_lpOriginalBrush;
	CFont *m_lpOriginalFont;
	DWORD m_dwOriginalBkMode;
	DWORD m_dwOriginalBkColor;
	DWORD m_dwOriginalTextColor;
	INT m_nOriginalROP;
	LPPOINT m_lpptOriginalViewport;
};
