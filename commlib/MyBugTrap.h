#include "stdafx.h"

// Windows下感知程序崩溃的处理函数
LONG COMMON_DLL_EXPORT WINAPI MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionPointers);