#include "Pch.h"
#include "FocusInfo.h"
#include "ComHelper.h"
#include "StringConvert.h"
#include "Log.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OleAccLibrary
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
OleAccLibrary::OleAccLibrary()
	: m_hLib(NULL),
	FuncObjectFromLresult(NULL)
{
}

OleAccLibrary::~OleAccLibrary()
{
	Close();
}

bool OleAccLibrary::Open()
{
	if(m_hLib != NULL)
		return true;

	m_hLib = ::LoadLibrary(_T("oleacc.dll"));
	if(m_hLib == NULL)
		return false;

	FuncObjectFromLresult = (LPFNOBJECTFROMLRESULT)::GetProcAddress(m_hLib, "ObjectFromLresult");
	if(FuncObjectFromLresult == NULL)
	{
		Close();
		return false;
	}
	return true;
}

bool OleAccLibrary::IsOpen() const
{
	return (m_hLib != NULL);
}

void OleAccLibrary::Close()
{
	FuncObjectFromLresult = NULL;
	if(m_hLib != NULL)
	{
		::FreeLibrary(m_hLib);
		m_hLib = NULL;
	}
}

HRESULT OleAccLibrary::ObjectFromLresult(LRESULT lResult, REFIID riid, WPARAM wParam, void** ppvObject)
{
	return (this->FuncObjectFromLresult)(lResult, riid, wParam, ppvObject);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FocusInfo
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
FocusInfo::FocusInfo()
	: m_fDataIsValid(false),
	m_hFocusWindow(NULL)
{
}

FocusInfo::~FocusInfo()
{
}

bool FocusInfo::EnableHTMLChecking()
{
	return m_OleAccLibrary.Open();
}

bool FocusInfo::UpdateCurrentInfo(bool fInHook, bool &rfFocusChanged)
{
	rfFocusChanged = false;
	bool fDataWasValid = m_fDataIsValid;
	HWND hPrevFocusWnd = m_hFocusWindow;

	m_fDataIsValid = false;

	HWND hForgroundWindow = ::GetForegroundWindow();
	m_GuiThreadInfo.cbSize = sizeof(m_GuiThreadInfo);
	if(!::GetGUIThreadInfo(::GetWindowThreadProcessId(hForgroundWindow, NULL), &m_GuiThreadInfo))
		return false;
	
	m_hFocusWindow = (m_GuiThreadInfo.hwndFocus != NULL) ? m_GuiThreadInfo.hwndFocus : hForgroundWindow;
	if(fDataWasValid && (hPrevFocusWnd != m_hFocusWindow))
	{
		rfFocusChanged = true;
	}

	if(!fInHook || rfFocusChanged)
	{
		_tstring SubObjectId;
		if(CheckHTML(m_hFocusWindow, SubObjectId))
		{
			if(!rfFocusChanged)
			{
				if(m_SubObjectId != SubObjectId)
				{
					rfFocusChanged = true;
				}
			}
			m_SubObjectId = SubObjectId;
		}
		else
		{
			m_SubObjectId = _T("");
		}
	}

	m_fDataIsValid = true;
	return true;
}

void FocusInfo::Clear()
{
	m_fDataIsValid = false;
	m_hFocusWindow = NULL;
}

HWND FocusInfo::QueryCurrentFocusWindow() const
{
	if(!m_fDataIsValid)
		return NULL;
	return m_hFocusWindow;
}

HWND FocusInfo::QueryCurrentCaretWindow() const
{
	if(!m_fDataIsValid)
		return NULL;
	return m_GuiThreadInfo.hwndCaret;
}

HWND FocusInfo::QueryCurrentEditingWindow() const
{
	if(!m_fDataIsValid)
		return NULL;
	if(m_GuiThreadInfo.hwndCaret != NULL)
		return m_GuiThreadInfo.hwndCaret;
	return m_hFocusWindow;
}

LPCTSTR FocusInfo::GetSubObjectId() const
{
	return m_SubObjectId.c_str();
}

bool FocusInfo::QueryCaretGlobalRect(RECT &rRect) const
{
	SetRect(&rRect, 0, 0, 0, 0);
	if(!m_fDataIsValid)
		return false;

	if(m_GuiThreadInfo.hwndCaret == NULL)
		return false;
	if(!::GetWindowRect(m_GuiThreadInfo.hwndCaret, &rRect))
		return false;

	rRect.left += m_GuiThreadInfo.rcCaret.left;
	rRect.top += m_GuiThreadInfo.rcCaret.top;
	rRect.right = rRect.left + (m_GuiThreadInfo.rcCaret.right - m_GuiThreadInfo.rcCaret.left);
	rRect.bottom = rRect.top + (m_GuiThreadInfo.rcCaret.bottom - m_GuiThreadInfo.rcCaret.top);

	return true;
}

bool FocusInfo::QueryWorkingRect(RECT &rRect, int &rRectType) const
{
	rRectType = WORKING_RECT_NONE;

	// try to see if caret rect exists
	if(QueryCaretGlobalRect(rRect))
	{
		rRectType = WORKING_RECT_CARET;
		return true;
	}

	// if no caret info exists, collect focus rect info and mouse position info
	bool fFocusWindowInfoExists = false;
	RECT FocusRect = { 0, 0, 0, 0 };
	if(m_fDataIsValid && m_hFocusWindow == NULL)
	{
		if(::GetWindowRect(m_hFocusWindow, &FocusRect))
		{
			fFocusWindowInfoExists = true;
		}
	}

	bool fCursorInfoExists = false;
	CURSORINFO CursorInfo;
	CursorInfo.cbSize = sizeof(CursorInfo);
	if(::GetCursorInfo(&CursorInfo))
	{
		fCursorInfoExists = true;
	}

	if(fFocusWindowInfoExists)
	{
		if(fCursorInfoExists && IsCursorInWindow(CursorInfo.ptScreenPos, FocusRect))
		{
			// follow mouse position
			SetRect(&rRect, CursorInfo.ptScreenPos.x, CursorInfo.ptScreenPos.y, CursorInfo.ptScreenPos.x, CursorInfo.ptScreenPos.y);
			rRectType = WORKING_RECT_MOUSE;
		}
		else
		{
			// top left of focus window
			SetRect(&rRect, FocusRect.left, FocusRect.top, FocusRect.left, FocusRect.top);
			rRectType = WORKING_RECT_TOP_LEFT_WINDOW;
		}
	}
	else
	{
		if(fCursorInfoExists)
		{
			// follow mouse position
			SetRect(&rRect, CursorInfo.ptScreenPos.x, CursorInfo.ptScreenPos.y, CursorInfo.ptScreenPos.x, CursorInfo.ptScreenPos.y);
			rRectType = WORKING_RECT_MOUSE;
		}
		else
		{
			return false;
		}
	}
	return true;
}

HKL FocusInfo::QueryCurrentLanguage() const
{
	if(!m_fDataIsValid)
		return NULL;
	return ::GetKeyboardLayout(GetWindowThreadProcessId(m_hFocusWindow, NULL));
}

bool FocusInfo::IsCursorInWindow(const POINT &CursorPos, const RECT &WindowRect) const
{
	if(CursorPos.x < WindowRect.left || CursorPos.x >= WindowRect.right)
		return false;
	if(CursorPos.y < WindowRect.top || CursorPos.y >= WindowRect.bottom)
		return false;
	return true;
}

bool FocusInfo::CheckHTML(HWND hwnd, _tstring &rSubObjectId)
{
	rSubObjectId = _T("");

	if(!m_OleAccLibrary.IsOpen())
		return false;

	LRESULT lRes;
	UINT nMsg = ::RegisterWindowMessage(_T("WM_HTML_GETOBJECT"));
	::SendMessageTimeout(hwnd, nMsg, 0L, 0L, SMTO_ABORTIFHUNG, 1000, (PDWORD_PTR)&lRes );
	if(lRes == 0)
		return false;

	ReleasableItem<IHTMLDocument2> pDoc;
	HRESULT hr = m_OleAccLibrary.ObjectFromLresult(lRes, IID_IHTMLDocument2, 0, (void**)&pDoc);
	if(FAILED(hr) || pDoc == NULL)
		return false;

	ReleasableItem<IHTMLElement> pActiveElement;
	hr = pDoc->get_activeElement(&pActiveElement);
	if(FAILED(hr) || pActiveElement == NULL)
		return true;

	BSTR bstrId = NULL;
	hr = pActiveElement->get_id(&bstrId);
	if(FAILED(hr))
		return true;

	if(bstrId != NULL)
	{
		SetString(rSubObjectId, LPCWSTR(bstrId));
		::SysFreeString(bstrId);
	}
	return true;
}
