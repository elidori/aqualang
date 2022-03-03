#ifndef _WEB_BROWSE_CONTROL_HEADER_
#define _WEB_BROWSE_CONTROL_HEADER_

#include "SimpleOleClientSite.h"

class WebBrowseControl
{
public:
	WebBrowseControl();
	virtual ~WebBrowseControl();

	bool Open(HWND hDialog);
	bool Navigate(LPCSTR szUrl);
	bool CheckIfNavigationComplete(bool &rfComplete);
	void Close();

	bool UpdateBrowserSize(int X, int Y, int Width, int Height);
	bool ScrollBy(int X, int Y);

	bool NavigateBack();
	bool NavigateForward();
	bool GetCurrentUrl(std::string &rCurrentUrl);
private:
	std::string m_Url;

	// ActiveX
	SimpleOleClientSite *m_pSite;
	CNullStorage m_Storage;
	IOleObject *m_pWebObject;
};

#endif // _WEB_BROWSE_CONTROL_HEADER_