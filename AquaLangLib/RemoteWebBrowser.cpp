#include "Pch.h"
#include "RemoteWebBrowser.h"
#include "WebBrowseSharedMemStruct.h"
#include "Log.h"

#define KEEP_ALIVE_INTERVAL_MSEC 5000
#define POLLING_INTERVAL_MSEC 10

#pragma warning (disable:4355)

RemoteWebBrowser::RemoteWebBrowser(LPCTSTR UserName)
	: m_Browser(*this),
	m_SharedMem(),
	m_pBuffer(NULL),
	m_LatestSequenceCountHandled(0),
	m_BrowseRequestEvent(NULL),
	m_BrowseResponseEvent(NULL),
	m_KeepAliveObject(NULL),
	m_NextKeepAliveTime(),
	m_fTerminateRequest(false)
{
	assert(UserName != NULL);
	if(UserName != NULL)
	{
		m_UserName = UserName;
		m_BrowseRequestEvent.UpdateName((_tstring(UserName) + _T("_BrowseRequestEvent")).c_str());
		m_BrowseResponseEvent.UpdateName((_tstring(UserName) + _T("_BrowseResponseEvent")).c_str());
		m_KeepAliveObject.UpdateName((_tstring(UserName) + _T("_BrowseKeepAlive")).c_str());
	}
}

RemoteWebBrowser::~RemoteWebBrowser()
{
	Close();
}

bool RemoteWebBrowser::Run()
{
	Log(_T("RemoteWebBrowser::Run - UserName is %s\n"), m_UserName.c_str());
	if(!Open())
	{
		Log(_T("RemoteWebBrowser::Run - failed open\n"));
		return false;
	}

	MSG msg;
	while(!m_pBuffer->fTerminate && !m_fTerminateRequest)
	{
		while(PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		m_BrowseRequestEvent.Wait(POLLING_INTERVAL_MSEC);
		if(m_fTerminateRequest || m_pBuffer->fTerminate)
		{
			m_Browser.Show(false);
			break;
		}

		if(m_LatestSequenceCountHandled != m_pBuffer->SequenceCount)
		{
			Log(_T("RemoteWebBrowser::Run - Executing request\n"));

			// copy the shared memory locally
			WebBrowseSharedMemStruct CurrentCopy = *m_pBuffer;
			LogA("RemoteWebBrowser::Run - search path is %s\n", CurrentCopy.Search.Url);

			// confirm handling the request
			m_LatestSequenceCountHandled = m_pBuffer->SequenceCount;
			m_BrowseResponseEvent.Set();

			// open browser
			if(m_Browser.Browse(CurrentCopy.Search.Url, CurrentCopy.Search.ScrollDown, CurrentCopy.Translate.Url, CurrentCopy.Translate.ScrollDown, CurrentCopy.SearchString))
			{
				m_Browser.Display(CurrentCopy.CaretRect, _tcslen(CurrentCopy.SearchString));
			}
			else
			{
				Log(_T("RemoteWebBrowser::Run - Failed using IE Com browser, running default browser\n"));
				::ShellExecuteA(NULL, "open", CurrentCopy.Search.Url, NULL, NULL, SW_SHOWNORMAL);
			}
		}

		if(m_NextKeepAliveTime.GetDiff() > KEEP_ALIVE_INTERVAL_MSEC)
		{
			if(!m_KeepAliveObject.Open())
			{
				Log(_T("RemoteWebBrowser::Run - Keep alive failed. Closing\n"));
				break; // AquaLang application has unexpectedly stopped. The browser should stop as well
			}
			m_KeepAliveObject.Destroy();
			m_NextKeepAliveTime.Restart();
		}
	}

	Log(_T("RemoteWebBrowser::Run - Exiting. fTerminate=%d\n"), m_pBuffer->fTerminate);
	m_pBuffer->fClientActive = false;
	m_BrowseResponseEvent.Set(); // confirm termination
	Close();
	return true;
}

bool RemoteWebBrowser::Open()
{
	if(!m_BrowseRequestEvent.Open())
		return false;
	if(!m_BrowseResponseEvent.Open())
		return false;

	m_pBuffer = (WebBrowseSharedMemStruct *)m_SharedMem.Open((m_UserName + _T("_SharedMem")).c_str());
	if(!m_pBuffer)
		return false;

	if(!m_Browser.Open(m_UserName.c_str(), m_pBuffer->DialogTitle))
		return false;

	m_pBuffer->fClientActive = true;
	m_BrowseResponseEvent.Set();

	m_NextKeepAliveTime.Restart();

	return true;
}

void RemoteWebBrowser::Close()
{
	m_Browser.Close();
	m_SharedMem.Destroy();
	m_pBuffer = NULL;
	m_LatestSequenceCountHandled = 0;
	m_BrowseRequestEvent.Destroy();
	m_BrowseResponseEvent.Destroy();
	m_KeepAliveObject.Destroy();
	m_NextKeepAliveTime.Restart();
	m_fTerminateRequest = false;
}

void RemoteWebBrowser::Terminate()
{
	m_fTerminateRequest = true;
	m_BrowseRequestEvent.Set();
}
