#include "Pch.h"
#include "WebBrowseDialog.h"
#include "SimpleOleClientSite.h"
#include "resource.h"
#include "StringConvert.h"
#include "UtilityWindow.h"
#include "UtilityWindowParams.h"
#include "ComHelper.h"
#include "ScreenFitter.h"
#include "WebBrowseDialogPage.h"
#include "Log.h"

enum
{
	WM_WEB_BROWSER_MSG = WM_USER + 11,
	WPARAM_WEB_BROWSE_TERMINATE = 1
};

enum
{
	DEFAULT_WEB_SEARCH_WINDOW_WIDTH = 400,
	DEFAULT_WEB_SEARCH_WINDOW_HEIGHT = 400,
};

#define MAX_DIALOG_TITLE_LENGTH 50

#define MIN_TEXT_LENGTH_FOR_WIDE_DIALOG 20
#define DEFAULT_DIALOG_WIDTH 512
#define WIDE_DIALOG_WIDTH 760

WebBrowseDialog::WebBrowseDialog(IBrowserOwner &rOwner)
	: DialogObject(IDI_ICON_DROP, false),
	m_rOwner(rOwner),
	m_WebBrowserXml(),
	m_fComInitialized(false),
	m_pSearchBrowser(NULL),
	m_pTranslateBrowser(NULL),
	m_BrowserOffsetFromDialogTop(0)
{
	SetRect(&m_RecentWindowRect, 0, 0, 0, 0);
}

WebBrowseDialog::~WebBrowseDialog()
{
	Close();
}

bool WebBrowseDialog::Open(LPCTSTR UserName, LPCTSTR Title)
{
	m_WebBrowserXml.Load(UserName);

	HWND hDialog = OpenDialog(UserName, IDD_DIALOG_WEB_BROWSE);
	if(hDialog == NULL)
		return false;

	SetTitle(Title);
	CalcOffsetFromDialogTop();

	if(!m_fComInitialized)
	{
		m_fComInitialized = SUCCEEDED(CoInitialize(NULL));
		if(!m_fComInitialized)
			return false;
	}

	HWND hForgroundWindow = ::GetForegroundWindow();

	// Add brows e pages
	HWND hPageWnd = ::GetDlgItem(hDialog, IDC_TAB_WEB_BROWSE);
	if(hPageWnd == NULL)
		return false;
	m_DialogTab.Open(hPageWnd);

	bool fSuccess = true;

	m_pSearchBrowser = new WebBrowseDialogPage;
	fSuccess &= AddPage(hPageWnd, m_pSearchBrowser, _T("Search"));

	m_pTranslateBrowser = new WebBrowseDialogPage;
	fSuccess &= AddPage(hPageWnd, m_pTranslateBrowser, _T("Translate"));

	RECT ClientRect;
	if(::GetClientRect(hDialog, &ClientRect))
	{
		UpdateBrowserSize(ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top);
	}

	if(m_WebBrowserXml.ShouldOpenAsSearcher())
	{
		m_DialogTab.ActivatePage(m_pSearchBrowser);
	}
	else
	{
		m_DialogTab.ActivatePage(m_pTranslateBrowser);
	}

	HideDialog();

	::SetForegroundWindow(hForgroundWindow);

	return true;
}

bool WebBrowseDialog::Browse(LPCSTR szSearchUrl, int SearchInitialScrollDownOffset, LPCSTR szTranslateUrl, int TranslateInitialScrollDownOffset, LPCTSTR SearchString)
{
	// set title to the dialog. Make sure that the string is not too long and that no \n are found in it
	_tstring Caption = _T("AquaLang Web Browser: ");
	Caption += SearchString;
	if(Caption.length() > MAX_DIALOG_TITLE_LENGTH)
	{
		Caption = Caption.substr(0, MAX_DIALOG_TITLE_LENGTH - 3) + _T("...");
	}
	index_t NewLinePos = (index_t)Caption.find(_T('\n'));
	if(NewLinePos != -1)
	{
		Caption = Caption.substr(0, NewLinePos) + _T("...");
	}
	index_t CarriageReturnPos = Caption.find(_T('\r'));
	if(CarriageReturnPos != -1)
	{
		Caption = Caption.substr(0, CarriageReturnPos) + _T("...");
	}

	SetTitle(Caption.c_str());

	bool fSuccess = true;
	if(m_pSearchBrowser)
	{
		fSuccess &= m_pSearchBrowser->Navigate(szSearchUrl, SearchInitialScrollDownOffset);
	}
	if(m_pTranslateBrowser)
	{
		fSuccess &= m_pTranslateBrowser->Navigate(szTranslateUrl, TranslateInitialScrollDownOffset);
	}
	return fSuccess;
}

