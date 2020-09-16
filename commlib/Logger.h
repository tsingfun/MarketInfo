#ifndef __LOGGER_H_
#define __LOGGER_H_

#include <iostream>
#include <atlstr.h>
#include "macro.h"
#pragma warning(disable:4996)

#define LEVEL_FATAL		0
#define LEVEL_ERROR		1
#define LEVEL_WARN		2
#define LEVEL_INFO		3
#define LEVEL_VERBOSE	4
#define LEVEL_DEBUG		5

static int nLoggerLevel = LEVEL_INFO;
COMMON_DLL_EXPORT void SetLoggerLevel(int nLevel);
COMMON_DLL_EXPORT void Log(int nLevel, LPCSTR func, INT line, LPCTSTR fmt, ...);

static const char * const LOGGER_LEVEL_NAME[] = {"±ÀÀ£", "´íÎó", "¾¯¸æ", "ÐÅÏ¢", "ÏêÏ¸", "µ÷ÊÔ"};
static const char * const LOGGER_LEVEL_CODE[] = {"FATAL", "ERROR", "WARN ", "INFO ", "VERBOSE", "DEBUG"};

#define logging(nLevel, x, ...) \
	Log(nLevel,  __FUNCTION__, __LINE__, (x), __VA_ARGS__)

#define LOG_FATAL(x,...)	logging(LEVEL_FATAL,   (x), __VA_ARGS__)
#define LOG_ERROR(x,...)	logging(LEVEL_ERROR,   (x), __VA_ARGS__)
#define LOG_WARN(x,	...)	logging(LEVEL_WARN,    (x), __VA_ARGS__)
#define LOG_INFO(x,	...)	logging(LEVEL_INFO,    (x), __VA_ARGS__)
#define LOG_VERBOSE(x,...)	logging(LEVEL_VERBOSE, (x), __VA_ARGS__)
#define LOG_DEBUG(x,...)	logging(LEVEL_DEBUG,   (x), __VA_ARGS__)

#endif