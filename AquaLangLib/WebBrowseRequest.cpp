#include "Pch.h"
#include "WebBrowseRequest.h"
#include "WebBrowseSharedMemStruct.h"
#include "ApplicationDataPath.h"
#include "DialogObject.h"
#include "WebBrowseDialog.h"
#include "Log.h"

#define BROWSE_RESPONSE_TIMEOUT_MSEC 2000
#define WEB_BROWSER_DIALOG_TITLE _T("AquaLang Web Browser")

WebBrowseRequest::WebBrowseRequest()
	: m_SharedMem(),
	m_BroweserProcessHandle(NULL),
	m_pBuffer(NULL),
	m_BrowseRequestEvent(NULL),
	m_BrowseResponseEvent(NULL),
	m_KeepAliveObject(NULL)
{
}

WebBrowseRequest::~WebBrowseRequest()
{
	Close();
}

bool WebBrowseRequest::Open(LPCTSTR UserName)
{
	if(UserName == NULL)
		return false;
	m_UserName = UserName;

	KillPreviousBrowserProcesses();

	m_BrowseRequestEvent.UpdateName((m_UserName + _T("_BrowseRequestEvent")).c_str());
	if(!m_BrowseRequestEvent.Create())
		return false;
	m_BrowseResponseEvent.UpdateName((m_UserName + _T("_BrowseResponseEvent")).c_str());
	if(!m_BrowseResponseEvent.Create())
		return false;
	m_KeepAliveObject.UpdateName((m_UserName + _T("_BrowseKeepAlive")).c_str());
	if(!m_KeepAliveObject.Create())
		return false;

	m_pBuffer = (WebBrowseSharedMemStruct *)m_SharedMem.Create((m_UserName + _T("_SharedMem")).c_str(), sizeof(WebBrowseSharedMemStruct));
	if(!m_pBuffer)
		return false;
	_tcscpy_s(m_pBuffer->DialogTitle, sizeof(m_pBuffer->DialogTitle) / sizeof(m_pBuffer->DialogTitle[0]), WEB_BROWSER_DIALOG_TITLE);

	if(!OpenBrowserProcess())
		return false;

	return true;
}

bool WebBrowseRequest::Browse(LPCSTR SearchUrl, int SearchScrollDown, LPCSTR TranslateUrl, int TranslateSrollDown, LPCTSTR SearchString, const RECT &rCaretRect)
{
	if(!m_pBuffer)
		return false;

	if(strlen(SearchUrl) >= sizeof(m_pBuffer->Search.Url))
		return false;
	strcpy_s(m_pBuffer->Search.Url, sizeof(m_pBuffer->Search.Url), SearchUrl);
	m_pBuffer->Search.ScrollDown = SearchScrollDown;

	if(strlen(TranslateUrl) >= sizeof(m_pBuffer->Translate.Url))
		return false;
	strcpy_s(m_pBuffer->Translate.Url, sizeof(m_pBuffer->Translate.Url), TranslateUrl);
	m_pBuffer->Translate.ScrollDown = TranslateSrollDown;

	if(_tcslen(SearchString) >= sizeof(m_pBuffer->SearchString) / sizeof(m_pBuffer->SearchString[0]))
		return false;
	_tcscpy_s(m_pBuffer->SearchString, sizeof(m_pBuffer->SearchString) / sizeof(m_pBuffer->SearchString[0]), SearchString);

	m_BrowseResponseEvent.Reset();

	m_pBuffer->CaretRect = rCaretRect;
	m_pBuffer->SequenceCount++;
	m_BrowseRequestEvent.Set();

	if(!m_BrowseResponseEvent.Wait(BROWSE_RESPONSE_TIMEOUT_MSEC))
	{
		Log(_T("WebBrowseRequest::Browse - failed waiting for response event\n"));
		CloseBrowserProcess();
		if(!OpenBrowserProcess())
			return false;
	
		m_pBuffer->SequenceCount++;
		m_BrowseRequestEvent.Set();
		if(!m_BrowseResponseEvent.Wait(BROWSE_RESPONSE_TIMEOUT_MSEC))
		{
			Log(_T("WebBrowseRequest::Browse - failed waiting for response event in reopen\n"));
			return false;
		}
	}
	return true;
}

void WebBrowseRequest::Close()
{
	CloseBrowserProcess();

	m_SharedMem.Destroy();
	m_pBuffer = NULL;
	m_BrowseRequestEvent.Destroy();
	m_BrowseResponseEvent.Destroy();
	m_KeepAliveObject.Destroy();
}

bool WebBrowseRequest::KillPreviousBrowserProcesses()
{
	HWND hBrowserWindow = DialogObject::FindAquaLangWindow(WEB_BROWSER_DIALOG_TITLE, m_UserName.c_str());
	if(hBrowserWindow == NULL)
		return true;

	WebBrowseDialog::TerminateRequest(hBrowserWindow);

	Sleep(500);

	hBrowserWindow = DialogObject::FindAquaLangWindow(WEB_BROWSER_DIALOG_TITLE, m_UserName.c_str());
	if(hBrowserWindow != NULL)
		return false;

	return true;
}

bool WebBrowseRequest::OpenBrowserProcess()
{
	if(!m_pBuffer)
		return false;
	m_pBuffer->fTerminate = false;

	ApplicationDataPath PathFinder(NULL);
	_tstring ApplicationName;
	PathFinder.GetApplicationName(ApplicationName);

	TCHAR CommandSwitches[1024];
	_stprintf_s(
		CommandSwitches, sizeof(CommandSwitches) / sizeof(CommandSwitches[0]),
		_T("-runbrowser \"%s\""),
		m_UserName.c_str()
		);
	if(!Process::Create(ApplicationName.c_str(), CommandSwitches, _T("AquaLang web search process"), &m_BroweserProcessHandle))
		return false;

	if(!m_BrowseResponseEvent.Wait(BROWSE_RESPONSE_TIMEOUT_MSEC))
		return false;
	if(!m_pBuffer->fClientActive)
		return false;
	return true;
}

void WebBrowseRequest::CloseBrowserProcess()
{
	m_BrowseResponseEvent.Reset();

	if(m_pBuffer && m_pBuffer->fClientActive)
	{
		m_pBuffer->fTerminate = true;
	}

	if(m_BroweserProcessHandle != NULL)
	{
		m_BrowseRequestEvent.Set();
		m_BrowseResponseEvent.Wait(BROWSE_RESPONSE_TIMEOUT_MSEC);
		if(m_pBuffer->fClientActive)
		{
			// try to kill the process
			Log(_T("WebBrowseRequest::CloseBrowserProcess - failed organized closing - terminating process!\n"));
			::TerminateProcess(m_BroweserProcessHandle, (UINT)-1);
			m_pBuffer->fClientActive = false;
		}
		::CloseHandle(m_BroweserProcessHandle);
		m_BroweserProcessHandle = NULL;
	}
}
