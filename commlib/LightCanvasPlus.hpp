#pragma once

#include "LightCanvas.hpp"
#include <GdiPlus.h>

typedef Gdiplus::Bitmap CBitmapPlus;

//////////////////////////////////////////////////////////////////////////
// for GDI plus
class CLightCanvasPlus : public CLightCanvas
{
public:
	CLightCanvasPlus( CDC *lpDC ): CLightCanvas( lpDC ){ m_lpDCPlus = NULL; }
	virtual ~CLightCanvasPlus(){ ResetAll(); delete m_lpDCPlus; }


	virtual BOOL ResetAll()
	{
		return __super::ResetAll();
	}

	BOOL DrawBitmap( INT x, INT y, CBitmap *lpBitmap, DWORD dwColorKey = NULL_COLORKEY )
	{
		return __super::DrawBitmap( x, y, lpBitmap, dwColorKey);
	}

	BOOL DrawBitmap( INT x, INT y, INT cx, INT cy, CBitmap *lpBitmap, DWORD dwColorKey = NULL_COLORKEY )
	{
		return __super::DrawBitmap( x, y, cx, cy, lpBitmap, dwColorKey);
	}

	BOOL DrawBitmap( INT x, INT y, CBitmapPlus *lpBitmapPlus, DWORD dwColorKey = NULL_COLORKEY )
	{
		ASSERT_RESULT( NULL != lpBitmapPlus && ReadyDCPlus() );

		if ( NULL_COLORKEY == dwColorKey )
		{
			GetPlusDC()->DrawImage( lpBitmapPlus, x, y );
		}
		else
		{
			Gdiplus::ImageAttributes ia;
			ia.SetColorKey( dwColorKey, dwColorKey );


			Gdiplus::Rect rcDes( x, y, lpBitmapPlus->GetWidth(), lpBitmapPlus->GetHeight() );

			DWORD dwStatus = GetPlusDC()->DrawImage( lpBitmapPlus, rcDes, 0, 0, lpBitmapPlus->GetWidth(), lpBitmapPlus->GetHeight(), Gdiplus::UnitPixel, &ia );
			CHECK_RESULT( Gdiplus::Ok == dwStatus );
		}

		return TRUE;
	}

	BOOL DrawBitmap( INT x, INT y, INT cx, INT cy, CBitmapPlus *lpBitmapPlus, DWORD dwColorKey = NULL_COLORKEY )
	{
		ASSERT_RESULT( NULL != lpBitmapPlus && ReadyDCPlus() );

		if ( NULL_COLORKEY == dwColorKey )
		{
			GetPlusDC()->DrawImage( lpBitmapPlus, x, y, cx, cy );
		}
		else
		{
			Gdiplus::ImageAttributes ia;
			ia.SetColorKey( dwColorKey, dwColorKey );

			Gdiplus::Rect rcDes( x, y, cx, cy );

			DWORD dwStatus = GetPlusDC()->DrawImage( lpBitmapPlus, rcDes, 0, 0, lpBitmapPlus->GetWidth(), lpBitmapPlus->GetHeight(), Gdiplus::UnitPixel, &ia );
			CHECK_RESULT( Gdiplus::Ok == dwStatus );
		}

		return TRUE;
	}

	BOOL ReadyDCPlus()
	{
		if ( NULL == m_lpDCPlus )
		{
			m_lpDCPlus = Gdiplus::Graphics::FromHDC( GetDC()->GetSafeHdc() );
			ASSERT_RESULT( NULL != m_lpDCPlus );
		}

		return TRUE;
	}

	Gdiplus::Graphics *GetPlusDC()
	{
		ASSERT_RESULT_R(  ReadyDCPlus(), NULL );
		return m_lpDCPlus;
	}

	static BOOL CreateValidRgn( OUT CRgn *lpRgn, IN CBitmapPlus *lpBitmapPlus, COLORREF crColorKey )
	{
		ASSERT_RESULT( NULL != lpBitmapPlus && NULL != lpRgn );

		CBitmap bitmap;
		HBITMAP hBitmap = NULL;

		// get handle of bitmap
		lpBitmapPlus->GetHBITMAP( crColorKey, &hBitmap );

		ASSERT_RESULT( NULL != hBitmap );

		// attach with bitmap handle
		bitmap.Attach( hBitmap );

		// create region by base function
		BOOL bResult = __super::CreateValidRgn( lpRgn, &bitmap, crColorKey );

		bitmap.Detach();

		return bResult;
	}

private:
	Gdiplus::Graphics *m_lpDCPlus;
};
