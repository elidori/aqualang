#include "Pch.h"
#include "ToolTip.h"

extern HMODULE g_hModule;

ToolTip::ToolTip()
	: m_hToolTip(NULL)
{
}

ToolTip::~ToolTip()
{
	Close();
}

bool ToolTip::Open(HWND hDialog, HWND hControlWindow, LPCTSTR Text)
{
	if(m_hToolTip != NULL)
		return false;
	m_hToolTip = ::CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
								WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
								CW_USEDEFAULT, CW_USEDEFAULT,
								CW_USEDEFAULT, CW_USEDEFAULT,
								hDialog, NULL, g_hModule,
								NULL);
	if(m_hToolTip == NULL)
		return false;

	TOOLINFO ToolInfo;
	memset(&ToolInfo, 0, sizeof(ToolInfo));
	ToolInfo.cbSize = sizeof(ToolInfo);
	ToolInfo.hwnd = hDialog;
	ToolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ToolInfo.uId = (UINT_PTR)hControlWindow;
	ToolInfo.lpszText = (LPTSTR)Text;
	if(!::SendMessage(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ToolInfo))
		return false;
	return true;
}

void ToolTip::Close()
{
	if(m_hToolTip)
	{
		::DestroyWindow(m_hToolTip);
		m_hToolTip = NULL;
	}
}
