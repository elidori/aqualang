#include "Pch.h"
#include "FloatingMenu.h"
#include "Log.h"

extern HMODULE g_hModule;

FloatingMenu::FloatingMenu(IMenuEventHandler &rEventHandler)
	: m_rEventHandler(rEventHandler),
	m_hWindow(NULL),
	m_OldWndProc(NULL),
	m_hMenu(NULL)
{
}

FloatingMenu::~FloatingMenu()
{
	Close();
}

bool FloatingMenu::Open(int MenuId)
{
	Close();

	if(!OpenWindow())
		return false;

	m_hMenu = ::LoadMenu(g_hModule, MAKEINTRESOURCE(MenuId));
	if(m_hMenu == NULL)
		return false; 

	return true;
}

bool FloatingMenu::Popup(const POINT &rSubjectPoint)
{
	if(m_hMenu == NULL || m_hWindow == NULL)
		return false;

	HMENU hPopupMenu = ::GetSubMenu(m_hMenu, 0);
	if(hPopupMenu == NULL)
		return false;

	// determine the menu alignment according to the subject point in the screen
	RECT rctDesktop;
	if(!::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rctDesktop, NULL))
		return false;
	int CenterScreenX = (rctDesktop.left + rctDesktop.right) / 2;
	int CenterScreenY = (rctDesktop.top + rctDesktop.bottom) / 2;

	int uFlags = 0;
	uFlags |= (rSubjectPoint.x > CenterScreenX) ? TPM_RIGHTALIGN : TPM_LEFTALIGN;
	uFlags |= (rSubjectPoint.y > CenterScreenY) ? TPM_BOTTOMALIGN : TPM_TOPALIGN;

	// display the pop up menu
	::SetForegroundWindow(m_hWindow); //a patch according to msdn guidelines
	if(!::TrackPopupMenu(hPopupMenu, uFlags | TPM_LEFTBUTTON, rSubjectPoint.x, rSubjectPoint.y, 0, m_hWindow, NULL))
		return false;
	::PostMessage(m_hWindow, WM_NULL, 0, 0); // a patch according to msdn guidelines

	return true;
}

bool FloatingMenu::EnableOption(int OptionId, bool fEnable)
{
	if(m_hMenu == NULL)
		return false;
	HMENU hPopupMenu = ::GetSubMenu(m_hMenu, 0);
	if(hPopupMenu == NULL)
		return false;

	MENUITEMINFO Info;
	Info.cbSize = sizeof(Info);
	Info.fMask = MIIM_STATE;
	if(!::GetMenuItemInfo(hPopupMenu, OptionId, FALSE, &Info))
		return false;
	if(fEnable)
	{
		Info.fState &= ~MFS_DISABLED;
	}
	else
	{
		Info.fState |= MFS_DISABLED;
	}
	if(!::SetMenuItemInfo(hPopupMenu, OptionId, FALSE, &Info))
		return false;
	return true;
}

void FloatingMenu::Close()
{
	CloseWindow();
	if(m_hMenu != NULL)
	{
		::DestroyMenu(m_hMenu);
		m_hMenu = NULL;
	}
}

LRESULT CALLBACK FloatingMenu::ThunkWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    FloatingMenu *pThis = (FloatingMenu *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if(pThis)
		return pThis->WindowProc(hwnd, uMsg, wParam, lParam);
	return 0;
}

LRESULT FloatingMenu::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	assert(hwnd == m_hWindow);
	WNDPROC OldWndProc = m_OldWndProc;

	switch(uMsg)
	{
	case WM_COMMAND:
		m_rEventHandler.OptionSelected(LOWORD(wParam));
		break;
	}

	return ::CallWindowProc(OldWndProc, hwnd, uMsg, wParam, lParam);
}

bool FloatingMenu::OpenWindow()
{
	CloseWindow();

	m_hWindow = ::CreateWindowEx(0, _T("BUTTON"), NULL, WS_POPUP, 0,0,0,0, NULL, NULL, NULL, NULL);
	if(!m_hWindow)
	{
		Log(_T("FloatingMenu::OpenWindow - Failed opening window\n"));
		return false;
	}

	::SetWindowLongPtr(m_hWindow, GWLP_USERDATA, (LONG_PTR)this);
	m_OldWndProc = (WNDPROC)::SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, (LONG_PTR)&ThunkWindowProc);

	return true;
}

void FloatingMenu::CloseWindow()
{
	if(m_OldWndProc != NULL)
	{
		::SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, (LONG_PTR)m_OldWndProc);
		m_OldWndProc = NULL;
	}
	if(m_hWindow != NULL)
	{
		::DestroyWindow(m_hWindow);
		m_hWindow = NULL;
	}
}
