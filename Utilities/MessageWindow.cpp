#include "Pch.h"
#include "MessageWindow.h"

MessageWindow::MessageWindow()
	: m_hMessageWnd(NULL),
	m_OldWndProc(NULL)
{
}

MessageWindow::~MessageWindow()
{
	CloseMessageWindow();
}

bool MessageWindow::OpenMessageWindow(LPCTSTR Title /*= _T("")*/)
{
	m_hMessageWnd = CreateWindowEx(0, _T("BUTTON"), Title, WS_POPUP, 0,0,0,0, NULL, NULL, NULL, NULL);
	if(m_hMessageWnd == NULL)
		return false;
	::SetWindowLongPtr(m_hMessageWnd, GWLP_USERDATA, (LONG_PTR)this);
	m_OldWndProc = (WNDPROC)::SetWindowLongPtr(m_hMessageWnd, GWLP_WNDPROC, (LONG_PTR)&ThunkWindowProc);
	return true;
}

void MessageWindow::CloseMessageWindow()
{
	if(m_OldWndProc != NULL)
	{
		::SetWindowLongPtr(m_hMessageWnd, GWLP_WNDPROC, (LONG_PTR)m_OldWndProc);
		m_OldWndProc = NULL;
	}
	if(m_hMessageWnd)
	{
		::DestroyWindow(m_hMessageWnd);
		m_hMessageWnd = NULL;
	}
}

void MessageWindow::PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	::PostMessage(m_hMessageWnd, uMsg, wParam, lParam);
}

LRESULT MessageWindow::SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ::SendMessage(m_hMessageWnd, uMsg, wParam, lParam);
}

LRESULT MessageWindow::WindowProc(HWND UNUSED(hwnd), UINT UNUSED(uMsg), WPARAM UNUSED(wParam), LPARAM UNUSED(lParam), bool &rfContinue)
{
	rfContinue = true;
	return TRUE;
}

LRESULT CALLBACK MessageWindow::ThunkWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    MessageWindow *pThis = (MessageWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if(pThis)
		return pThis->MyWindowProc(hwnd, uMsg, wParam, lParam);
	return 0;
}

LRESULT MessageWindow::MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	assert(hwnd == m_hMessageWnd);
	WNDPROC OldWndProc = m_OldWndProc;

	bool fContinue = true;
	LRESULT lResult = WindowProc(hwnd, uMsg, wParam, lParam, fContinue);
	if(!fContinue)
	{
		return lResult;
	}
	return ::CallWindowProc(OldWndProc, hwnd, uMsg, wParam, lParam);
}
