#include "Pch.h"
#include "GuessLanguageHandler.h"
#include "KeyboardSequenceEvent.h"
#include "UserInformationNotifier.h"
#include "KeyboardEventInfo.h"
#include "StringConvert.h"
#include "FocusInfo.h"
#include "StringSizeLimits.h"
#include "Corpora.h"
#include "LanguageStringTable.h"
#include "KeyboardMapper.h"
#include "StringConvert.h"
#include "LanguageDetector.h"
#include "ITextConverter.h"
#include "INotificationAreaRequestsListener.h"
#include "Log.h"

#define PROBABILITY_DIFF_RATIO_THESHOLD 0
#define MAX_GUESS_LANGUAGE_NOTIFICATION_EVENTS 3

#pragma warning(disable:4355)

static const bool fExcludeOlderText = true;

GuessLanguageHandler::GuessLanguageHandler(
										TipCoordinator &rTipCoordinator,
										UtilityWindow &rUtilityWindow, 
										FocusInfo &rFocusInfo, 
										KeyboardSequenceEvent &rKeyboardSequenceEvent, 
										UserInformationNotifier &rUserInformationNotifier,
										AsyncHookDispatcher &rAsyncHookDispatcher
										)
	: PeriodicEvaluationHandler(TIMER_ID_GUESS_LANGUAGE, rUtilityWindow, rFocusInfo, rKeyboardSequenceEvent),
	m_rUserInformationNotifier(rUserInformationNotifier),
	m_pNotificationAreaListener(NULL),
	m_fCurrentlyNotifyingGuess(false),
	m_NotificationAreaEvents(0),
	m_GuessLanguageTip(*this, rTipCoordinator, rAsyncHookDispatcher),
	m_hTypedTextKL(NULL),
	m_hGuessedLanguage(NULL),
	m_hCaretWindow(NULL),
	m_pTextConverter(NULL)
{
}

GuessLanguageHandler::~GuessLanguageHandler()
{
	Close();
}

bool GuessLanguageHandler::Configure(
								LPCTSTR UserName, 
								int ApearanceDurationIn100msec, 
								int MinKbPauseBeforeTipPopupIn100msec,
								int MinimalNumberOfCharacters,
								int MaximalNumberOfCharacters,
								const LanguageStringTable &rLanguageStringTable
								)
{
	DoConfigure(UserName, ApearanceDurationIn100msec, MinKbPauseBeforeTipPopupIn100msec, MinimalNumberOfCharacters, MaximalNumberOfCharacters);

	KeyboardMapper LanguageMapper;
	for(int i = 0; i < LanguageMapper.GetLayoutCount(); i++)
	{
		HKL hKL = LanguageMapper.GetLayout(i);
		_tstring Dummy;
		_tstring LanguageString;
		if(rLanguageStringTable.GetLanguageString(hKL, Dummy, LanguageString, LANGUAGE_FORMAT_SHORT_NAME))
		{
			Corpora *pNew = Corpora::Create(LanguageString.c_str(), hKL);
			if(pNew == NULL)
				return false;
			m_CorporaList.push_back(pNew);
		}
	}
	return true;
}

bool GuessLanguageHandler::IsGuessing(HKL &hTargetKL) const
{
	if(m_hGuessedLanguage == NULL)
		return false;
	hTargetKL = m_hGuessedLanguage;
	return true;
}

void GuessLanguageHandler::GuessAccepted()
{
	m_hTypedTextKL = NULL;
	m_hGuessedLanguage = NULL;
	TermTip();

	m_rUserInformationNotifier.IncreaseGuessLanguageAcceptEvents();
}

bool GuessLanguageHandler::GuessLanguage(LPCWSTR wText, HKL &rhKL)
{
	rhKL = NULL;

	int PreferredIndex = -1;
	float PrefferedProb = 0;
	for(unsigned int i = 0; i < m_CorporaList.size(); i++)
	{
		std::string asciiString;
		SetString(asciiString, wText, (size_t)-1, m_CorporaList.at(i)->GetCodePage());
	
		float Probability = m_CorporaList.at(i)->probOfStr(asciiString.c_str());
		if(PrefferedProb < Probability)
		{
			PrefferedProb = Probability;
			PreferredIndex = i;
		}
	}
	if(PreferredIndex == -1)
		return false;

	rhKL = m_CorporaList.at(PreferredIndex)->GetKL();
	return true;
}

bool GuessLanguageHandler::OpenTip(LPCTSTR UserName, int ApearanceDurationIn100msec)
{
	if(!m_GuessLanguageTip.Open(UserName, ApearanceDurationIn100msec))
		return false;
	m_rKeyboardSequenceEvent.IgnoreMouseEvents(m_GuessLanguageTip, true);
	return true;
}

