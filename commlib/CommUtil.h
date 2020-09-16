#ifndef _UTILCOM_H_
#define _UTILCOM_H_

#include "macro.h"
#include "stdafx.h"

// get last error message
COMMON_DLL_EXPORT void GetLastErrorString(CString& strErrMsg);

enum NUM_UNIT_TYPE			// 数字精度
{
	NUM_UNIT_0  = 0,		// 个位数
	NUM_UNIT_WAN,			// 万
	NUM_UNIT_YI,			// 亿
	NUM_UNIT_WANYI,			// 万亿
};

NUM_UNIT_TYPE GetEffectivdNum( double dbNum, TCHAR *pSzChar, int nSize );

// 科学显示数字
COMMON_DLL_EXPORT void GetKXZSString( double dbTmp, CString &strText );

// 小数部分指定位数转化为字符串(bClearZero:末尾是否去零，无小数返回空)
COMMON_DLL_EXPORT void Xiaoshu2String(double d, int n, CString& outStr, bool bClearZero = false);



// 正数界限
#define POSITIVE_LIMIT				0.00001
// 负数界限
#define NEGATIVE_LIMIT				-0.00001

// 是正数
static inline bool IsPositive( double fNum )
{
	return fNum >= POSITIVE_LIMIT;
}

// 是负数
static inline bool IsNegative( double fNum )
{
	return fNum <= NEGATIVE_LIMIT;
}

// 是否为零
static inline bool IsZeroValue( double fNum)
{
	return NEGATIVE_LIMIT < fNum && fNum < POSITIVE_LIMIT;
}

// 根据四舍五入法则，修整浮点数
COMMON_DLL_EXPORT double SiSheWuRuFloatValue(double fNum, int nDecimal);

// 根据四舍五入法则，修整浮点数
COMMON_DLL_EXPORT double SiSheWuRuInt64FloatValue(double fNum, int nDecimal);

#endif // _UTILCOM_H_