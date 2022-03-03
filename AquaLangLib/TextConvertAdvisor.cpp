#include "Pch.h"
#include "TextConvertAdvisor.h"
#include "KeyboardSequenceEvent.h"
#include "AsyncHookDispatcher.h"
#include "KeyboardEventInfo.h"
#include "StringConvert.h"
#include "ClipboardAccess.h"
#include "ILanguageModifier.h"
#include "Log.h"

#pragma warning (disable:4355)

TextConvertAdvisor::TextConvertAdvisor(
				TipCoordinator &rTipCoordinator,
				KeyboardSequenceEvent &rKeyboardSequenceEvent,
				AsyncHookDispatcher &rAsyncHookDispatcher,
				ILanguageModifier &rLanguageModifier
				)
	: m_rKeyboardSequenceEvent(rKeyboardSequenceEvent),
	m_rAsyncHookDispatcher(rAsyncHookDispatcher),
	m_rLanguageModifier(rLanguageModifier),
	m_hCurrentWindow(NULL),
	m_hCurrentLanguage(NULL),
	m_fWaitingForEndOfBackspaces(false),
	m_nBackspacesInWait(0),
	m_TextAdvisorTip(*this, rTipCoordinator)
{
}

TextConvertAdvisor::~TextConvertAdvisor()
{
	Close();
}

void TextConvertAdvisor::ResetLanguageHistory()
{
	m_hCurrentLanguage = NULL;
}

bool TextConvertAdvisor::Open(LPCTSTR UserName)
{
	Result res = m_rKeyboardSequenceEvent.Register(_T("PRINTABLE"), *this, true);
	if(res != Result_Success)
		return false;
	res = m_rKeyboardSequenceEvent.RegisterOnFocusChange(*this);
	if(res != Result_Success)
		return false;
	
	// register all keyboard events
	res = m_rAsyncHookDispatcher.RegisterOnKeyboard(*this);
	if(res != Result_Success)
		return false;

	if(!m_TextAdvisorTip.Open(UserName))
		return false;

	return true;
}

void TextConvertAdvisor::Close()
{
	m_TextAdvisorTip.Close();
	m_rAsyncHookDispatcher.UnregisterOnKeyboard(*this);
	m_rKeyboardSequenceEvent.UnregisterOnFocusChange(*this);
	m_rKeyboardSequenceEvent.Unregister(*this);
	m_hCurrentWindow = NULL;
	m_hCurrentLanguage = NULL;
	m_fWaitingForEndOfBackspaces = false;
	m_nBackspacesInWait = 0;
}

bool TextConvertAdvisor::CanOperate(KeyboardEventInfo &Info, bool &fExclusive)
{
	fExclusive = false;

	_tstring Text;
	HKL hCurrentKL = NULL;
	Info.GetRecentText(Text, hCurrentKL, false);

	if(!m_TextAdvisorTip.IsActive())
	{
		if(m_fWaitingForEndOfBackspaces)
		{
			int nBackspaces;
			if(m_rKeyboardSequenceEvent.CheckIfAllKeysAreBackspace(nBackspaces))
			{
				m_nBackspacesInWait = nBackspaces;
			}
			else
			{
				ActivateIfTextMatches(Text, hCurrentKL, m_nBackspacesInWait);
			}
		}
		else if(hCurrentKL != m_hCurrentLanguage && m_hCurrentLanguage != NULL)
		{
			if(m_hCurrentWindow == Info.GetCurrentWindow() && m_hCurrentWindow != NULL)
			{
				if(Text.length() == 0)
				{
					// this means that the user changed language and now starts deleting
					int nBackspace;
					if(m_rKeyboardSequenceEvent.CheckIfAllKeysAreBackspace(nBackspace))
					{
						assert(nBackspace == 1);
						m_fWaitingForEndOfBackspaces = true;
						m_nBackspacesInWait = nBackspace;
					}
				}
				else
				{
					// this means that the user changed language after typing\erasing
					ActivateIfTextMatches(Text, hCurrentKL, 0);
				}
			}
		}
	}
	else
	{
		// cancel active mode if focus changed, language changed, or not match was found to the recently typed text
		if(hCurrentKL != m_hCurrentLanguage || m_hCurrentWindow != Info.GetCurrentWindow() || !DoTextsMatch(m_SuggestedText.c_str(), m_SuggestedText.length(), Text.c_str(), Text.length()))
		{
			m_TextAdvisorTip.Term();
		}
	}

	m_hCurrentWindow = Info.GetCurrentWindow();
	m_hCurrentLanguage = hCurrentKL;

	return m_TextAdvisorTip.IsActive();
}

