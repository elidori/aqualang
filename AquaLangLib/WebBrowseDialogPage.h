#ifndef _WEB_BROWSE_DIALOG_PAGE_HEADER_
#define _WEB_BROWSE_DIALOG_PAGE_HEADER_

#include "DialogObject.h"
#include "WebBrowseControl.h" 

class WebBrowseDialogPage : public DialogObject
{
public:
	WebBrowseDialogPage();
	virtual ~WebBrowseDialogPage();

	bool Open(HWND hParent);
	bool Navigate(LPCSTR szUrl, int InitialScrollDownSize);
	bool NavigateToNULL();
	void Close();

	bool UpdateBrowserSize(int Width, int Height);

	bool NavigateBack();
	bool NavigateForward();
	bool GetCurrentUrl(std::string &rCurrentUrl);
private:
	// DialogObject overrides {
	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault);
	// }
private:
	WebBrowseControl m_Browser;

	int m_InitialScrollDownSize; // the browser shall automatically sroll down to this value in each navigate

	int m_XOffset;
	int m_YOffset;
};

#endif // _WEB_BROWSE_DIALOG_PAGE_HEADER_