#include "Pch.h"
#include "DialogIcon.h"

extern HMODULE g_hModule;

DialogIcon::DialogIcon(int IconId)
	: m_IconId(IconId),
	m_hIcon(NULL)
{
}

DialogIcon::~DialogIcon()
{
}

bool DialogIcon::SetToDialog(HWND hDialog)
{
	if(m_IconId == -1)
		return true;

	// load icon
	if(m_hIcon == NULL)
	{
		m_hIcon = ::LoadIcon(g_hModule, MAKEINTRESOURCE(m_IconId));
		if(m_hIcon == NULL)
			return false;
	}

	// set to dialog
	::SendMessage(hDialog, WM_SETICON, ICON_BIG, (LPARAM)m_hIcon);
	::SendMessage(hDialog, WM_SETICON, ICON_SMALL, (LPARAM)m_hIcon);
	return true;
}
