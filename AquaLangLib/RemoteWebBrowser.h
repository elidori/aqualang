#ifndef _REMOTE_WEB_BROWSER_HEADER_
#define _REMOTE_WEB_BROWSER_HEADER_

#include "WebBrowseDialog.h"
#include "ProcessUtils.h"
#include "TimeHelpers.h"

class UtilityWindow;
struct WebBrowseSharedMemStruct;

class RemoteWebBrowser : public IBrowserOwner
{
public:
	RemoteWebBrowser(LPCTSTR UserName);
	virtual ~RemoteWebBrowser();

	bool Run();
private:
	bool Open();
	void Close();

	// IBrowserOwner pure virtuals {
	virtual void Terminate();
	// }
private:
	_tstring m_UserName;

	WebBrowseDialog m_Browser;
	
	ProcessSharedMem m_SharedMem;
	WebBrowseSharedMemStruct *m_pBuffer;

	int m_LatestSequenceCountHandled;
	ProcessEvent m_BrowseRequestEvent;
	ProcessEvent m_BrowseResponseEvent;

	ProcessEvent m_KeepAliveObject;
	Timer m_NextKeepAliveTime;

	bool m_fTerminateRequest;
};

#endif // _REMOTE_WEB_BROWSER_HEADER_