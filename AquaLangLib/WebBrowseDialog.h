#ifndef _WEB_BROWSE_DIALOG_HEADER_
#define _WEB_BROWSE_DIALOG_HEADER_

#include "SimpleOleClientSite.h"
#include "DialogObject.h"
#include "WebBrowserXml.h"
#include "DialogTab.h"

class WebBrowseDialogPage;

class IBrowserOwner
{
protected:
	virtual ~IBrowserOwner() {}
public:
	virtual void Terminate() = 0;
};

class WebBrowseDialog :
	public DialogObject
{
public:
	WebBrowseDialog(IBrowserOwner &rOwner);
	virtual ~WebBrowseDialog();

	bool Open(LPCTSTR UserName, LPCTSTR Title);
	bool Browse(LPCSTR szSearchUrl, int SearchInitialScrollDownOffset, LPCSTR szTranslateUrl, int TranslateInitialScrollDownOffset, LPCTSTR SearchString);
	bool Display(const RECT &rCaretRect, size_t nTextLength);
	void Close();

	static void TerminateRequest(HWND hwnd);
private:
	void CalcOffsetFromDialogTop();
	void HideDialog();

	bool UpdateBrowserSize(int Width, int Height);

	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault);

	bool AddPage(HWND hPageWnd, WebBrowseDialogPage *pPage, LPCTSTR Name);
	WebBrowseDialogPage *GetActiveBrowserPage();
private:
	IBrowserOwner &m_rOwner;
	WebBrowserXml m_WebBrowserXml;
	bool m_fComInitialized;

	RECT m_RecentWindowRect;

	DialogTab m_DialogTab;
	WebBrowseDialogPage *m_pSearchBrowser;
	WebBrowseDialogPage *m_pTranslateBrowser;
	typedef std::map<int, WebBrowseDialogPage *> BrowserMap;
	BrowserMap m_BrowserMap;

	int m_BrowserOffsetFromDialogTop;
};

#endif // _WEB_BROWSE_DIALOG_HEADER_