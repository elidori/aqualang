#include "Pch.h"
#include "TextLanguageConverter.h"
#include "WinHookResult.h"
#include "KeyboardMapper.h"
#include "KeyboardSequenceEvent.h"
#include "UserInformationNotifier.h"
#include "KeyboardEventInfo.h"
#include "IKeyboardEventCallback.h"
#include "UtilityWindowParams.h"
#include "LanguageDetector.h"
#include "ClipboardAccess.h"
#include "ILanguageModifier.h"
#include "ITextChangeClient.h"
#include "GuessLanguageHandler.h"
#include "WindowItem.h"
#include "Log.h"

#define LANGUAGE_CHANGE_VERIFY_RETRIES 10
#define LANGUAGE_CHANGE_VERIFY_INTERVAL 10

static const bool fExcludeOlderText = true;

TextLanguageConverter::TextLanguageConverter(
							UtilityWindow &rUtilityWindow,
							KeyboardSequenceEvent &rKeyboardSequenceEvent,
							UserInformationNotifier &rUserInformationNotifier,
							const LanguageDetector &rLanguageDetector,
							ILanguageModifier &rLanguageModifier,
							ITextChangeClient &rTextChangeClient,
							GuessLanguageHandler &rGuessLanguageHandler
							)
	: m_rUtilityWindow(rUtilityWindow),
	m_rKeyboardSequenceEvent(rKeyboardSequenceEvent),
	m_rUserInformationNotifier(rUserInformationNotifier),
	m_rLanguageDetector(rLanguageDetector),
	m_rLanguageModifier(rLanguageModifier),
	m_rTextChangeClient(rTextChangeClient),
	m_rGuessLanguageHandler(rGuessLanguageHandler),
	m_pCallback(NULL)
{
	m_rGuessLanguageHandler.SetTextConverter(*this);
}

TextLanguageConverter::~TextLanguageConverter()
{
	Close();
}

bool TextLanguageConverter::Open(LPCTSTR KeyboardTrigger)
{
	Result res = m_rKeyboardSequenceEvent.Register(KeyboardTrigger, *this, false);
	if(res != Result_Success)
		return false;
	if(!m_rUtilityWindow.Register(*this))
		return false;
	return true;
}

void TextLanguageConverter::Close()
{
	m_rKeyboardSequenceEvent.Unregister(*this);
	m_rUtilityWindow.Unregister(*this);
	m_pCallback = NULL;
}

bool TextLanguageConverter::CanOperate(KeyboardEventInfo &Info, bool &fExclusive)
{
	fExclusive = false;
	Info.PeekAndLoadMarkedText();
	return true;
}

void TextLanguageConverter::Operate(const KeyboardEventInfo &Info, IKeyboardEventCallback &rCallback)
{
	HKL Dummy;
	LPCWSTR MarkedText = Info.PeekMarkedText(Dummy);

	bool fUsedByAnotherClient = false;
	m_rTextChangeClient.NotifyOnTextChangeTrigger(Info, fUsedByAnotherClient);

	// check if clipboard is enabled
	bool fClipboardIsEnabled = true;
	const WindowItem *pWindowItemRules = NULL;
	Info.GetWindowItem(&pWindowItemRules);
	if(pWindowItemRules != NULL)
	{
		const xmlBool *pProp = (const xmlBool *)pWindowItemRules->GetPropertyValue(RuleTypeClipboardEnabled);
		if(pProp != NULL)
		{
			fClipboardIsEnabled = pProp->GetBool();
		}
	}

	if(fUsedByAnotherClient)
	{
		rCallback.EventOperationEnded(true);
	}
	else if(fClipboardIsEnabled && MarkedText != NULL && wcslen(MarkedText) > 0)
	{
		std::wstring ToggledText;
		m_rLanguageDetector.ToggleStringLanguage(MarkedText, ToggledText);
		if(ToggledText.length() > 0)
		{
			ClipboardAccess::PasteText(Info.GetCurrentWindow(), ToggledText.c_str());
		}
		rCallback.EventOperationEnded(true);
	}
	else if(!m_rKeyboardSequenceEvent.IsEmpty(true))
	{
		HKL hCurrentKL = ::GetKeyboardLayout(::GetWindowThreadProcessId(Info.GetCurrentWindow(), NULL));

		// get target language
		HKL hToggledKL = NULL;
		if(m_rGuessLanguageHandler.IsGuessing(hToggledKL))
		{
			m_rGuessLanguageHandler.GuessAccepted();
		}
		else
		{
			KeyboardMapper CharacterMapper;
			hToggledKL = CharacterMapper.ToggleLayout(hCurrentKL);
		}

		if(!ChangeTypedText(hCurrentKL, hToggledKL, Info.GetCurrentWindow(), &rCallback))
		{
			rCallback.EventOperationEnded(false);
		}
	}
	else
	{
		rCallback.EventOperationEnded(false);
	}
}

bool TextLanguageConverter::ChangeTypedText(HKL hCurrentKL, HKL hTargetKL, HWND hCurrentWindow, IKeyboardEventCallback *pCallback)
{
	// clear old characters
	int BackspacesToIgnore = m_rKeyboardSequenceEvent.BackspaceSequence(hCurrentKL, fExcludeOlderText);
	// change language
	m_rLanguageModifier.ChangeLanguage(hCurrentWindow, hTargetKL); // note: it sends a keboard key notification of a2 (Left Control)

	m_nRetries = LANGUAGE_CHANGE_VERIFY_RETRIES;
	m_hCurrentWindow = hCurrentWindow;
	m_hTargetKL = hTargetKL;
	m_BackspacesToIgnore = BackspacesToIgnore;
	m_pCallback = pCallback;
	
	return m_rUtilityWindow.StartTimer(LANGUAGE_CHANGE_VERIFY_INTERVAL, TIMER_ID_TEXT_CHANGE_VERIFY);
}

void TextLanguageConverter::Message(UINT uMsg, WPARAM wParam, LPARAM UNUSED(lParam))
{
	switch(uMsg)
	{
	case WM_TIMER:
		if(wParam == TIMER_ID_TEXT_CHANGE_VERIFY)
		{
			m_nRetries--;

			HKL hCurrentKL = ::GetKeyboardLayout(GetWindowThreadProcessId(m_hCurrentWindow, NULL));
			if(hCurrentKL == m_hTargetKL || m_nRetries == 0)
			{
				m_rUtilityWindow.StopTimer(TIMER_ID_TEXT_CHANGE_VERIFY);

				m_rKeyboardSequenceEvent.ReplayLatestKeys(m_BackspacesToIgnore, m_hTargetKL, fExcludeOlderText);
				m_rKeyboardSequenceEvent.UpdateRecentTextLanguage(m_hTargetKL, fExcludeOlderText);

				m_rUserInformationNotifier.IncreaseTextLanguageChangeEvents();

				if(m_pCallback != NULL)
				{
					m_pCallback->EventOperationEnded(true);
					m_pCallback = NULL;
				}
			}
		}
	}
}
