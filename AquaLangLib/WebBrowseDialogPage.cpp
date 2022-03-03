#include "Pch.h"
#include "WebBrowseDialogPage.h"
#include "StringConvert.h"
#include "resource.h"
#include "Log.h"

#define WAIT_FOR_NAV_COMPLTETE_POLL_TIME 100

enum
{
	WAIT_FOR_NAV_TIMER_ID = 1,
};

WebBrowseDialogPage::WebBrowseDialogPage()
	: DialogObject(-1, false),
	m_Browser(),
	m_InitialScrollDownSize(0),
	m_XOffset(0),
	m_YOffset(0)
{
}

WebBrowseDialogPage::~WebBrowseDialogPage()
{
	Close();
}

bool WebBrowseDialogPage::Open(HWND hParent)
{
	HWND hDialog = OpenDialog(_T(""), IDD_DIALOG_WEB_BROWSE_PAGE, hParent);
	if(hDialog == NULL)
		return false;

	POINT TabClientPt = {0, 0};
	POINT PagePt = {0, 0};
	if(::ClientToScreen(hParent, &TabClientPt) && ::ClientToScreen(hDialog, &PagePt))
	{
		m_XOffset = PagePt.x - TabClientPt.x;
		m_YOffset = PagePt.y - TabClientPt.y;
	}

	if(!m_Browser.Open(hDialog))
		return false;

	return true;
}

bool WebBrowseDialogPage::Navigate(LPCSTR szUrl, int InitialScrollDownSize)
{
	StopTimer(WAIT_FOR_NAV_TIMER_ID);
	m_InitialScrollDownSize = InitialScrollDownSize;
	if(!m_Browser.Navigate(szUrl))
		return false;
	StartTimer(WAIT_FOR_NAV_TIMER_ID, WAIT_FOR_NAV_COMPLTETE_POLL_TIME);
	return true;
}

bool WebBrowseDialogPage::NavigateToNULL()
{
	StopTimer(WAIT_FOR_NAV_TIMER_ID);
	if(!m_Browser.Navigate("about:blank"))
		return false;
	return true;
}

void WebBrowseDialogPage::Close()
{
	m_Browser.Close();
	StopTimer(WAIT_FOR_NAV_TIMER_ID);
	CloseDialog();
}

bool WebBrowseDialogPage::UpdateBrowserSize(int Width, int Height)
{
	::MoveWindow(GetDialogHandle(), m_XOffset, m_YOffset, Width - 2 * m_XOffset, Height - m_YOffset - m_XOffset, TRUE);
	return m_Browser.UpdateBrowserSize(0, 0, Width - 2 * m_XOffset, Height - m_YOffset - m_XOffset);
}

bool WebBrowseDialogPage::NavigateBack()
{
	StopTimer(WAIT_FOR_NAV_TIMER_ID);
	return m_Browser.NavigateBack();
}

bool WebBrowseDialogPage::NavigateForward()
{
	StopTimer(WAIT_FOR_NAV_TIMER_ID);
	return m_Browser.NavigateForward();
}

bool WebBrowseDialogPage::GetCurrentUrl(std::string &rCurrentUrl)
{
	return m_Browser.GetCurrentUrl(rCurrentUrl);
}

LRESULT WebBrowseDialogPage::WindowProc(HWND UNUSED(hwnd), UINT uMsg, WPARAM UNUSED(wParam), LPARAM UNUSED(lParam), bool &fSkipDefault)
{
	fSkipDefault = false;
	LRESULT lResult = 0;

	switch(uMsg)
	{
	case WM_TIMER:
		{
			bool fComplete = false;
			bool fSuccess = m_Browser.CheckIfNavigationComplete(fComplete);
			
			if(!fSuccess || fComplete)
			{
				StopTimer(WAIT_FOR_NAV_TIMER_ID);
			}
			if(fSuccess && fComplete)
			{
				m_Browser.ScrollBy(0, m_InitialScrollDownSize);
			}
		}
		break;
	}
	return lResult;
}
