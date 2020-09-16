//####################################################################
//	Comments:	Modify from Original LiveUpdate ver1
//
//	UpdateLogs:	
//####################################################################
#include "StdAfx.h"
#include "LiveUpdate.h"
#include "LiveUpdateDlg.h"
#include "comengine.h"
#include <afxinet.h>
#include <io.h>
#include "LogTracer.h"
#include "Logger.h"
#include <ImageHlp.h>
#include "curl/types.h"
#include "curl/easy.h"
#include "Markup.h"
#include "CommUtil.h"
#pragma comment ( lib, "ws2_32.lib" )
#pragma comment ( lib, "wldap32.lib" )
#pragma comment ( lib, "winmm.lib" )
#pragma comment ( lib, "imagehlp.lib" )

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif


CComEngine* CComEngine::m_pComEngine = NULL;
int CComEngine::m_nRef = 0;
HANDLE CComEngine::m_hThreadRunning = NULL;

const TCHAR STR_PROXY_FILE_NAME[] = _T("UpdateDownloadProxy.ini");
const TCHAR STR_UPDATE_PROXY_SEC[] = _T("UpdateProxy");
const TCHAR STR_UPDATE_PROXY_ADDR[] = _T("UpdateProxyAddr");
const TCHAR STR_UPDATE_PROXY_PORT[] = _T("UpdateProxyPort");

//用户设置代理的路径
#define STR_USER_PROXY_PATH _T(".\\config\\LogonUserSetProxy.xml")
///代理服务器根节点名字
#define STR_PROXY_ROOT_NODE_NAME _T("Proxy")
//用户设置代理服务器域名或IP节点，是STR_PROXY_ROOT_NAME的子节点
#define STR_PROXY_TYPE_NODE_NAME _T("Type")
//用户设置代理服务器域名或IP节点，是STR_PROXY_ROOT_NAME的子节点
#define STR_PROXY_SERVER_NODE_NAME _T("Server")
//用户设置代理服务器域名或IP节点，是STR_PROXY_ROOT_NAME的子节点
#define STR_PROXY_PORT_NODE_NAME _T("Port")
//用户设置代理服务器域名或IP节点，是STR_PROXY_ROOT_NAME的子节点
#define STR_PROXY_USER_NAME_NODE_NAME _T("Name")
//用户设置代理服务器域名或IP节点，是STR_PROXY_ROOT_NAME的子节点
#define STR_PROXY_PASSWORD_NODE_NAME _T("Password")

std::map<DWORD, DWORD> CComEngine::m_mapClientProxyTypeCurlProxyType;
std::pair<DWORD, DWORD> CComEngine::m_pairClientProxyTypeCurlProxyType[] = 
{
	make_pair(CLIENT_PROXY_HTTP, CURLPROXY_HTTP),
	make_pair(CLIENT_PROXY_SOCK4, CURLPROXY_SOCKS4),
	make_pair(CLIENT_PROXY_SOCK5, CURLPROXY_SOCKS5),
};

// get the only object
CComEngine* CComEngine::GetComEngine()
{
	if (m_pComEngine == NULL)
	{
		m_pComEngine = new CComEngine();
	}
	m_nRef++;
	return m_pComEngine;
}

// release the reference
void CComEngine::Release()
{
	if (m_nRef > 0)
	{
		m_nRef--;
	}
	if (m_nRef == 0 && m_pComEngine != NULL)
	{
        delete m_pComEngine;
		m_pComEngine = NULL;
	}
}

// create a new thread to update
bool CComEngine::BeginDownloadThread(HWND hWnd, int nFileNum, 
																			 LPCTSTR lpszUrlList, unsigned int nUrlListLen, 
																			 LPCTSTR lpszDirList, unsigned int nDirListLen,
																			 LPBYTE lpbZipList, unsigned int nZipListLen,
																			 HANDLE hCancel,
																			 LPCTSTR lpszUpdatePath,
																			 LPVOID pParam)
{
	LOG_TRACER();
	LOG_INFO(_T("BeginDownloadThread开始"));
	// copy parameters for new thread usage
	RtlZeroMemory(&m_dlpParam,sizeof(m_dlpParam));
	m_dlpParam.nFileNum = nFileNum;
	m_dlpParam.hWnd = hWnd;
	m_dlpParam.hCancel = hCancel;

	memcpy(m_dlpParam.szUrlList, lpszUrlList, sizeof (TCHAR) * nUrlListLen);
	m_dlpParam.nUrlListLen = nUrlListLen;

	memcpy(m_dlpParam.szDirList, lpszDirList, sizeof (TCHAR) * nDirListLen);
	m_dlpParam.nDirListLen = nDirListLen;

	lstrcpy(m_dlpParam.szUpdatePath, lpszUpdatePath);

	memcpy(m_dlpParam.bZipList,lpbZipList,nZipListLen);
	m_dlpParam.nZipListLen = nZipListLen;

	m_dlpParam.param = pParam;//增加一个可选参数

	//::PostMessage(hWnd, WM_DOWNLOAD_START, (WPARAM)pParam, 0);

	// Thread will run
	::SetEvent(m_hThreadRunning);

	//Spin off the background thread which will do the actual downloading
	LOG_INFO(_T("创建下载线程"));
	DWORD dwThreadId;
	m_hThread = ::CreateThread( NULL,									// default security attributes 
														0,										// use default stack size  
														(LPTHREAD_START_ROUTINE)DownloadProc,   // thread function 
														&m_dlpParam,							// argument to thread function 
														0,										// use default creation flags 
														&dwThreadId);							// returns the thread identifier 

	// Check the return value for success. 
	if (m_hThread == NULL) 
	{
		LOG_ERROR(_T("下载线程创建失败"));
		::AfxMessageBox(_T("failed to create download thread!\n"));
		::ResetEvent(m_hThreadRunning);
		return false;
	}

	//::PostMessage(hWnd, WM_DOWNLOAD_END, (WPARAM)pParam, 0);

	return true;
}

