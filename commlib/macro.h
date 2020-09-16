#pragma once

#include <assert.h>

#ifdef COMMON_EXPORTS
#define COMMON_DLL_EXPORT __declspec(dllexport)
#else
#define COMMON_DLL_EXPORT __declspec(dllimport)
#endif

#ifdef _DEBUG_MSG 

#define JIF(x) if (FAILED(hr=(x))) \
	{Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n\0"), hr); return FALSE;}

#define LIF(x) if (FAILED(hr=(x))) \
	{Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n\0"), hr);}

#else

#define JIF(x) if (FAILED(hr=(x))) \
	{return FALSE;}

#define LIF(x) if (FAILED(hr=(x))) \
	{NULL;}

#endif

#ifndef CHECK_RESULT

	#define CHECK_RESULT( x )		\
		if ( !( x ) )				\
		{							\
			return FALSE;			\
		}

#endif

#ifdef _DEBUG

#ifndef ASSERT_RESULT

	#define ASSERT_RESULT( x )		\
		if ( !( x ) )				\
		{							\
			/*MessageBoxA( GetActiveWindow(), __FILE__, "Error", MB_OK );*/\
			__asm {\
					int 3\
				  }\
			assert( FALSE );		\
			return FALSE;			\
		}

#endif

#else	// _DEBUG

	#define ASSERT_RESULT( x )		\
		if ( !( x ) )				\
		{							\
		/*char buf[ 1024 ];			\
		itoa( __LINE__, buf, 10 );	\
		MessageBoxA( NULL, __FILE__, "Error", MB_OK );\
		MessageBoxA( NULL, buf, "Error", MB_OK );*/\
			return FALSE;			\
		}

#endif // _DEBUG




#ifndef CHECK_RESULT_R

	#define CHECK_RESULT_R( x, r )		\
		if ( !( x ) )				\
		{							\
		return r;			\
		}

	#endif

#ifndef ASSERT_RESULT_R

	#ifdef _DEBUG

		#define ASSERT_RESULT_R( x, r )		\
			if ( !( x ) )				\
			{							\
			__asm {\
			int 3\
					  }\
			assert( FALSE );		\
			return r;			\
			}
	#else
	#define ASSERT_RESULT_R( x, r )		\
		if ( !( x ) )				\
			{							\
					  return r;			\
			}
	#endif
#endif


#ifdef _DEBUG
	#define NOTHROW 
#else
	#define NOTHROW (std::nothrow)
#endif


#ifndef SAFEDELETE
#define SAFEDELETE(p) if(p){delete p; p=NULL;}
#endif

#ifndef SAFEFREE
#define SAFEFREE(p) if(p){free(p); p=NULL;}
#endif

#ifndef SAFEDELETEARR
#define SAFEDELETEARR(p) if(p){delete [] p; p=NULL;}	// 安全删除数组
#endif


#ifdef TEST_DATACENTER_IO
// 定义TickCount
#define DECLARE_TICK_COUNT					CTickCount __TickCount; __TickCount.Begin();
// 输出TickCount
#define OUTPUT_TICK_COUNT( NAME ) \
	WriteLogW(L"[DataCenterIO]", L"%s, Ticks = %d us\r\n", L#NAME, __TickCount.End() );

#define RESET_TICK_COUNT					__TickCount.Begin();
	
#else

#define DECLARE_TICK_COUNT					
#define OUTPUT_TICK_COUNT( NAME ) 
#define RESET_TICK_COUNT
#endif