bool WebBrowseDialog::Display(const RECT & UNUSED(rCaretRect), size_t UNUSED(nTextLength))
{
	HWND hDialog = GetDialogHandle();
	if(hDialog == NULL)
		return false;

/*
	RECT rctDesktop;
	::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rctDesktop, NULL);
	long Width = DEFAULT_DIALOG_WIDTH;
	if(nTextLength > MIN_TEXT_LENGTH_FOR_WIDE_DIALOG)
	{
		Width = WIDE_DIALOG_WIDTH;
	}
	long Height = ((rctDesktop.bottom - rctDesktop.top) / 2);

	RECT WindowRect;
	if(!ScreenFitter::PlaceInScreen(Width, Height, rCaretRect, WindowRect))
		return false;
*/
	RECT WindowRect = m_RecentWindowRect;

	WINDOWPLACEMENT WindowPlacement;
	WindowPlacement.length = sizeof(WindowPlacement);
	::GetWindowPlacement(hDialog, &WindowPlacement);
	SetRect(
		&WindowPlacement.rcNormalPosition,
		WindowRect.left,
		WindowRect.top,
		WindowRect.right,
		WindowRect.bottom);
	::SetWindowPlacement(hDialog, &WindowPlacement);

	::SetWindowPos(
			hDialog,
			HWND_TOP,
			WindowRect.left,
			WindowRect.top,
			WindowRect.right - WindowRect.left,
			WindowRect.bottom - WindowRect.top,
			SWP_SHOWWINDOW
			);
	::ShowWindow(hDialog, SW_NORMAL); // to restore from minimized or maximized mode
	::SetForegroundWindow(hDialog);
	::InvalidateRect(hDialog, NULL, TRUE);

	return true;
}

void WebBrowseDialog::Close()
{
	if(IsDialogOpen())
	{
		m_WebBrowserXml.Store();
	}

	m_pSearchBrowser = NULL;
	m_pTranslateBrowser = NULL;
	m_DialogTab.Close();
	BrowserMap::iterator Iterator = m_BrowserMap.begin();
	for(; Iterator != m_BrowserMap.end(); Iterator++)
	{
		delete (*Iterator).second;
	}
	m_BrowserMap.erase(m_BrowserMap.begin(), m_BrowserMap.end());
	if(m_fComInitialized)
	{
		CoUninitialize();
		m_fComInitialized = false;
	}

	CloseDialog();
}

void WebBrowseDialog::TerminateRequest(HWND hwnd)
{
	::PostMessage(hwnd,	WM_WEB_BROWSER_MSG, WPARAM_WEB_BROWSE_TERMINATE, 0);
}

/*
bool WebBrowseDialog::GetHTMLRect(long &rWidth, long &rHeight, bool &rfPending)
{
	// notes:
	// 1. Only when a navigation to a url succeeds and there is an html document, this function shall succeed
	// 2. The measurement is the maximum of all elements right and bottom edges. This suggests that it is better that the scroll
	//    is left aligned. well, it may not be that way when the language is right to left.
	rfPending = false;
	rWidth = DEFAULT_WEB_SEARCH_WINDOW_WIDTH;
	rHeight = DEFAULT_WEB_SEARCH_WINDOW_HEIGHT;

	if(!m_pWebObject)
	{
		Log(_T("WebBrowseDialog::GetHTMLRect - m_pWebObject=NULL\n"));
		return false;
	}

	ReleasableItem<IWebBrowser2> pBrowser;
	HRESULT hr = m_pWebObject->QueryInterface(IID_IWebBrowser2,(void**)&pBrowser);
	if(FAILED(hr) || pBrowser == NULL)
	{
		Log(_T("WebBrowseDialog::GetHTMLRect - cannot get IWebBrowser2 interface\n"));
		return false;
	}

	ReleasableItem<IDispatch> pDispatch;
	hr = pBrowser->get_Document(&pDispatch);

	if(pDispatch == NULL)
	{
		if(SUCCEEDED(hr))
		{
			rfPending = true;
		}
		Log(_T("WebBrowseDialog::GetHTMLRect - failed getting document interface. rfPending=%d\n"), rfPending);
		return false;
	}
	ReleasableItem<IHTMLDocument2> pDoc;
	hr = pDispatch->QueryInterface(IID_IHTMLDocument2,(void**)&pDoc);
	if(FAILED(hr) || pDoc == NULL)
		return false;

	ReleasableItem<IHTMLElementCollection> pAll;
	hr = pDoc->get_all(&pAll);
	if(FAILED(hr) || pAll == NULL)
		return false;
	ReleasableItem<IUnknown> pUnknown;
	hr = pAll->get__newEnum(&pUnknown);
	if(FAILED(hr) || pUnknown == NULL)
		return false;
	ReleasableItem<IEnumVARIANT> pEnumVar;
	hr = pUnknown->QueryInterface(IID_IEnumVARIANT, (void **)&pEnumVar);
	if(FAILED(hr) || pEnumVar == NULL)
		return false;

	long Width = 0;
	long Height = 0;

	VARIANT Var;
	ULONG Fethced;
	pEnumVar->Next(1, &Var, &Fethced);

	while(Fethced == 1)
	{
		pEnumVar->Next(1, &Var, &Fethced);
		
		if(Var.vt == VT_DISPATCH)
		{
			ReleasableItem<IHTMLElement> pElement;
			hr = Var.pdispVal->QueryInterface(IID_IHTMLElement, (void **)&pElement);
			if(SUCCEEDED(hr))
			{
				long ElementWidth;
				long ElementLeft;
				long ElementHeight;
				long ElementTop;
				hr = pElement->get_offsetWidth(&ElementWidth);
				hr = pElement->get_offsetLeft(&ElementLeft);
				hr = pElement->get_offsetHeight(&ElementHeight);
				hr = pElement->get_offsetTop(&ElementTop);

				Width = max(ElementWidth + ElementLeft, Width);
				Height = max(ElementHeight + ElementTop, Height);
			}
		}
		VariantClear(&Var);
	}

	if(Width > 0)
	{
		rWidth = Width;
	}
	if(Height > 0)
	{
		rHeight = Height;
	}

	return true;
}
*/