// Http下载指定文件
bool CComEngine::Download(HWND hWnd, HANDLE hCancel, LPCTSTR lpszUrl, 
													LPCTSTR lpszFileName, CString& strErrMsg, 
													unsigned long& nFileSize, bool isZip, int nTimeOut, LPVOID pParam)
{
	LOG_TRACER() ;
	LOG_INFO(_T("Download开始"));
	DWORD dwServiceType;
	INTERNET_PORT nPort;
	CString strServer, strObject;
	CFile file;
	CFileException fileException;
	strErrMsg.Empty();
	nFileSize = 0;
	long lFileStartDownloadIndex = 0;
	if (pParam)
	{
		lFileStartDownloadIndex = *((long*)pParam);
	}
	CString strFileName(lpszFileName);
	if (-1 != strFileName.Find(TEMP_FILE_EXTEND_NAME))
	{
		strFileName = strFileName.Mid(0,strFileName.GetLength() - 4);
	}
	HINTERNET hInternetSession = NULL;
	HINTERNET hHttpConnection = NULL;
	HINTERNET hHttpFile = NULL;
	// parse url
	if (!::AfxParseURL(lpszUrl, dwServiceType, strServer, strObject, nPort))
	{
		GetLastErrorString(strErrMsg);
		LOG_ERROR(_T("AfxParseURL失败:%s"), strErrMsg);
		strErrMsg.Format(CE_MSG_001, lpszUrl);
		return false;
	}
	LOG_INFO(_T("AfxParseURL成功"));
	// create the Internet session handle
	ASSERT(hInternetSession == NULL);
	hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternetSession == NULL)
	{
		GetLastErrorString(strErrMsg);
		LOG_ERROR(_T("InternetOpen失败，Error：%s"), strErrMsg);
		return false;
	}
	LOG_INFO(_T("InternetOpen成功"));
	// should we exit the thread
	if (::WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
	{
		LOG_INFO(_T("收到线程退出的要求，退出线程"));
		return true;
	}

	//Make the connection to the HTTP server          
	ASSERT(hHttpConnection == NULL);
	hHttpConnection = ::InternetConnect(hInternetSession, strServer, nPort, NULL, NULL, 
																	dwServiceType, 0, (DWORD_PTR)CWnd::FromHandle(hWnd));
	if (hHttpConnection == NULL)
	{
		GetLastErrorString(strErrMsg);
		LOG_ERROR(_T("InternetConnect失败，Error：%s"), strErrMsg);
		return false;
	}
	LOG_INFO(_T("InternetConnect成功"));

	// should we exit the thread
	if (::WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
	{
		LOG_INFO(_T("收到线程退出的要求，退出线程"));
		return true;
	}
	//Issue the request to read the file
	LPCTSTR ppszAcceptTypes[2];
	ppszAcceptTypes[0] = _T("*/*");  //We support accepting any mime file type since this is a simple download of a file
	ppszAcceptTypes[1] = NULL;
	ASSERT(hHttpFile == NULL);
	hHttpFile = HttpOpenRequest(hHttpConnection, NULL, strObject, NULL, NULL, ppszAcceptTypes, 
														INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE 
														| INTERNET_FLAG_KEEP_CONNECTION, 
														(DWORD_PTR)CWnd::FromHandle(hWnd));
	if (hHttpFile == NULL)
	{
		GetLastErrorString(strErrMsg);
		LOG_ERROR(_T("HttpOpenRequest失败，Error：%s"), strErrMsg);
		return false;
	}
	LOG_INFO(_T("HttpOpenRequest成功"));
	// should we exit the thread
	if (::WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
	{
		LOG_INFO(_T("收到线程退出的要求，退出线程"));
		return true;
	}
	BOOL bSend = FALSE;
	static bool bHasSettedTimer;
	bHasSettedTimer = false;
	while(true)
	{
		if (!bHasSettedTimer)
		{
			if (hWnd)
				SetTimer(hWnd,SEND_REQUEST_TIMER_ID,SEND_REQUEST_TIMER_PERIOD,NULL);
			bHasSettedTimer = true;
		}
		bSend = ::HttpSendRequest(hHttpFile, NULL, 0, NULL, 0);
		LOG_INFO(_T("不断发送http请求，直到网络连接成功退出发送http请求"));
		if (bSend)
		{	
			LOG_INFO(_T("网络连接成功，退出发送http请求"));
			if (hWnd)
			{
				KillTimer(hWnd,SEND_REQUEST_TIMER_ID);
				SendMessage(hWnd,WM_INTERNET_REQUEST_END,NULL,NULL);
			}
			break;
		}
		if (::WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
		{
			// receive command to stop downloading
			LOG_ERROR(_T("用户强制按终止按钮终止更新，退出发送http请求"));
			LOG_INFO(_T("收到线程退出的要求，退出线程"));
			break;
		}
	}
	LOG_INFO(_T("HttpSendRequest成功"));
	//Check the HTTP status code
	TCHAR szStatusCode[32];
	DWORD dwInfoSize = 32;
	if (!::HttpQueryInfo(hHttpFile, HTTP_QUERY_STATUS_CODE, szStatusCode, &dwInfoSize, NULL))
	{
		strErrMsg.Format(_T("下载升级文件失败，升级中止! [%s]"), lpszUrl);
		GetLastErrorString(strErrMsg);
		LOG_ERROR(_T("HttpQueryInfo失败，Error：%s"), strErrMsg);
		return false;
	}
	else
	{
		long nStatusCode = _ttol(szStatusCode);
		if (nStatusCode != HTTP_STATUS_OK)
		{	
			strErrMsg.Format(_T("无法下载升级文件，升级中止! [%s]"), lpszUrl);
			LOG_ERROR(_T("HttpQueryInfo失败，返回code：%s"), nStatusCode);
			GetLastErrorString(strErrMsg);
			LOG_ERROR(_T("Error：%s"), strErrMsg);
			return false;
		}
	}
	LOG_INFO(_T("HttpQueryInfo QUERY_STATUS_CODE 成功"));
	// Get the length of the file.            
	TCHAR szContentLength[32];
	dwInfoSize = 32;
	DWORD dwFileSize = 0;
	BOOL bGotFileSize = FALSE;
	if ((hWnd != NULL) && 
		::HttpQueryInfo(hHttpFile, HTTP_QUERY_CONTENT_LENGTH, szContentLength, &dwInfoSize, NULL))
	{
		//Set the progress control range
		bGotFileSize = TRUE;
		dwFileSize = (DWORD) _ttol(szContentLength);
		LOG_INFO(_T("发送WM_PROGRESS_SET_RANGE，dwFileSize = %d, lpszFileName=%s"), dwFileSize, strFileName);
		::SendMessage(hWnd, WM_PROGRESS_SET_RANGE, (WPARAM)dwFileSize, (LPARAM)strFileName.GetBuffer());
		strFileName.ReleaseBuffer();
	}
	LOG_INFO(_T("HttpQueryInfo QUERY_STATUS_CONTENT_LENGTH 成功"));
	if (0 < lFileStartDownloadIndex)
	{
		CString strHead(L"");
		strHead.Format(L"%s%d%s",L"Range:bytes=",lFileStartDownloadIndex,L"-");
		if (hHttpFile)
		{
			LOG_INFO(_T("InternetCloseHandle(hHttpFile)"));
			::InternetCloseHandle(hHttpFile);
			hHttpFile = NULL;
		}
		hHttpFile = ::InternetOpenUrl(hInternetSession,lpszUrl,strHead,strHead.GetLength(),INTERNET_FLAG_RELOAD,1);
		if (!hHttpFile)
		{
			strErrMsg.Format(_T("下载时网络出现错误，升级中断!"));
			GetLastErrorString(strErrMsg);
			LOG_ERROR(_T("HttpSendRequest失败，Error：%s"), strErrMsg);
			return false;
		}
		if (::WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
		{
			LOG_INFO(_T("收到线程退出的要求，退出线程"));
			return true;
		}
		LOG_INFO(_T("取得断点续传文件成功"));
	}
	// open local file to write data download from server
	if (!file.Open (lpszFileName, CFile::modeReadWrite, &fileException))
	{
		if (!file.Open(lpszFileName, CFile::modeCreate | CFile::modeReadWrite, &fileException))
		{
			strErrMsg.Format(CE_MSG_002, lpszFileName, fileException.m_cause);
			LOG_ERROR(strErrMsg);
			GetLastErrorString(strErrMsg);
			LOG_ERROR(_T("打开本地文件失败，Error：%s"), strErrMsg);
			return false;
		}
	}
	file.Seek(lFileStartDownloadIndex,CFile::begin);
	LOG_INFO(_T("打开本地文件%s 成功"), lpszFileName);
	//Now do the actual read of the file
	DWORD dwStartTicks = ::GetTickCount();
	DWORD dwCurrentTicks = dwStartTicks;
	DWORD dwBytesRead = 0;
	char szReadBuf[1024];
	DWORD dwBytesToRead = 1024;
	DWORD dwTotalBytesRead = 0;
	DWORD dwLastTotalBytes = 0;
	DWORD dwLastPercentage = 0;
	do
	{
		if (::WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
		{
			// receive command to stop downloading
			LOG_INFO(_T("收到线程退出的要求，退出线程"));
			return true;
		}
		BOOL bReadSucceed = FALSE;
		bHasSettedTimer = false;
		if(!(bReadSucceed = ::InternetReadFile(hHttpFile, szReadBuf, dwBytesToRead, &dwBytesRead)))
		{
			LOG_ERROR(_T("下载过程中发生网络中断"));
			while(true)
			{
				if (!bHasSettedTimer && hWnd)
				{
					SetTimer(hWnd,SEND_REQUEST_TIMER_ID,SEND_REQUEST_TIMER_PERIOD,NULL);
					bHasSettedTimer = true;
				}
				bSend = ::HttpSendRequest(hHttpFile, NULL, 0, NULL, 0);
				LOG_ERROR(_T("不断发送http请求，直到网络连接成功退出发送http请求"));
				if (bSend && hWnd)
				{	
					KillTimer(hWnd,SEND_REQUEST_TIMER_ID);
					SendMessage(hWnd,WM_INTERNET_REQUEST_END,NULL,NULL);
					LOG_ERROR(_T("网络连接成功，退出发送http请求"));
					break;
				}
				if (::WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
				{
					// receive command to stop downloading
					LOG_ERROR(_T("用户强制按终止按钮终止更新，退出发送http请求"));
					LOG_INFO(_T("收到线程退出的要求，退出线程"));
					break;
				}
			}
			CString strHead(L"");
			long lRestartDownloadIndex = static_cast<long>(file.GetLength());
			LOG_ERROR(_T("取得文件断点续传的位置lRestartDownloadIndex为：%s"), lRestartDownloadIndex);
			strHead.Format(L"%s%d%s",L"Range:bytes=",lRestartDownloadIndex,L"-");
			if (hHttpFile)
			{
				LOG_INFO(_T("InternetCloseHandle(hHttpFile)"));
				::InternetCloseHandle(hHttpFile);
				hHttpFile = NULL;
			}
			LOG_ERROR(_T("利用断点续传机制，重新取得httpFile，也就是文件还需要下载的部分"));
			hHttpFile = ::InternetOpenUrl(hInternetSession,lpszUrl,strHead,strHead.GetLength(),INTERNET_FLAG_RELOAD,1);
			if (hHttpFile)
			{
				::InternetReadFile(hHttpFile, szReadBuf, dwBytesToRead, &dwBytesRead);
			}
		}
		if (dwBytesRead != 0)
		{
			try
			{
				file.Write(szReadBuf, dwBytesRead);
				nFileSize += dwBytesRead;
			}
			catch (CFileException* pEx)
			{
				strErrMsg = CE_MSG_003;
				pEx->Delete();
				LOG_ERROR(_T("写入本地文件失败，CFileException，Error：%s"), strErrMsg);
				return false;
			}
		}
		//Increment the total number of bytes read
		dwTotalBytesRead += dwBytesRead; 

		if (hWnd != NULL)
		{
			::SendMessage(hWnd, WM_PROGRESS_SET_POS, (WPARAM)dwTotalBytesRead + lFileStartDownloadIndex, (LPARAM)lpszFileName);
		}
	} while (dwBytesRead != 0);
	file.Close();
	try
	{
		CFile::Rename(lpszFileName,strFileName);
	}
	catch (CFileException* e)
	{
		strErrMsg = L"下载的文件由临时文件名重命名为最终文件名失败";
		e->Delete();
		LOG_ERROR(_T("重命名%s为%s失败"), lpszFileName, strFileName);
		return false;
	}
	LOG_INFO(_T("下载完毕，关闭本地文件"));
	//Free up the internet handles we may be using
	if (hHttpFile)
	{
		LOG_INFO(_T("InternetCloseHandle(hHttpFile)"));
		::InternetCloseHandle(hHttpFile);
		hHttpFile = NULL;
	}
	if (hHttpConnection)
	{
		LOG_INFO(_T("InternetCloseHandle(hHttpConnection)"));
		::InternetCloseHandle(hHttpConnection);
		hHttpConnection = NULL;
	}
	if (hInternetSession)
	{
		LOG_INFO(_T("InternetCloseHandle(hInternetSession)"));
		::InternetCloseHandle(hInternetSession);
		hInternetSession = NULL;
	}
	return true;
}

//这个函数是为了符合CURLOPT_PROGRESSFUNCTION而构造的
//显示文件传输进度，t代表文件大小，d代表传输已经完成部分
int CComEngine::ProcessCallbackFunc(void *pClientParam, double dltotal, double dlnow, double ultotal, double ulnow)

{
	DispProcessInfo* pDispProcessInfo = (DispProcessInfo*)pClientParam;
	if (!pClientParam)
	{
		return 0;
	}
	LOG_TRACER() ;
	if (::WaitForSingleObject(pDispProcessInfo->m_hCancel, 0) == WAIT_OBJECT_0)
	{
		// receive command to stop downloading
		LOG_INFO(_T("收到线程退出的要求，退出线程"));
		return 0;
	}
	if (!pDispProcessInfo->m_bHasSetRange && 0 < dltotal)
	{
		::SendNotifyMessage(pDispProcessInfo->m_hWnd, WM_PROGRESS_SET_RANGE, 
									(WPARAM)(dltotal + pDispProcessInfo->m_lFileStartDownloadIndex), 
									(LPARAM)pDispProcessInfo->m_lpszFileName);
		pDispProcessInfo->m_bHasSetRange = true;
	}
	static double dPre = 0;
	if (dPre != dlnow)
	{
		long lFileDownloadedSize = static_cast<long>(dlnow + pDispProcessInfo->m_lFileStartDownloadIndex);
		/*LOG_INFO(_T("发送WM_PROGRESS_SET_POS，lFileDownloadedSize = ")
							<<lFileDownloadedSize<<_T(", lpszFileName = ")<<CS2TS(pDispProcessInfo->m_lpszFileName));*/
		::SendNotifyMessage(pDispProcessInfo->m_hWnd, WM_PROGRESS_SET_POS, 
									(WPARAM)(lFileDownloadedSize), 
									(LPARAM)pDispProcessInfo->m_lpszFileName);
		dPre = dlnow;
	}
	return 0;
}

size_t CComEngine::WriteDataCallbackFunc(void *pBuffer, size_t nDataItemCount, size_t nBufferSize, void *pUserPrama)

{
	WriteDataInfo* writeDataInfo = (WriteDataInfo*)pUserPrama;
	if (!writeDataInfo)
	{
		return 0;
	}
	LOG_TRACER() ;
	if (::WaitForSingleObject(writeDataInfo->m_hCancel, 0) == WAIT_OBJECT_0)
	{
		// receive command to stop downloading
		LOG_INFO(_T("收到线程退出的要求，退出线程"));
		return 0;
	}
	CFile* pFile = writeDataInfo->m_pFile;
	if (!pFile)
	{
		return 0;
	}
	try
	{
		pFile->Write(pBuffer,static_cast<UINT>(nBufferSize));
	}
	catch (CFileException* pEx)
	{
		LOG_TRACER() ;
		CString strErrMsg = CE_MSG_003;
		pEx->Delete();
		LOG_ERROR(_T("写入本地文件失败，CFileException，Error：%s"), strErrMsg);
		return 0;
	}
	return nBufferSize;
}

void CComEngine::CurlReConnect(CURL* pCurlEasyHandle,HWND hWnd, HANDLE hCancel, LPCTSTR lpszUrl, 
												LPCTSTR lpszFileName, CString& strErrMsg, unsigned long& nFileSize,
												bool isZip, int nTimeOut,  LPVOID pParam,CURLcode& successCode,CFile& file)
{
	if (!pCurlEasyHandle)
	{
		return ;
	}
	LOG_TRACER() ;
	CString strFileName(lpszFileName);
	if (-1 != strFileName.Find(TEMP_FILE_EXTEND_NAME))
	{
		strFileName = strFileName.Mid(0,strFileName.GetLength() - 4);
	}
	CFileException fileException;
	if (!file.Open (lpszFileName, CFile::modeReadWrite, &fileException))
	{
		if (!file.Open(lpszFileName, CFile::modeCreate | CFile::modeReadWrite, &fileException))
		{
			strErrMsg.Format(CE_MSG_002, lpszFileName, fileException.m_cause);
			LOG_ERROR(strErrMsg);
			GetLastErrorString(strErrMsg);
			LOG_ERROR(_T("打开本地文件失败，Error：%s"), strErrMsg);
			file.Close();
			return ;
		}
	}
	USES_CONVERSION_EX;
	LPCSTR lpszUrlA = W2A_CP_EX(lpszUrl, lstrlen(lpszUrl) * 2, CP_UTF8);
	successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_URL, lpszUrlA);
	if (CURLE_OK != successCode)
	{
		LOG_ERROR(_T("CUrlLib请求URL地址失败"));
		file.Close();
		return;
	}
	if (::WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
	{
		// receive command to stop downloading
		LOG_INFO(_T("收到线程退出的要求，退出线程"));
		file.Close();
		return ;
	}
	/*LOG_INFO(_T("设置CURLOPT_USERAGENT"));
	successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1)");
	if ( CURLE_OK != successCode)		
	{
		LOG_INFO(_T("设置CURLOPT_USERAGENT失败"));
		file.Close();
		return ;
	}*/
	curl_slist* pCurlSList=NULL;
	pCurlSList = curl_slist_append(pCurlSList, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	LOG_INFO(_T("设置CURLOPT_HTTPHEADER"));
	if (pCurlSList)
	{
		successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_HTTPHEADER, pCurlSList);
		if ( CURLE_OK != successCode)		
		{
			LOG_INFO(_T("设置CURLOPT_HTTPHEADER失败"));
			curl_slist_free_all(pCurlSList);
			file.Close();
			return ;
		}
	}
	ProxyInfo proxyInfo;
	if (GetDownloadProxy(proxyInfo))
	{
		// 代理服务器类型设置
		successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_PROXYTYPE, proxyInfo.m_dwProxyType); 
		LOG_INFO(_T("当前设置代理服务器类型为%d"), proxyInfo.m_dwProxyType);
		if (CURLE_OK != successCode)
		{
			LOG_ERROR(_T("CUrlLib设置代理服务器类型失败，退出"));
			if (pCurlSList)
			{
				curl_slist_free_all(pCurlSList);
			}
			file.Close();
			return;
		}
		// 代理服务器设置
		successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_PROXY, proxyInfo.m_strProxy.c_str());
		LOG_INFO(_T("当前设置的代理服务器地址和端口为%s"), proxyInfo.m_strProxy.c_str());
		if (CURLE_OK != successCode)
		{
			LOG_ERROR(_T("CUrlLib设置代理服务器失败，退出"));
			if (pCurlSList)
			{
				curl_slist_free_all(pCurlSList);
			}
			file.Close();		
			return;
		}
		if (0 < proxyInfo.m_strUserNamePsd.size())
		{ 
			///代理服务器用户名密码设置
			successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_PROXYUSERPWD, proxyInfo.m_strUserNamePsd.c_str());
			LOG_INFO(_T("当前设置代理服务器用户名密码为%s"), proxyInfo.m_strUserNamePsd.c_str());
			if (CURLE_OK != successCode)
			{
				LOG_ERROR(_T("CUrlLib设置代理服务器用户名密码失败，退出"));
				if (pCurlSList)
				{
					curl_slist_free_all(pCurlSList);
				}
				file.Close();		
				return;
			}
		}
	}
	long lFileStartDownloadIndex = static_cast<long>(file.GetLength());
	bool bResumeDownload = lFileStartDownloadIndex > 0;
	file.Seek(0,CFile::end);
	if (0 < lFileStartDownloadIndex)
	{
		successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_RESUME_FROM, lFileStartDownloadIndex);
		if (CURLE_OK != successCode)
		{
			LOG_ERROR(_T("CUrlLib请求从指定位置下载失败"));
			if (pCurlSList)
			{
				curl_slist_free_all(pCurlSList);
			}
			file.Close();	
			if ( bResumeDownload )
				::DeleteFile( lpszFileName );
			return;
		}
	}
	//curl_easy_setopt(pCurlEasyHandle, CURLOPT_CONNECTTIMEOUT,0xFFFFFF);
	successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_FAILONERROR, 1);
	if (CURLE_OK != successCode)
	{
		LOG_ERROR(_T("CUrlLib设置出错标志位CURLOPT_FAILONERROR失败"));
		if (pCurlSList)
		{
			curl_slist_free_all(pCurlSList);
		}
		file.Close();
		if ( bResumeDownload )
			::DeleteFile( lpszFileName );
		return;
	}
	successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_FOLLOWLOCATION, 1);
	if (CURLE_OK != successCode)
	{
		LOG_ERROR(_T("CUrlLib设置CURLOPT_FOLLOWLOCATION标志位失败"));
		if (pCurlSList)
		{
			curl_slist_free_all(pCurlSList);
		}
		file.Close();
		if ( bResumeDownload )
			::DeleteFile( lpszFileName );
		return;
	}
	////CURLOPT_WRITEFUNCTION 将后继的动作交给write_data函数处理
	successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_WRITEFUNCTION, WriteDataCallbackFunc);
	if (CURLE_OK != successCode)
	{
		LOG_ERROR(_T("CUrlLib设置CURLOPT_WRITEFUNCTION失败"));
		if (pCurlSList)
		{
			curl_slist_free_all(pCurlSList);
		}
		file.Close();
		if ( bResumeDownload )
			::DeleteFile( lpszFileName );
		return;
	}
	WriteDataInfo writeDataInfo;
	writeDataInfo.m_pFile = &file;
	writeDataInfo.m_hCancel = hCancel;
	successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_WRITEDATA,&writeDataInfo);
	if (CURLE_OK != successCode)
	{
		LOG_ERROR(_T("CUrlLib设置CURLOPT_WRITEDATA失败"));
		if (pCurlSList)
		{
			curl_slist_free_all(pCurlSList);
		}
		file.Close();
		if ( bResumeDownload )
			::DeleteFile( lpszFileName );
		return;
	}
	successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_NOPROGRESS, 0L);
	if (CURLE_OK != successCode)
	{
		LOG_ERROR(_T("CUrlLib设置CURLOPT_NOPROGRESS失败"));
		if (pCurlSList)
		{
			curl_slist_free_all(pCurlSList);
		}
		file.Close();
		if ( bResumeDownload )
			::DeleteFile( lpszFileName );
		return;
	}
	////设置Progress相应函数
	successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_PROGRESSFUNCTION, ProcessCallbackFunc);
	if (CURLE_OK != successCode)
	{
		LOG_ERROR(_T("CUrlLib设置CURLOPT_PROGRESSFUNCTION失败"));
		if (pCurlSList)
		{
			curl_slist_free_all(pCurlSList);
		}
		file.Close();
		if ( bResumeDownload )
			::DeleteFile( lpszFileName );
		return;
	}
	/*long lFileLength = 0;
	successCode = curl_easy_getinfo(pCurlEasyHandle,CURLINFO_CONTENT_LENGTH_UPLOAD,&lFileLength);
	if (CURLE_OK == successCode)
	{
		::SendMessage(hWnd, WM_PROGRESS_SET_RANGE, 
									(WPARAM)(lFileLength), (LPARAM)strFileName.GetBuffer());
		strFileName.ReleaseBuffer();
	}*/
	DispProcessInfo dispProcessInfo;
	dispProcessInfo.m_hWnd = hWnd;
	StrCpy(dispProcessInfo.m_lpszFileName,strFileName);
	dispProcessInfo.m_bHasSetRange = false;
	dispProcessInfo.m_lFileStartDownloadIndex = lFileStartDownloadIndex;
	dispProcessInfo.m_hCancel = hCancel;
	successCode = curl_easy_setopt(pCurlEasyHandle, CURLOPT_PROGRESSDATA, &dispProcessInfo);
	if (CURLE_OK != successCode)
	{
		LOG_ERROR(_T("CUrlLib设置CURLOPT_PROGRESSDATA失败"));
		if (pCurlSList)
		{
			curl_slist_free_all(pCurlSList);
		}
		file.Close();
		if ( bResumeDownload )
			::DeleteFile( lpszFileName );
		return;
	}
	if (::WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
	{
		// receive command to stop downloading
		LOG_INFO(_T("收到线程退出的要求，退出线程"));
		file.Close();
		return ;
	}
	successCode = curl_easy_perform(pCurlEasyHandle);
	if (pCurlSList)
	{
		curl_slist_free_all(pCurlSList);
	}
	file.Close();
	return ;
}