void TextConvertAdvisor::Operate(const KeyboardEventInfo &Info, IKeyboardEventCallback &rCallback)
{
	if(m_TextAdvisorTip.IsActive())
	{
		_tstring Text;
		HKL hCurrentKL = NULL;
		Info.GetRecentText(Text, hCurrentKL, false);

		RECT CaretRect;
		int RectType;
		if(Info.QueryWorkingRect(CaretRect, RectType))
		{
			if(m_TextAdvisorTip.Init(CaretRect, Text.c_str(), m_SuggestedText.substr(Text.length()).c_str()))
			{
				m_fWaitingForEndOfBackspaces = false;
				m_nBackspacesInWait = 0;
			}
			else
			{
				m_TextAdvisorTip.ShouldActive();
			}
		}
	}

	rCallback.EventOperationEnded(false);
}

void TextConvertAdvisor::ActivateIfTextMatches(const _tstring &rText, HKL hCurrentKL, int nBackspacesAfterLanguageChanged)
{
	assert(rText.length() == 1);
	if(rText.length() == 1)
	{
		if(m_rKeyboardSequenceEvent.GetPreviouslyErasedText(m_SuggestedText, hCurrentKL, nBackspacesAfterLanguageChanged))
		{
			if(DoTextsMatch(m_SuggestedText.c_str(), m_SuggestedText.length(), rText.c_str(), rText.length()))
			{
				m_TextAdvisorTip.ShouldActive();
			}
		}
	}
	else
	{
		Log(_T("TextConvertAdvisor::ActivateIfTextMatches - Text length is not null!!! Text=%s\n"), rText.c_str());
	}

}

LRESULT TextConvertAdvisor::OnKeyboardHookEvent(UINT uMsg, int Code)
{
	if(Code >= 0xFF)
		return TRUE;

	if(uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
	{
		switch(Code)
		{
		case VK_CONTROL:
		case VK_LCONTROL:
		case VK_RCONTROL:
		case VK_MENU:
		case VK_LMENU:
		case VK_RMENU:
		case VK_F1:
		case VK_F2:
		case VK_F3:
		case VK_F4:
		case VK_F5:
		case VK_F6:
		case VK_F7:
		case VK_F8:
		case VK_F9:
		case VK_F10:
		case VK_F11:
		case VK_F12:
		case VK_F13:
		case VK_F14:
		case VK_F15:
		case VK_F16:
		case VK_F17:
		case VK_F18:
		case VK_F19:
		case VK_F20:
		case VK_F21:
		case VK_F22:
		case VK_F23:
		case VK_F24:
		case VK_TAB:
		case VK_DELETE:
		case VK_HOME:
		case VK_END:
		case VK_PRIOR:
		case VK_NEXT:
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
		case VK_CANCEL:
		case VK_RETURN:
		case VK_ESCAPE:
			if(m_TextAdvisorTip.IsActive())
			{
				m_TextAdvisorTip.Term();
				m_fWaitingForEndOfBackspaces = false;
				m_nBackspacesInWait = 0;
			}
			break;
		}
	}
	return TRUE;
}

void TextConvertAdvisor::FocusChanged(HWND hFocusWindow)
{
	if(m_TextAdvisorTip.HasWindow(hFocusWindow) || hFocusWindow == NULL)
		return;
	m_TextAdvisorTip.Term();
	m_fWaitingForEndOfBackspaces = false;
	m_nBackspacesInWait = 0;
}

void TextConvertAdvisor::NotifyOnTextChangeTrigger(const KeyboardEventInfo & UNUSED(Info), bool &fUsed)
{
	fUsed = false;
	if(m_TextAdvisorTip.IsActive())
	{
		fUsed = true;
		TipShouldClose(COMPACT_TIP_OP_APPLY);
		m_rLanguageModifier.ClearTextHistory();
	}
}

void TextConvertAdvisor::TipShouldClose(int Operation)
{
	_tstring TextToPaste = m_TextAdvisorTip.GetRemainingText();

	m_TextAdvisorTip.Term();

	if(Operation == COMPACT_TIP_OP_APPLY)
	{
		std::wstring wText;
		SetString(wText, TextToPaste.c_str());
		ClipboardAccess::PasteText(m_hCurrentWindow, wText.c_str());
	}
}

bool TextConvertAdvisor::DoTextsMatch(LPCTSTR SuggestedText, size_t SuggestedTextLength, LPCTSTR TypedText, size_t TypedTextLength) const
{
	if(TypedTextLength > SuggestedTextLength)
		return false;
	return (_tcsnicmp(SuggestedText, TypedText, TypedTextLength) == 0);
}
