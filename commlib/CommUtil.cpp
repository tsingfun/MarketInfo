#include "stdafx.h"
#include "CommUtil.h"

void GetLastErrorString(CString& strErrMsg)
{
	LPVOID lpMsgBuf;
	if (!FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL))
	{
		// Handle the error.
		strErrMsg = _T("Cannot get last error string.");
		return;
	}

	strErrMsg.Format(_T("%s"), lpMsgBuf);

	// Free the buffer.
	LocalFree(lpMsgBuf);
}

NUM_UNIT_TYPE GetEffectivdNum( double dbNum, TCHAR *pSzChar, int nSize )
{
	NUM_UNIT_TYPE nUnitType;
	double dbYushu = 0;
	if (dbNum - 999999999999.9999 > 0 || dbNum + 999999999999.9999 < 0)//万亿
	{
		dbYushu = dbNum/1000000000000;
		nUnitType = NUM_UNIT_WANYI;
	}
	else if (dbNum - 99999999.9999 > 0 || dbNum + 99999999.9999 < 0)//亿
	{
		dbYushu = dbNum/100000000;
		nUnitType = NUM_UNIT_YI;
	}
	else if (dbNum - 9999.9999 > 0 || dbNum + 9999.9999 < 0)//万
	{
		dbYushu = dbNum/10000;
		nUnitType = NUM_UNIT_WAN;
	}
	else
	{
		dbYushu = dbNum;
		nUnitType = NUM_UNIT_0;
	}
	if (nUnitType == NUM_UNIT_0 || IsZeroValue(dbYushu))
		swprintf( pSzChar, nSize, _T("%.0f"), dbYushu);
	else if (dbYushu - 99.9999 > 0 || dbYushu + 99.9999 < 0)
		swprintf( pSzChar, nSize, _T("%.1f"), dbYushu);
	else if (dbYushu - 9.9999 > 0 || dbYushu + 9.9999 < 0)
		swprintf( pSzChar, nSize, _T("%.2f"), dbYushu);		
	else
		swprintf( pSzChar, nSize, _T("%.2f"), dbYushu);

	return nUnitType;
}

void GetKXZSString( double dbTmp, CString &strText )
{
	TCHAR szTChar[32] = {0};
	NUM_UNIT_TYPE numType;
	numType = GetEffectivdNum( dbTmp, szTChar, 32 );
	strText = szTChar;
	switch ( numType )
	{
	case NUM_UNIT_WAN:
		strText += L"万";
		break;
	case NUM_UNIT_YI:
		strText += L"亿";
		break;
	case NUM_UNIT_WANYI:
		strText += L"万亿";
		break;
	}
}

void Xiaoshu2String(double d, int n, CString& outStr, bool bClearZero /*= false*/)
{
	const double fFixValue = 1e-6;
	switch (n)
	{
	case 0:
		d += IsPositive (d) ? 0.5 + fFixValue : -0.5 - fFixValue;
		break;
	case 1:
		d += IsPositive( d ) ? 0.05 + fFixValue : - 0.05 - fFixValue;
		break;
	case 2:
		d += IsPositive( d ) ? 0.005 + fFixValue: - 0.005 - fFixValue;
		break;
	case 3:
		d += IsPositive( d ) ? 0.0005 + fFixValue: - 0.0005 - fFixValue;
		break;
	case 4:
		d += IsPositive( d ) ? 0.00005 + fFixValue: - 0.00005 - fFixValue;
		break;
	default:
		break;
	}

	double xs = d - (int)d;
	CString tmp;
	for (int i = 0; i < n; i++)
	{
		xs *= 10;
		tmp.Format(_T("%s%d"), tmp, (int)xs);
		xs = xs - (int)xs;
	}

	// 去掉小数点后末尾的0
	if (bClearZero && !tmp.IsEmpty())
	{
		while (tmp.Mid(tmp.GetLength() - 1, tmp.GetLength()).Compare(_T("0")) == 0)
			tmp = tmp.Mid(0, tmp.GetLength() - 1);
	}
	
	outStr = tmp;
}

// 根据四舍五入法则，修整浮点数
double SiSheWuRuFloatValue(double fNum, int nDecimal)
{
	if ( IsZeroValue( fNum ) ) 
		return 0;

	if ( nDecimal < 0 )
	{
		// 直接保留整数			
		return (double)(int)fNum;
	}

	//处理 0.14499999 表示 0.145 时，2精度四舍五入成0.14的问题（理论是0.15）
	const double fFixValue = 1e-6;

	switch (nDecimal)
	{
	case 0:
		return (double)((int)((fNum + ( IsPositive (fNum) ? 0.5 + fFixValue : -0.5 - fFixValue )) * 1.0) / 1.0);
		break;
	case 1:
		return (double)((int)((fNum + ( IsPositive( fNum ) ? 0.05 + fFixValue : - 0.05 - fFixValue)) * 10.0) / 10.0);
		break;
	case 2:
		return (double)((int)((fNum + ( IsPositive( fNum ) ? 0.005 + fFixValue: - 0.005 - fFixValue)) * 100.0) / 100.0);
		break;
	case 3:
		return (double)((int)((fNum + ( IsPositive( fNum ) ? 0.0005 + fFixValue: - 0.0005 - fFixValue)) * 1000.0) / 1000.0);
		break;
	case 4:
		return (double)((int)((fNum + ( IsPositive( fNum ) ? 0.00005 + fFixValue: - 0.00005 - fFixValue)) * 10000.0) / 10000.0);
		break;
	default:
		// 必须传入此参数
		ASSERT( false );
		return (double)((int)((fNum + ( IsPositive( fNum ) ? 0.005 + fFixValue: - 0.005 - fFixValue)) * 100.0) / 100.0);
		break;
	}

	return fNum;
}

// 根据四舍五入法则，修整浮点数
double SiSheWuRuInt64FloatValue(double fNum, int nDecimal)
{
	if ( IsZeroValue( fNum ) ) 
		return 0;

	if ( nDecimal < 0 )
	{
		// 直接保留整数			
		return (double)(__int64)fNum;
	}

	//处理 0.14499999 表示 0.145 时，2精度四舍五入成0.14的问题（理论是0.15）
	const double fFixValue = 1e-12;

	switch (nDecimal)
	{
	case 0:
		return (double)((__int64)((fNum + ( IsPositive (fNum) ? 0.5 + fFixValue: -0.5 - fFixValue)) * 1.0) / 1.0);
		break;
	case 1:
		return (double)((__int64)((fNum + ( IsPositive( fNum ) ? 0.05 + fFixValue: - 0.05 - fFixValue)) * 10.0) / 10.0);
		break;
	case 2:
		return (double)((__int64)((fNum + ( IsPositive( fNum ) ? 0.005 + fFixValue: - 0.005 - fFixValue)) * 100.0) / 100.0);
		break;
	case 3:
		return (double)((__int64)((fNum + ( IsPositive( fNum ) ? 0.0005 + fFixValue: - 0.0005 - fFixValue )) * 1000.0) / 1000.0);
		break;
	case 4:
		return (double)((__int64)((fNum + ( IsPositive( fNum ) ? 0.00005 + fFixValue: - 0.00005 - fFixValue)) * 10000.0) / 10000.0);
		break;
	default:
		// 必须传入此参数
		ASSERT( false );
		return (double)((__int64)((fNum + ( IsPositive( fNum ) ? 0.005 + fFixValue: - 0.005 - fFixValue)) * 100.0) / 100.0);
		break;
	}

	return fNum;
}