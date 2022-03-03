#include "Pch.h"
#include "NotificationAreaHandler.h"
#include "UtilityWindow.h"
#include "UtilityWindowParams.h"
#include "StringConvert.h"
#include "resource.h"

extern HMODULE g_hModule;

#define AQUALANG_ICON_ID 1

#pragma warning (disable:4355)

NotificationAreaHandler::NotificationAreaHandler(IMenuEventHandler &rMenuEventHandler, IConfigUpdateListener & UNUSED(rListener), UtilityWindow &rUtilityWindow)
	: m_rMenuEventHandler(rMenuEventHandler),
	m_rUtilityWindow(rUtilityWindow),
	m_fOpened(false),
	m_Menu(*this),
//	m_TipOfTheDay(m_TipOfTheDayScheduler, rListener),
	m_TipOfTheDayScheduler(rUtilityWindow, *this)
{
}

NotificationAreaHandler::~NotificationAreaHandler()
{
	Close();
}

bool NotificationAreaHandler::Open(bool fCalculatorEnabled)
{
	if(!m_fOpened)
	{
		m_rUtilityWindow.Register(*this);
		
		NOTIFYICONDATA NotificationData;
		memset(&NotificationData, 0, sizeof(NotificationData));
		NotificationData.cbSize = sizeof(NotificationData);
		NotificationData.hWnd = m_rUtilityWindow.GetHandle();
		NotificationData.uID = AQUALANG_ICON_ID;
		NotificationData.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
		NotificationData.hIcon = ::LoadIcon(g_hModule, MAKEINTRESOURCE(IDI_ICON_DROP));
		if(NotificationData.hIcon == NULL)
			return false;
		_tcscpy_s(NotificationData.szTip, sizeof(NotificationData.szTip) / sizeof(NotificationData.szTip[0]), _T("AquaLang"));
		NotificationData.uCallbackMessage = WM_NOTIFICATIONAREA;

		if(!::Shell_NotifyIcon(NIM_ADD, &NotificationData))
			return false;

		m_fOpened = true;

		if(!m_Menu.Open(IDR_MENU_NOTIFICATION_AREA))
			return false;
		m_Menu.EnableOption(ID_MENU_DISABLE, true);
		m_Menu.EnableOption(ID_MENU_ENABLE, false);
	}
	
	m_Menu.EnableOption(ID_MENU_CALCULATOR, fCalculatorEnabled);
	return true;
}

bool NotificationAreaHandler::OpenTipOfTheDay(LPCTSTR UserName)
{
//	if(!m_TipOfTheDay.Open(UserName))
//		return false;
	if(!m_TipOfTheDayScheduler.Open(UserName))
		return false;
	return true;
}

void NotificationAreaHandler::CloseTipOfTheDay()
{
	m_TipOfTheDayScheduler.Close();
//	m_TipOfTheDay.Close();
}

void NotificationAreaHandler::Close()
{
	CloseTipOfTheDay();

	m_rUtilityWindow.Unregister(*this);

	if(m_fOpened)
	{
		NOTIFYICONDATA NotificationData;
		memset(&NotificationData, 0, sizeof(NotificationData));
		NotificationData.cbSize = sizeof(NotificationData);
		NotificationData.hWnd = m_rUtilityWindow.GetHandle();
		NotificationData.uID = AQUALANG_ICON_ID;
		NotificationData.uFlags = 0;

		::Shell_NotifyIcon(NIM_DELETE, &NotificationData);

		m_fOpened = false;
	}
	m_Menu.Close();
}

void NotificationAreaHandler::Message(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_NOTIFICATIONAREA:
		if(wParam == AQUALANG_ICON_ID)
		{
			switch(lParam)
			{
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
				{
					CURSORINFO CursorInfo;
					CursorInfo.cbSize = sizeof(CursorInfo);
					if(::GetCursorInfo(&CursorInfo))
					{
						EnableTip(false);
						m_Menu.Popup(CursorInfo.ptScreenPos);
						EnableTip(true);
					}
				}
				break;
			}
		}
		break;
	}
}

void NotificationAreaHandler::OptionSelected(int OptionId)
{
	switch(OptionId)
	{
	case ID_MENU_DISABLE:
		m_Menu.EnableOption(ID_MENU_DISABLE, false);
		m_Menu.EnableOption(ID_MENU_ENABLE, true);
		ModifyIcon(IDI_ICON_DROP_DISABLED);
		break;
	case ID_MENU_ENABLE:
		m_Menu.EnableOption(ID_MENU_DISABLE, true);
		m_Menu.EnableOption(ID_MENU_ENABLE, false);
		ModifyIcon(IDI_ICON_DROP);
		break;
	}
	m_rMenuEventHandler.OptionSelected(OptionId);
}