void GuessLanguageHandler::ShowTip(LPCTSTR ProposedText, const RECT &rCaretRect, HWND hCaretWindow)
{
	if(m_pNotificationAreaListener != NULL)
	{
		m_NotificationAreaEvents++;
		if(m_NotificationAreaEvents <= MAX_GUESS_LANGUAGE_NOTIFICATION_EVENTS)
		{
			if(m_pNotificationAreaListener->PresentTextRequest(
											_T("AquaLang"), 
											_T("Press <Shift><Shift> to switch language")
											))
			{
				m_fCurrentlyNotifyingGuess = true;
			}
		}
	}
	m_hCaretWindow = hCaretWindow;
	m_GuessLanguageTip.Init(ProposedText, rCaretRect);
}

bool GuessLanguageHandler::CanEvaluate()
{
	return true;
}

bool GuessLanguageHandler::EvalString(const _tstring &rInString, HKL hTextKL, _tstring &ProposedText)
{
	m_hTypedTextKL = NULL;
	m_hGuessedLanguage = NULL;

	if((int)rInString.length() < GetMinimalNumberOfCharacters() || (GetMaximalNumberOfCharacters() != 0 && (int)rInString.length() > GetMaximalNumberOfCharacters()))
		return false;

	int PreferredIndex = -1;
	float PrefferedProb = 0;
	float CurrentLanguageProb = 0;
	for(unsigned int i = 0; i < m_CorporaList.size(); i++)
	{
		std::string asciiString;
		LanguageDetector::ConvertText(rInString.c_str(), hTextKL, m_CorporaList.at(i)->GetKL(), asciiString, m_CorporaList.at(i)->GetCodePage());
	
		float Probability = m_CorporaList.at(i)->probOfStr(asciiString.c_str());
		if(PrefferedProb < Probability)
		{
			PrefferedProb = Probability;
			PreferredIndex = i;
		}

		if(m_CorporaList.at(i)->GetKL() == hTextKL)
		{
			CurrentLanguageProb = Probability;
		}
	}

	if(PreferredIndex == -1 || m_CorporaList.at(PreferredIndex)->GetKL() == hTextKL)
	{
		TermTip();
		return false;
	}

	// check probability diff. If it is below threshold, don't guess
	assert(PrefferedProb >= CurrentLanguageProb);
	if(CurrentLanguageProb > 0.0 && (PrefferedProb - CurrentLanguageProb) / CurrentLanguageProb < PROBABILITY_DIFF_RATIO_THESHOLD)
	{
		TermTip();
		return false;
	}

	m_hTypedTextKL = hTextKL;
	m_hGuessedLanguage = m_CorporaList.at(PreferredIndex)->GetKL();

	LanguageDetector::ConvertText(rInString.c_str(), hTextKL, m_hGuessedLanguage, ProposedText, CP_UTF8);
	ProposedText += _T("[");
	ProposedText += m_CorporaList.at(PreferredIndex)->GetLanguage();
	ProposedText += _T("]");
	
	m_rUserInformationNotifier.IncreaseGuessLanguageAdviseEvents();

	return true;
}

void GuessLanguageHandler::Close()
{
	m_hTypedTextKL = NULL;
	m_hGuessedLanguage = NULL;
	m_hCaretWindow = NULL;

	m_rKeyboardSequenceEvent.IgnoreMouseEvents(m_GuessLanguageTip, false);
	m_GuessLanguageTip.Close();
	for(unsigned int i = 0; i < m_CorporaList.size(); i++)
	{
		delete m_CorporaList.at(i);
	}
	m_CorporaList.erase(m_CorporaList.begin(), m_CorporaList.end());

	DoClose();
}

void GuessLanguageHandler::TipShouldClose(int Operation)
{
	TermTip();

	HKL hTargetKL = m_hGuessedLanguage;
	HKL hTypedKL = m_hTypedTextKL;
	HWND hCaretWindow = m_hCaretWindow;

	m_hGuessedLanguage = NULL;
	m_hTypedTextKL = NULL;
	m_hCaretWindow = NULL;

	if(Operation == COMPACT_TIP_OP_APPLY)
	{
		if(m_pTextConverter != NULL)
		{
			m_rKeyboardSequenceEvent.BlockHooks(true);

			if(m_pTextConverter->ChangeTypedText(hTypedKL, hTargetKL, hCaretWindow, this))
			{
				m_rUserInformationNotifier.IncreaseGuessLanguageAcceptEvents();
			}
			else
			{
				EventOperationEnded(false);
			}
		}
	}
}

void GuessLanguageHandler::EventOperationEnded(bool UNUSED(fComplete))
{
	m_rKeyboardSequenceEvent.BlockHooks(false);
}

void GuessLanguageHandler::TermTip()
{
	if(m_fCurrentlyNotifyingGuess)
	{
		m_fCurrentlyNotifyingGuess = false;

		if(m_pNotificationAreaListener != NULL)
		{
			m_pNotificationAreaListener->PresentTextRequest(_T(""), _T(""));
		}
	}

	m_GuessLanguageTip.Term();
}