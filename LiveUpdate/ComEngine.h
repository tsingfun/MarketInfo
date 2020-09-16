//####################################################################
//	Comments:	Modify from Original LiveUpdate ver1
//
//	UpdateLogs:	
//####################################################################
#pragma once

#include <wininet.h>
#include "curl/curl.h"
#include <string>
#include <map>
#include <vector>
#include "GlobalDefine.h"


enum _DownloadStatus
{	
	STATUS_INIT,
	STATUS_START,
	STATUS_INI,
	STATUS_SERVER_GROUP_LIST,
	STATUS_SELF,
	STATUS_LOG,
	STATUS_FILES,
	STATUS_INSTALL_PACK,
	STATUS_REPLACE,
	STATUS_CANCEL,
	STATUS_END,// all done
	STATUS_FAIL
};


#define CE_MSG_001					_T("非常抱歉，URL解析出错: %s。")
#define CE_MSG_002					_T("非常抱歉，文件打开错误: %s\nErrCode=%d。")
#define CE_MSG_003					_T("在写入文件的时候出错。")

#define SEND_REQUEST_TIMER_ID		1000
#define SEND_REQUEST_TIMER_PERIOD	500
#define APP_RUN_PROCESS_TIMER_ID	1001
#define APP_RUN_PROCESS_PERIOD		200
#define APP_RUN_PROCESS_TOTAL_STEP  100

#define TEMP_FILE_EXTEND_NAME		_T(".tpf")



struct ProxyInfo
{
	DWORD m_dwProxyType;
	std::string m_strProxy;
	std::string m_strUserNamePsd;
};

class CComEngine
{
private:
	struct DispProcessInfo
	{
		HWND m_hWnd;
		TCHAR m_lpszFileName[MAX_PATH];
		bool m_bHasSetRange;
		long m_lFileStartDownloadIndex;
		HANDLE m_hCancel;
		DispProcessInfo()
			:m_lFileStartDownloadIndex(0),
			m_bHasSetRange(false),
			m_hWnd(NULL),
			m_hCancel(NULL)
		{
			ZeroMemory(m_lpszFileName,sizeof(TCHAR) * MAX_PATH);
		}
	};
	struct WriteDataInfo
	{
		CFile* m_pFile;
		HANDLE m_hCancel;
	};
public:
	// get the only object
	static CComEngine* GetComEngine();
	static void Release();

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Added:		2009/10/20   13:56
	// comments:	增加一个0值，为了ini和log的异步下载成为可能
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//为了改造ini和log文件的下载为异步方式（调用m_pComEngine->BeginDownloadThread）
	//所以要让m_uaIsZip数组不在BeginDownloadThread里访问
	//使用了一个传入的BYTE数组表示Zip与否的列表
	//避免和CComEngine耦合度太高
	// create a new thread to update
	bool BeginDownloadThread(HWND hWnd, int nFileNum, 
		LPCTSTR lpszUrlList, unsigned int nUrlListLen, 
		LPCTSTR lpszDirList, unsigned int nDirListLen, 
		LPBYTE lpbZipList, unsigned int nZipListLen,
		HANDLE hCancel, LPCTSTR lpszUpdatePath, LPVOID param = NULL);
	
	// Http下载指定文件
	static bool Download(HWND hWnd, HANDLE hCancel, LPCTSTR lpszUrl, LPCTSTR lpszFileName, 
		CString& strErrMsg, unsigned long& nFileSize, bool isZip = false, int nTimeOut = 180,  LPVOID pParam = NULL);

	static bool DownloadUseCurl(HWND hWnd, HANDLE hCancel, LPCTSTR lpszUrl, 
															LPCTSTR lpszFileName, CString& strErrMsg, unsigned long& nFileSize,
															bool isZip = false, int nTimeOut = 180,  LPVOID pParam = NULL);
	static void CurlReConnect(CURL* pCurlEasyHandle,HWND hWnd, HANDLE hCancel, LPCTSTR lpszUrl, 
													LPCTSTR lpszFileName, CString& strErrMsg, unsigned long& nFileSize,
													bool isZip, int nTimeOut,  LPVOID pParam,CURLcode& successCode,CFile& file);
	static int ProcessCallbackFunc(void *pClientParam, double dltotal, double dlnow, double ultotal, double ulnow);

	static size_t WriteDataCallbackFunc(void *pBuffer, size_t nDataItemCount, size_t nBufferSize, void *pUserPrama);

	static BOOL SetDownloadProxy();

	static BOOL GetDownloadProxy(ProxyInfo& proxyInfo);

	static BOOL GetProxyFromInternet(ProxyInfo& proxyInfo);

	static BOOL GetProxyFromClient(ProxyInfo& proxyInfo);


	// check if download thread is running
	bool IsDownloading(void);

	// close the downloading thread by force
	void StopDownload(void);

protected:
	CComEngine(void);
	~CComEngine(void);

private:
	struct DLP_PARAM
	{
		bool bSuccess;
		int nFileNum;
		HWND hWnd;
		HANDLE hCancel;
		TCHAR szUrlList[_MAX_PATH * 300];
		unsigned int nUrlListLen;
		TCHAR szDirList[_MAX_PATH * 300];
		unsigned int nDirListLen;
		BYTE bZipList[_MAX_PATH];
		unsigned int nZipListLen;
		TCHAR szUpdatePath[_MAX_PATH];
		LPVOID param;//额外参数

		DLP_PARAM()
		{
			bSuccess = false;
			nFileNum = 0;
			hWnd = NULL;
			hCancel = NULL;
			ZeroMemory(szUrlList, sizeof (TCHAR) * _MAX_PATH * 10);
			nUrlListLen = 0;
			ZeroMemory(szDirList, sizeof (TCHAR) * _MAX_PATH * 10);
			nDirListLen = 0;
			ZeroMemory(szUpdatePath, sizeof (TCHAR) * _MAX_PATH);
			param = NULL;
		};
	};

	static CComEngine* m_pComEngine;
	static int m_nRef;
	static HANDLE m_hThreadRunning;
	enum ClientProxyType
	{
		CLIENT_PROXY_HTTP = 1,
		CLIENT_PROXY_SOCK4,
		CLIENT_PROXY_SOCK5,
	};
	static std::map<DWORD, DWORD> m_mapClientProxyTypeCurlProxyType;
	static std::pair<DWORD, DWORD> m_pairClientProxyTypeCurlProxyType[];

	HANDLE m_hReadFile;
	HANDLE m_hThread;

public:
	DLP_PARAM m_dlpParam;

private:
	// thread func to download the list files user provide.
	static UINT DownloadProc(LPVOID lpParam);

};