BOOL CComEngine::SetDownloadProxy() 
{
	CString strDownloadProxyFileName(_T(""));
	TCHAR szPath[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, szPath);
	strDownloadProxyFileName.Format(_T("%s\\%s"), szPath, STR_PROXY_FILE_NAME);
	TCHAR lpszDownloadProxyAddr[MAX_PATH] = {0};
	BOOL bIsWriteAddrSuccessful = ::WritePrivateProfileString(STR_UPDATE_PROXY_SEC,STR_UPDATE_PROXY_ADDR,
																											lpszDownloadProxyAddr,strDownloadProxyFileName);
	if (!bIsWriteAddrSuccessful)
	{
		return FALSE;
	}
	TCHAR lpszDownloadProxyPort[MAX_PATH] = {0};
	DWORD bIsWritePortSuccessful = ::WritePrivateProfileString(STR_UPDATE_PROXY_SEC,STR_UPDATE_PROXY_PORT,
																												lpszDownloadProxyPort,strDownloadProxyFileName);
	if (!bIsWritePortSuccessful)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CComEngine::GetDownloadProxy(ProxyInfo& proxyInfo)
{
	if (GetProxyFromClient(proxyInfo))
	{
		return TRUE;
	}
	if (GetProxyFromInternet(proxyInfo))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CComEngine::GetProxyFromInternet(ProxyInfo& proxyInfo)
{
	unsigned long nSize = 4096;
	char szBuf[4096] = { 0 };
	INTERNET_PROXY_INFO* pInfo = (INTERNET_PROXY_INFO*)szBuf;
	if (!InternetQueryOption(NULL, INTERNET_OPTION_PROXY, pInfo, &nSize))
	{
		return FALSE;
	}
	if (INTERNET_OPEN_TYPE_DIRECT == pInfo->dwAccessType)
	{
		return FALSE;
	}
	HKEY hKeyIn = HKEY_CURRENT_USER;
	HKEY hSubKey = NULL;
	LPCTSTR lpszRegTablePath =  _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");
	if( ERROR_SUCCESS != RegOpenKeyEx(hKeyIn, lpszRegTablePath, 0, 
															KEY_READ , &hSubKey))
	{
		return FALSE;
	}
	LPCTSTR lpszProxyItemName = _T("ProxyServer");
	ULONG nProxyContentsLen = 0;
	if(ERROR_SUCCESS != RegQueryValueEx(hSubKey, lpszProxyItemName, NULL, NULL, NULL, &nProxyContentsLen))
	{
		return FALSE;
	}
	std::vector<TCHAR> proxyValueVec(nProxyContentsLen);
	memset(&(proxyValueVec[0]), 0, nProxyContentsLen * sizeof(TCHAR));
	if(ERROR_SUCCESS != RegQueryValueEx(hSubKey, lpszProxyItemName, NULL, NULL, 
															LPBYTE(&proxyValueVec[0]), &nProxyContentsLen))
	{
		return FALSE;
	}
	USES_CONVERSION_EX;
	proxyInfo.m_strProxy = W2A_CP_EX(&proxyValueVec[0], NULL, CP_ACP);
	size_t nHttpProxyStartIndex = 0;
	size_t nHttpProxyEndIndex = 0;
	LPCSTR lpszHttpProxyNameA = "http=";
	char chProxySeperateSymbol = ';';
	nHttpProxyStartIndex = proxyInfo.m_strProxy.find(lpszHttpProxyNameA);
	if (string::npos != nHttpProxyStartIndex)
	{
		nHttpProxyStartIndex += strlen(lpszHttpProxyNameA);
		nHttpProxyEndIndex = proxyInfo.m_strProxy.find(chProxySeperateSymbol, nHttpProxyStartIndex);
		if (string::npos != nHttpProxyEndIndex)
		{
			size_t nProxyLength = nHttpProxyEndIndex - nHttpProxyStartIndex;
			proxyInfo.m_strProxy = proxyInfo.m_strProxy.substr(nHttpProxyStartIndex, nProxyLength);
			proxyInfo.m_dwProxyType = CURLPROXY_HTTP;
			return TRUE;
		}
	}
	else
	{
		if (0 < proxyInfo.m_strProxy.size())
		{
			proxyInfo.m_dwProxyType = CURLPROXY_HTTP;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CComEngine::GetProxyFromClient(ProxyInfo& proxyInfo)
{
	CMarkup markup;
	bool bSucceed = markup.Load(STR_USER_PROXY_PATH);
	if (bSucceed)
	{
		bSucceed = markup.FindElem();
		if (!bSucceed)
		{
			return FALSE;
		}
		markup.IntoElem();
		bSucceed = markup.FindElem(STR_PROXY_ROOT_NODE_NAME);
		if (bSucceed)
		{
			bSucceed = markup.IntoElem();
			if (bSucceed)
			{
				bSucceed = markup.FindElem(STR_PROXY_TYPE_NODE_NAME);
				if (!bSucceed)
				{
					return FALSE;
				}
				CString strProxyType = markup.GetData();
				if (0 == _tcslen(strProxyType))
				{
					return FALSE;
				}
				proxyInfo.m_dwProxyType = _ttoi(strProxyType);
				std::map<DWORD, DWORD>::iterator iter = m_mapClientProxyTypeCurlProxyType.find(proxyInfo.m_dwProxyType);
				if (iter == m_mapClientProxyTypeCurlProxyType.end())
				{
					return FALSE;
				}
				proxyInfo.m_dwProxyType = iter->second;
				bSucceed = markup.FindElem(STR_PROXY_SERVER_NODE_NAME);
				if (bSucceed)
				{
					CString strServer = markup.GetData();
					if (0 == _tcslen(strServer))
					{
						return FALSE;
					}
					bSucceed = markup.FindElem(STR_PROXY_PORT_NODE_NAME);
					if (bSucceed)
					{
						CString strPort = markup.GetData();
						if (0 == _tcslen(strPort))
						{
							return FALSE;
						}
						CString strProxy;
						strProxy.Format(_T("%s:%s"), strServer, strPort);
						USES_CONVERSION_EX;
						proxyInfo.m_strProxy = W2A_CP_EX(strProxy, NULL, CP_ACP);
						bSucceed = markup.FindElem(STR_PROXY_USER_NAME_NODE_NAME);
						CString strUserName = markup.GetData();
						if (0 < _tcslen(strUserName))
						{
							bSucceed = markup.FindElem(STR_PROXY_PASSWORD_NODE_NAME);
							CString strPassword = markup.GetData();
							if (0 < _tcslen(strPassword))
							{
								CString strUserNamePsd;
								strUserNamePsd.Format(_T("%s:%s"), strUserName, strPassword);
								proxyInfo.m_strUserNamePsd = W2A_CP_EX(strUserNamePsd, NULL, CP_ACP);
							}
						}
					}
				}
			}
			markup.OutOfElem();
		}
		markup.OutOfElem();
	}
	if (!bSucceed)
	{
		return FALSE;
	}
	return TRUE;
}

bool CComEngine::DownloadUseCurl(HWND hWnd, HANDLE hCancel, LPCTSTR lpszUrl, 
												  LPCTSTR lpszFileName, CString& strErrMsg, unsigned long& nFileSize,
												  bool isZip, int nTimeOut,  LPVOID pParam)
{
	LOG_TRACER() ;
	CURLcode succedCode = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != succedCode)
	{
		return false;
	}
	CURL* pCurlEasyHandle = NULL;
	pCurlEasyHandle = curl_easy_init();
	if (!pCurlEasyHandle)
	{
		return false;
	}
	CFile file;
	CURLcode successCode = CURL_LAST;
	if (::WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
	{
		// receive command to stop downloading
		curl_easy_cleanup(pCurlEasyHandle);
		LOG_INFO(_T("收到线程退出的要求，退出线程"));
		return true;

	}
	SetTimer(hWnd,SEND_REQUEST_TIMER_ID,SEND_REQUEST_TIMER_PERIOD,NULL);
	CurlReConnect(pCurlEasyHandle,hWnd,hCancel,lpszUrl,lpszFileName,
								strErrMsg,nFileSize,isZip,nTimeOut,pParam,successCode,file);
	while (CURLE_OK != successCode && CURLE_WRITE_ERROR != successCode && CURLE_RANGE_ERROR != successCode)
	{
		if (::WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
		{
			// receive command to stop downloading
			curl_easy_cleanup(pCurlEasyHandle);
			LOG_INFO(_T("收到线程退出的要求，退出线程"));
			return true;

		}
		LOG_INFO(_T("重置pCurlEasyHandle"));
		curl_easy_reset(pCurlEasyHandle);
		LOG_INFO(_T("网络断开，不断重连，直到网络连接良好！"));
		LOG_INFO(_T("错误代码为%s"), successCode);
		CurlReConnect(pCurlEasyHandle,hWnd,hCancel,lpszUrl,lpszFileName,
									strErrMsg,nFileSize,isZip,nTimeOut,pParam,successCode,file);
	}
	if (::WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
	{
		// receive command to stop downloading
		curl_easy_cleanup(pCurlEasyHandle);
		if (INVALID_HANDLE_VALUE != file.m_hFile)
		{
			file.Close();
		}
		LOG_INFO(_T("收到线程退出的要求，退出线程"));
		return true;
	}
	KillTimer(hWnd,SEND_REQUEST_TIMER_ID);
	CString strFileName(lpszFileName);
	if (INVALID_HANDLE_VALUE != file.m_hFile)
	{
		file.Close();
	}
	if (-1 != strFileName.Find(TEMP_FILE_EXTEND_NAME))
	{
		strFileName = strFileName.Mid(0,strFileName.GetLength() - 4);
	}
	try
	{
		CFile::Rename(lpszFileName,strFileName);
	}
	catch (CFileException* e)
	{
		strErrMsg = L"下载的文件由临时文件名重命名为最终文件名失败";
		e->Delete();
		LOG_ERROR(_T("重命名%s为%s失败"), lpszFileName, strFileName);
		return false;
	}
	curl_easy_cleanup(pCurlEasyHandle);
	return true;
}

// check if download thread is running
bool CComEngine::IsDownloading(void)
{
	if (::WaitForSingleObject(m_hThreadRunning, 100) == WAIT_OBJECT_0)
	{
		// event is signaled, means thread is running
		return true;
	}
	else
	{
		return false;
	}
}

// close the downloading thread by force
void CComEngine::StopDownload(void)
{
	if (IsDownloading())
	{
		::TerminateThread(m_hThread, 0);
		m_hThread = NULL;
	}
}

CComEngine::CComEngine(void)
{
	m_hReadFile = ::CreateEvent(NULL, TRUE, TRUE, NULL);
	if (m_hThreadRunning == NULL)
	{
		m_hThreadRunning = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	m_hThread = NULL;
	int nProxyCount = _countof(m_pairClientProxyTypeCurlProxyType);
	for ( int i = 0; i < nProxyCount; ++i)
	{
		m_mapClientProxyTypeCurlProxyType[m_pairClientProxyTypeCurlProxyType[i].first] = m_pairClientProxyTypeCurlProxyType[i].second;
	}
}

CComEngine::~CComEngine(void)
{
	// close download thread
	if (IsDownloading())
	{
		::TerminateThread(m_hThread, 0);
		m_hThread = NULL;
	}

	if (m_hThreadRunning != NULL)
	{
		::CloseHandle(m_hThreadRunning);
	}
}

// thread func to download the list files user provide.
UINT CComEngine::DownloadProc(LPVOID lpParam)
{
	LOG_TRACER() ;
	LOG_INFO(_T("DownloadProc开始"));
	unsigned long nFileSize = 0;
	DLP_PARAM* pDLPP = (DLP_PARAM*)lpParam;
	TCHAR szUrl[_MAX_PATH] = {0};
	TCHAR szDir[_MAX_PATH] = {0};
	TCHAR szFileName[_MAX_FNAME] = {0};
	//TCHAR szFileNameWithoutPath[_MAX_FNAME] = {0};
	TCHAR* pFileName = NULL;
	TCHAR* pIndexUrl = (TCHAR*)pDLPP->szUrlList;
	TCHAR* pIndexDir = (TCHAR*)pDLPP->szDirList;
	LPBYTE pbZip = pDLPP->bZipList;
	CString strErrMsg, strFullDir;
	pDLPP->bSuccess = false;

	int nFileCount = 0;
	
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Added:		2009/10/19   14:24
	// comments:	下载开始的通知
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	LOG_INFO(_T("PostMessage WM_DOWNLOAD_START"));
	::PostMessage(pDLPP->hWnd, WM_DOWNLOAD_START, 0, (LPARAM)pDLPP->param);

	//::PostMessage(pDLPP->hWnd, WM_PROGRESS_SET_RANGE, 0, pDLPP->nFileNum);

	//------------------------------------------------------------------------
	// Deleted:		2009/10/20   17:15
	// Comments:	现在改用传入的BYTE数组来判断是否Zip
	/**************************************************************************
	CUIntArray &ZipArry = theApp.m_pUpdateInfo->FilesIsZip();
	***************************************************************************/
	//------------------------------------------------------------------------

	while (true)
	{
		if (::WaitForSingleObject(pDLPP->hCancel, 100) == WAIT_OBJECT_0)
		{
			// receive command to stop downloading
			LOG_INFO(_T("收到线程退出的要求，退出线程"));
			//break;
			::ResetEvent(m_hThreadRunning);
			return false;
		}

		// get url
		if (pIndexUrl >= pDLPP->szUrlList + pDLPP->nUrlListLen)
		{
			break;
		}
		lstrcpy(szUrl, pIndexUrl);
		pIndexUrl += lstrlen(szUrl) + 1;

		// get directory
		if (pIndexDir >= pDLPP->szDirList + pDLPP->nDirListLen)
		{
			break;
		}
		lstrcpy(szDir, pIndexDir);
		pIndexDir += lstrlen(szDir) + 1;

		// get file name from url
		pFileName = szUrl + lstrlen(szUrl) - 1;
		while (pFileName >= szUrl)
		{
			if ((pFileName[0] == _T('\\')) || (pFileName[0] == _T('/')))
			{
				break;
			}
			pFileName--;
		}
		pFileName++;
		if (lstrlen(szDir) == 0)
		{
			_stprintf_s(szFileName, _MAX_FNAME, _T("%s\\%s"), pDLPP->szUpdatePath, pFileName);
		}
		else
		{
			_stprintf_s(szFileName, _MAX_FNAME, _T("%s\\%s\\%s"), pDLPP->szUpdatePath, szDir, pFileName);

			// check directory for existence
			strFullDir.Format(_T("%s\\%s\\"), pDLPP->szUpdatePath, szDir);
			if ((::_taccess(strFullDir, 0)) == -1)
			{
				LOG_INFO(_T("%s不存在，创建之"), strFullDir);

				// make the directory
				USES_CONVERSION;
				if(!::MakeSureDirectoryPathExists(T2A(strFullDir)))
				{
					LOG_ERROR(_T("MakeSureDirectoryPathExists %s失败"), strFullDir);

					break;
				}
			}
		}

		LOG_INFO(_T("下载文件URL：%s"), szUrl);
		LOG_INFO(_T("下载文件名：%s"), szFileName);

		//更改：(ZipArry.GetAt(nFileCount)) != 0)   ->    (pbZip[nFileCount] != 0)
		double dServicePackSize = 0.0;
		long lFileStartDownloadIndex = 0;
		//if(STATUS_FILES == ((LONGLONG)(pDLPP->param)) || STATUS_INSTALL_PACK == ((LONGLONG)(pDLPP->param)))
		//{
		//	LOG_INFO(_T("当前下载状态为STATUS_FILES或者STATUS_INSTALL_PACK"));
		//	_stprintf_s(szFileName,_MAX_FNAME, _T("%s%s"),szFileName,TEMP_FILE_EXTEND_NAME);
		//	std::vector<double>* pServicePackSizes = theApp.m_pUpdateInfo->GetServicePackSizes();
		//	if(pServicePackSizes)
		//	{
		//		dServicePackSize = (*pServicePackSizes)[nFileCount];
		//		LOG_INFO(_T("文件%s原始大小为：%d Byte"), CS2TS(pFileName), dServicePackSize);
		//	}
		//	std::vector<long>* pFileDownloadIndexs = theApp.m_pUpdateInfo->GetFileStartDownloadIndexs();
		//	if(pFileDownloadIndexs)
		//	{
		//		lFileStartDownloadIndex = (*pFileDownloadIndexs)[nFileCount];
		//		LOG_INFO(_T("文件%s已下载大小为：%d Byte"), CS2TS(pFileName), lFileStartDownloadIndex);
		//	}
		//}
		bool bDownloadSucceed = CComEngine::DownloadUseCurl(pDLPP->hWnd, pDLPP->hCancel, 
																						szUrl, szFileName, strErrMsg, nFileSize, 
																						(pbZip[nFileCount] != 0),NULL,
																						&lFileStartDownloadIndex);
		if (::WaitForSingleObject(pDLPP->hCancel, 100) == WAIT_OBJECT_0)
		{
			// receive command to stop downloading
			LOG_INFO(_T("收到线程退出的要求，退出线程"));
			//break;
			::ResetEvent(m_hThreadRunning);
			return false;
		}
		bool bFileRealSizeCorrect = true;
		if(STATUS_FILES == ((LONGLONG)(pDLPP->param)) || STATUS_INSTALL_PACK == ((LONGLONG)(pDLPP->param)))
		{
			if(bDownloadSucceed)
			{
				CString strFileLastName(szFileName);
				if (-1 != strFileLastName.Find(TEMP_FILE_EXTEND_NAME))
				{
					strFileLastName = strFileLastName.Mid(0,strFileLastName.GetLength() - 4);
					_tcscpy(szFileName,strFileLastName);
				}
				LOG_INFO(_T("%s已经成功下载到本地"), pFileName);
				HANDLE hFile = ::CreateFile(szFileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,
																	OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,   NULL);
				if (INVALID_HANDLE_VALUE != hFile)
				{
					LOG_INFO(_T("取得%s下载到本地的大小"), pFileName);
					DWORD dServicePackRealSize = ::GetFileSize(hFile,NULL);
					::CloseHandle(hFile);
					LOG_INFO(_T("判断文件%s原始大小和下载到本地的大小是否相等"), pFileName);
					bFileRealSizeCorrect = (dServicePackSize == dServicePackRealSize);
				}
			}
		}
		if (!bDownloadSucceed || !bFileRealSizeCorrect)
		{
			//log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("lu");
			//LOG4CXX_TRACE(logger, strErrMsg.GetBuffer(0));
			if (!bDownloadSucceed)
			{
				LOG_INFO(_T("%s下载不成功"), pFileName);
			}
			else
			{
				LOG_INFO(_T("%s下载成功，但是下载到本地文件的大小和文件原始大小不相等，下载失败！"), pFileName);
			}
			::DeleteFile(szFileName);
			strErrMsg.ReleaseBuffer();
			
			CString str;
			if (!bDownloadSucceed)
			{
				GetLastErrorString(str);
			}
			LOG_ERROR(_T("SendMessage WM_PROGRESS_ERROR"));
			::SendMessage(pDLPP->hWnd, WM_PROGRESS_ERROR, (WPARAM)::GetLastError(), (LPARAM)szFileName);
			LOG_ERROR(_T("设置成功标志 pDLPP->bSuccess = false"));
			pDLPP->bSuccess = false;
			::ResetEvent(m_hThreadRunning);
			//之前是break，会导致再次发送DownloadEnd的消息回主线程
			return 0;
		}
		LOG_INFO(_T("%s原始大小和下载到本地的大小相等，下载成功！"), pFileName);
		nFileCount ++;
		LOG_INFO(_T("已下载文件个数：%d"), nFileCount);
		//::PostMessage(pDLPP->hWnd, WM_PROGRESS_STEP_IT, (WPARAM)pFileName, (LPARAM)nFileCount);

		// wait a while for user comfort
		//::Sleep(500);
	}
	
	if (nFileCount == pDLPP->nFileNum)
	{
		LOG_INFO(_T("设置成功标志 pDLPP->bSuccess = true"));
		pDLPP->bSuccess = true;
	}

	// set thread end flag
	::ResetEvent(m_hThreadRunning);

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Added:		2009/10/19   14:24
	// comments:	下载完成的通知
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	LOG_INFO(_T("PostMessage WM_DOWNLOAD_END"));
	::PostMessage(pDLPP->hWnd, WM_DOWNLOAD_END, 0, (LPARAM)pDLPP->param);

	return 0;
}