bool NotificationAreaHandler::CanPresentTipOfTheDay()
{
//	return m_TipOfTheDay.CanPresent();
	return true;
}

void NotificationAreaHandler::PresentTipOfTheDay(LPCTSTR Text, bool UNUSED(fMoreNext), bool UNUSED(fMorePrev))
{
//	m_TipOfTheDay.Present(Text, fMoreNext, fMorePrev);
	PopupMessage(_T("AquaLang Tip of the Day"), Text);
}

bool NotificationAreaHandler::PresentTextRequest(const char *Title, const char *Text)
{
	_tstring TitleStr;
	_tstring TextStr;

	SetString(TitleStr, Title);
	SetString(TextStr, Text);
	return PopupMessage(TitleStr.c_str(), TextStr.c_str());
}

bool NotificationAreaHandler::PresentTextRequest(const wchar_t *wTitle, const wchar_t *wText)
{
	_tstring TitleStr;
	_tstring TextStr;

	SetString(TitleStr, wTitle);
	SetString(TextStr, wText);
	return PopupMessage(TitleStr.c_str(), TextStr.c_str());
}

bool NotificationAreaHandler::EnableTip(bool fEnable)
{
	NOTIFYICONDATA NotificationData;
	memset(&NotificationData, 0, sizeof(NotificationData));
	NotificationData.cbSize = sizeof(NotificationData);
	NotificationData.hWnd = m_rUtilityWindow.GetHandle();
	NotificationData.uID = AQUALANG_ICON_ID;
	NotificationData.uFlags = NIF_TIP;
	if(fEnable)
	{
		_tcscpy_s(NotificationData.szTip, sizeof(NotificationData.szTip) / sizeof(NotificationData.szTip[0]), _T("AquaLang"));
	}
	else
	{
		_tcscpy_s(NotificationData.szTip, sizeof(NotificationData.szTip) / sizeof(NotificationData.szTip[0]), _T(""));
	}
	if(!::Shell_NotifyIcon(NIM_MODIFY, &NotificationData))
		return false;
	return true;
}

bool NotificationAreaHandler::ModifyIcon(int IconId)
{
	NOTIFYICONDATA NotificationData;
	memset(&NotificationData, 0, sizeof(NotificationData));
	NotificationData.cbSize = sizeof(NotificationData);
	NotificationData.hWnd = m_rUtilityWindow.GetHandle();
	NotificationData.uID = AQUALANG_ICON_ID;
	NotificationData.uFlags = NIF_ICON;

	NotificationData.hIcon = ::LoadIcon(g_hModule, MAKEINTRESOURCE(IconId));
	if(NotificationData.hIcon == NULL)
		return false;

	if(!::Shell_NotifyIcon(NIM_MODIFY, &NotificationData))
		return false;
	return true;
}

bool NotificationAreaHandler::PopupMessage(LPCTSTR Title, LPCTSTR Text)
{
	NOTIFYICONDATA NotificationData;
	memset(&NotificationData, 0, sizeof(NotificationData));
	NotificationData.cbSize = sizeof(NotificationData);
	NotificationData.hWnd = m_rUtilityWindow.GetHandle();
	NotificationData.uID = AQUALANG_ICON_ID;
	NotificationData.uFlags = NIF_INFO;

	if(_tcslen(Title) >= sizeof(NotificationData.szInfoTitle) / sizeof(NotificationData.szInfoTitle[0]))
	{
		Log(_T("NotificationAreaHandler::PopupMessage - title requested exceeds 64 characters\n"));
		return false;
	}
	_tcscpy_s(NotificationData.szInfoTitle, sizeof(NotificationData.szInfoTitle) / sizeof(NotificationData.szInfoTitle[0]), Title);

	if(_tcslen(Text) >= sizeof(NotificationData.szInfo) / sizeof(NotificationData.szInfo[0]))
	{
		Log(_T("NotificationAreaHandler::PopupMessage - text requested exceeds 256 characters\n"));
		return false;
	}
	_tcscpy_s(NotificationData.szInfo, sizeof(NotificationData.szInfo) / sizeof(NotificationData.szInfo[0]), Text);

	if(!::Shell_NotifyIcon(NIM_MODIFY, &NotificationData))
		return false;
	return true;
}