void WebBrowseDialog::CalcOffsetFromDialogTop()
{
	// calculate the top of the browser rect reltive to the dialog client rect
	m_BrowserOffsetFromDialogTop = 0;
	HWND hBackButton = ::GetDlgItem(GetDialogHandle(), IDC_BUTTON_BACK);
	if(hBackButton)
	{
		POINT DialogClientPt = {0, 0};
		POINT ButtonPt = {0, 0};
		RECT ButtonRect;

		if(
			::ClientToScreen(GetDialogHandle(), &DialogClientPt) &&
			::ClientToScreen(hBackButton, &ButtonPt) &&
			::GetClientRect(hBackButton, &ButtonRect)
			)
		{
			m_BrowserOffsetFromDialogTop = (ButtonRect.bottom - ButtonRect.top) + 2 * (ButtonPt.y - DialogClientPt.y);
		}
	}
}

void WebBrowseDialog::HideDialog()
{
	::GetWindowRect(GetDialogHandle(), &m_RecentWindowRect);

	Show(false);

	BrowserMap::iterator Iterator = m_BrowserMap.begin();
	for(; Iterator != m_BrowserMap.end(); Iterator++)
	{
		(*Iterator).second->NavigateToNULL();
	}
}

bool WebBrowseDialog::UpdateBrowserSize(int Width, int Height)
{
	::MoveWindow(::GetDlgItem(GetDialogHandle(), IDC_TAB_WEB_BROWSE), 0, m_BrowserOffsetFromDialogTop, Width, Height - m_BrowserOffsetFromDialogTop, TRUE);

	bool fSuccess = true;
	BrowserMap::iterator Iterator = m_BrowserMap.begin();
	for(; Iterator != m_BrowserMap.end(); Iterator++)
	{
		fSuccess &= (*Iterator).second->UpdateBrowserSize(Width, Height - m_BrowserOffsetFromDialogTop);
	}
	return fSuccess;
}

LRESULT WebBrowseDialog::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault)
{
	fSkipDefault = false;
	LRESULT lResult = 0;

	switch(uMsg)
	{
	case WM_COMMAND:
		if((HWND)lParam == ::GetDlgItem(hwnd, IDCANCEL))
		{
			HideDialog();
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_BACK))
		{
			WebBrowseDialogPage *pActivePage = GetActiveBrowserPage();
			if(pActivePage)
			{
				pActivePage->NavigateBack();
			}
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_FORWARD))
		{
			WebBrowseDialogPage *pActivePage = GetActiveBrowserPage();
			if(pActivePage)
			{
				pActivePage->NavigateForward();
			}
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_OPEN_DEFAULT_BROWSER))
		{
			WebBrowseDialogPage *pActivePage = GetActiveBrowserPage();
			if(pActivePage)
			{
				std::string CurrentUrl;
				if(pActivePage->GetCurrentUrl(CurrentUrl))
				{
					::ShellExecuteA(NULL, "open", CurrentUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
				}
			}
		}
		break;
	case WM_NOTIFY:
		// Tab change
		if(wParam == IDC_TAB_WEB_BROWSE)
		{
			m_DialogTab.OnNotifySelectChange(*(NMHDR *)lParam);
			m_WebBrowserXml.OpenAsSearcher(GetActiveBrowserPage() == m_pSearchBrowser);
		}
		break;
	case WM_SIZE:
		UpdateBrowserSize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_WEB_BROWSER_MSG:
		if(wParam == WPARAM_WEB_BROWSE_TERMINATE)
		{
			m_rOwner.Terminate();
		}
		break;
	}
	return lResult;
}

bool WebBrowseDialog::AddPage(HWND hPageWnd, WebBrowseDialogPage *pPage, LPCTSTR Name)
{
	if(!pPage->Open(hPageWnd))
		return false;

	if(!m_DialogTab.AddPage(pPage, Name))
		return false;

	m_BrowserMap.insert(BrowserMap::value_type((int)(DialogObject *)pPage, pPage));

	return true;
}

WebBrowseDialogPage *WebBrowseDialog::GetActiveBrowserPage()
{
	DialogObject *pActiveDialog = m_DialogTab.GetActivePage();
	if(!pActiveDialog)
		return NULL;
	BrowserMap::iterator Iterator = m_BrowserMap.find((int)pActiveDialog);
	if(Iterator == m_BrowserMap.end())
		return NULL;
	return (*Iterator).second;
}
