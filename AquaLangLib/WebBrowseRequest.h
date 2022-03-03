#ifndef _WEB_BROWSE_REQUEST_HEADER_
#define _WEB_BROWSE_REQUEST_HEADER_

#include "ProcessUtils.h"

struct WebBrowseSharedMemStruct;

class WebBrowseRequest
{
public:
	WebBrowseRequest();
	virtual ~WebBrowseRequest();

	bool Open(LPCTSTR UserName);
	bool Browse(LPCSTR SearchUrl, int SearchScrollDown, LPCSTR TranslateUrl, int TranslateSrollDown, LPCTSTR SearchString, const RECT &rCaretRect);
	void Close();
private:
	bool KillPreviousBrowserProcesses();
	bool OpenBrowserProcess();
	void CloseBrowserProcess();
private:
	_tstring m_UserName;

	HANDLE m_BroweserProcessHandle;

	ProcessSharedMem m_SharedMem;
	WebBrowseSharedMemStruct *m_pBuffer;

	ProcessEvent m_BrowseRequestEvent;
	ProcessEvent m_BrowseResponseEvent;

	ProcessEvent m_KeepAliveObject; // only creating. This object shall be polled by the remote browser
};

#endif // _WEB_BROWSE_REQUEST_HEADER_