#include "Pch.h"
#include "TransparentEditBox.h"
#include "Log.h"

TransparentEditBox::TransparentEditBox(IEditBoxListener *pListener)
	: m_pEditListener(pListener)
{
}

TransparentEditBox::~TransparentEditBox()
{
	Close();
}

bool TransparentEditBox::Open(HWND hWindow)
{
	Close();

	m_hWindow = hWindow;
	::SetWindowLongPtr(m_hWindow, GWLP_USERDATA, (LONG_PTR)this);
	m_OldWndProc = (WNDPROC)::SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, (LONG_PTR)&ThunkWindowProc);
	return true;
}

void TransparentEditBox::Close()
{
	if(m_OldWndProc != NULL)
	{
		::SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, (LONG_PTR)&m_OldWndProc);
		m_OldWndProc = NULL;
	}
	m_hWindow = NULL;
}

LRESULT CALLBACK TransparentEditBox::ThunkWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TransparentEditBox *pThis = (TransparentEditBox *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if(pThis)
	{
		return pThis->WindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

LRESULT TransparentEditBox::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	assert(hwnd == m_hWindow);
	WNDPROC OldWndProc = m_OldWndProc;

	switch(uMsg)
	{
	case WM_KILLFOCUS:
		if(m_pEditListener != NULL)
		{
			m_pEditListener->NotifyKeyDown(hwnd, (int)wParam);
		}
		break;
	case WM_KEYDOWN:
		if(m_pEditListener != NULL)
		{
			m_pEditListener->NotifyKeyDown(hwnd, (int)wParam);
		}
		break;
	}
	LRESULT lResult = ::CallWindowProc(OldWndProc, hwnd, uMsg, wParam, lParam);
	
	return lResult;
}
