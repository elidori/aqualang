#include "Pch.h"
#include "KeyboardEventInfo.h"
#include "FocusInfo.h"
#include "ClipboardAccess.h"
#include "LanguageDetector.h"
#include "StringConvert.h"
#include "KeyboardSequenceEvent.h"
#include "WindowItemDataBase.h"

KeyboardEventInfo::KeyboardEventInfo(HWND hCurrentWindow, const FocusInfo &rFocusInfo, const LanguageDetector &rLanguageDetector, const KeyboardSequenceEvent &rKeyboardSequenceEvent, const WindowItemDataBase &rWindowsDatabase)
	: m_hCurrentWindow(hCurrentWindow),
	m_rFocusInfo(rFocusInfo),
	m_rLanguageDetector(rLanguageDetector),
	m_rKeyboardSequenceEvent(rKeyboardSequenceEvent),
	m_fClipboardWasPeeked(false),
	m_rWindowsDatabase(rWindowsDatabase)
{
}

KeyboardEventInfo::~KeyboardEventInfo()
{
}

const WindowItem *KeyboardEventInfo::GetWindowItem(const WindowItem** ppWindowRule) const
{
	return m_rWindowsDatabase.QueryWindowItem(m_hCurrentWindow, ppWindowRule);
}

bool KeyboardEventInfo::PeekAndLoadMarkedText()
{
	if(!m_fClipboardWasPeeked)
	{
		if(!ClipboardAccess::PeekMarkedText(m_rFocusInfo.QueryCurrentEditingWindow(), m_ClipboardText))
			return false;
		
		if(!m_rLanguageDetector.Detect(m_ClipboardText.c_str(), m_hTextKL))
		{
			// could not detect. guess according to current language
			m_hTextKL = ::GetKeyboardLayout(GetWindowThreadProcessId(m_rFocusInfo.QueryCurrentEditingWindow(), NULL));
		}

		m_fClipboardWasPeeked = true;
	}
	return true;
}

LPCWSTR KeyboardEventInfo::PeekMarkedText(HKL &rhTextKL) const
{
	rhTextKL = NULL;
	if(!m_fClipboardWasPeeked)
		return L"";
	rhTextKL = m_hTextKL;
	return m_ClipboardText.c_str();
}

bool KeyboardEventInfo::GetMarkedOrRecentText(_tstring &rText, HKL &rhTextKL, bool fExcludeOlderText) const
{
	rhTextKL = NULL;
	LPCWSTR wMarkedText = PeekMarkedText(rhTextKL);
	if(wMarkedText != NULL && wcslen(wMarkedText) > 0)
	{
		SetString(rText, wMarkedText);
	}
	else
	{
		if(!m_rKeyboardSequenceEvent.GetRecentText(rText, rhTextKL, fExcludeOlderText))
			return false;
	}
	return true;
}

bool KeyboardEventInfo::GetRecentText(_tstring &rText, HKL &rhTextKL, bool fExcludeOlderText) const
{
	return m_rKeyboardSequenceEvent.GetRecentText(rText, rhTextKL, fExcludeOlderText);
}

bool KeyboardEventInfo::QueryWorkingRect(RECT &rRect, int &rRectType) const
{
	return m_rFocusInfo.QueryWorkingRect(rRect, rRectType);
}
