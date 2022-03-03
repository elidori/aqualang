#include "Pch.h"
#include "WebBrowseControl.h"
#include "SimpleOleClientSite.h"
#include "StringConvert.h"
#include "ComHelper.h"
#include "Log.h"

WebBrowseControl::WebBrowseControl()
	: m_pWebObject(NULL),
	m_pSite(NULL)
{
}

WebBrowseControl::~WebBrowseControl()
{
	Close();
}

bool WebBrowseControl::Open(HWND hDialog)
{
	Close();

	m_pSite = new SimpleOleClientSite(hDialog);
	if(!m_pSite)
		return false;

	HRESULT hr = OleCreate(CLSID_WebBrowser, IID_IOleObject, OLERENDER_DRAW, 0, m_pSite, &m_Storage, (void**)&m_pWebObject);
	if(FAILED(hr))
		return false;
	hr = m_pWebObject->SetClientSite(m_pSite);
	if(FAILED(hr))
		return false;
	RECT rect;
	if(!::GetClientRect(hDialog, &rect))
		return false;
	hr = m_pWebObject->DoVerb(OLEIVERB_SHOW, NULL, m_pSite, 0, hDialog, &rect);
	if(FAILED(hr))
		return false;
	return true;
}

bool WebBrowseControl::Navigate(LPCSTR szUrl)
{
	if(m_pWebObject == NULL)
		return false;

	ReleasableItem<IWebBrowser2> pBrowser;
	HRESULT hr = m_pWebObject->QueryInterface(IID_IWebBrowser2,(void**)&pBrowser);
	if(FAILED(hr))
		return false;

	VARIANT vURL;
	vURL.vt = VT_BSTR;
	std::wstring wURL;
	SetString(wURL, szUrl);
	vURL.bstrVal = ::SysAllocStringLen(wURL.c_str(), (UINT)wURL.size());

	VARIANT ve1, ve2, ve3, ve4;
	ve1.vt = VT_EMPTY;
	ve2.vt = VT_EMPTY;
	ve3.vt = VT_EMPTY;
	ve4.vt = VT_EMPTY;

	hr = pBrowser->Navigate2(&vURL, &ve1, &ve2, &ve3, &ve4);
	
	::VariantClear(&vURL);
	if(FAILED(hr))
		return false;

	return true;
}

bool WebBrowseControl::CheckIfNavigationComplete(bool &rfComplete)
{
	rfComplete = false;
	if(m_pWebObject == NULL)
		return false;

	ReleasableItem<IWebBrowser2> pBrowser;
	HRESULT hr = m_pWebObject->QueryInterface(IID_IWebBrowser2,(void**)&pBrowser);
	if(FAILED(hr))
		return false;

	READYSTATE ReadState;
	hr = pBrowser->get_ReadyState(&ReadState);
	if(FAILED(hr))
		return false;

	rfComplete = (ReadState == READYSTATE_COMPLETE);
	return true;
}

void WebBrowseControl::Close()
{
	if(m_pWebObject)
	{
		m_pWebObject->SetClientSite(NULL); // it is important to call this for enabling a safe close of the hosting window
		m_pWebObject->Release();
		m_pWebObject = false;
	}
	if(m_pSite)
	{
		delete m_pSite;
		m_pSite = NULL;
	}
}

bool WebBrowseControl::UpdateBrowserSize(int X, int Y, int Width, int Height)
{
	if(!m_pWebObject)
		return false;;
	ReleasableItem<IOleInPlaceObject> pInPlaceObject;
	HRESULT hr = m_pWebObject->QueryInterface(IID_IOleInPlaceObject, (void **)&pInPlaceObject);
	if(FAILED(hr))
		return false;

	RECT Rect;
	::SetRect(&Rect, X, Y, Width, Height);
	hr = pInPlaceObject->SetObjectRects(&Rect, &Rect);
	if(FAILED(hr))
		return false;

	return true;
}

bool WebBrowseControl::ScrollBy(int X, int Y)
{
	if(m_pWebObject == NULL)
		return false;

	ReleasableItem<IWebBrowser2> pBrowser;
	HRESULT hr = m_pWebObject->QueryInterface(IID_IWebBrowser2,(void**)&pBrowser);
	if(FAILED(hr))
		return false;

	ReleasableItem<IDispatch> pDispatch;
	hr = pBrowser->get_Document(&pDispatch);
	if(pDispatch == NULL)
		return false;

	ReleasableItem<IHTMLDocument2> pDocument;
	hr = pDispatch->QueryInterface(IID_IHTMLDocument2,(void**)&pDocument);
	if(FAILED(hr) || pDocument == NULL)
		return false;

	ReleasableItem<IHTMLWindow2> pWindow;
	hr = pDocument->get_parentWindow(&pWindow);
	if(FAILED(hr) || pWindow == NULL)
		return false;

	hr = pWindow->scrollBy(X, Y);
	if(FAILED(hr))
		return false;
	return true;
}

bool WebBrowseControl::NavigateBack()
{
	if(!m_pWebObject)
		return false;
	ReleasableItem<IWebBrowser2> pBrowser;
	HRESULT hr = m_pWebObject->QueryInterface(IID_IWebBrowser2,(void**)&pBrowser);
	if(FAILED(hr))
		return false;

	hr = pBrowser->GoBack();

	return SUCCEEDED(hr);
}

bool WebBrowseControl::NavigateForward()
{
	if(!m_pWebObject)
		return false;
	ReleasableItem<IWebBrowser2> pBrowser;
	HRESULT hr = m_pWebObject->QueryInterface(IID_IWebBrowser2,(void**)&pBrowser);
	if(FAILED(hr))
		return false;

	hr = pBrowser->GoForward();

	return SUCCEEDED(hr);
}

bool WebBrowseControl::GetCurrentUrl(std::string &rCurrentUrl)
{
	if(!m_pWebObject)
		return false;
	ReleasableItem<IWebBrowser2> pBrowser;
	HRESULT hr = m_pWebObject->QueryInterface(IID_IWebBrowser2,(void**)&pBrowser);
	if(FAILED(hr))
		return false;

	BSTR bstrURL = NULL;
	hr = pBrowser->get_LocationURL(&bstrURL);

	if(FAILED(hr))
		return false;
	SetString(rCurrentUrl, (LPCWSTR)bstrURL);
	::SysFreeString(bstrURL);
	return true;
}
