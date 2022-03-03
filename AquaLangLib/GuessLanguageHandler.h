#ifndef _GUESS_LANGUAGE_HANDLER_HEADER_
#define _GUESS_LANGUAGE_HANDLER_HEADER_

#include "PeriodicEvaluationHandler.h"
#include "IKeyboardEvent.h"
#include "GuessLanguageTip.h"
#include "KeyboardSequence.h"
#include "ICompactTipResult.h"
#include "IKeyboardEventCallback.h"

class KeyboardSequenceEvent;
class UserInformationNotifier;
class FocusInfo;
class Corpora;
class LanguageStringTable;
class ITextConverter;
class TipCoordinator;
class INotificationAreaRequestsListener;
class AsyncHookDispatcher;

class GuessLanguageHandler :
	public PeriodicEvaluationHandler,
	public ICompactTipResult,
	public IKeyboardEventCallback
{
public:
	GuessLanguageHandler(
				TipCoordinator &rTipCoordinator,
				UtilityWindow &rUtilityWindow, 
				FocusInfo &rFocusInfo, 
				KeyboardSequenceEvent &rKeyboardSequenceEvent, 
				UserInformationNotifier &rUserInformationNotifier,
				AsyncHookDispatcher &rAsyncHookDispatcher
				);
	virtual ~GuessLanguageHandler();

	void SetNotificationAreaHandler(INotificationAreaRequestsListener &rListener)	{ m_pNotificationAreaListener = &rListener; }

	bool Configure(
				LPCTSTR UserName, 
				int ApearanceDurationIn100msec, 
				int MinKbPauseBeforeTipPopupIn100msec,
				int MinimalNumberOfCharacters,
				int MaximalNumberOfCharacters,
				const LanguageStringTable &rLanguageStringTable
				);
	void Close();

	void SetTextConverter(ITextConverter &rConverter)	{ m_pTextConverter = &rConverter; }

	bool IsGuessing(HKL &hTargetKL) const;
	void GuessAccepted();

	bool GuessLanguage(LPCWSTR wText, HKL &rhKL);
private:
	// PeriodicEvaluationHandler pure virtuals {
	virtual bool OpenTip(LPCTSTR UserName, int ApearanceDurationIn100msec);
	virtual void ShowTip(LPCTSTR ProposedText, const RECT &rCaretRect, HWND hCaretWindow);
	virtual bool CanEvaluate();
	virtual bool EvalString(const _tstring &rInString, HKL hTextKL, _tstring &ProposedText);
	// }

	// ICompactTipResult pure virtuals {
	virtual void TipShouldClose(int Operation);
	// }

	// IKeyboardEventCallback pure virtuals {
	virtual void EventOperationEnded(bool fComplete);
	// }
	void TermTip();
private:
	GuessLanguageTip m_GuessLanguageTip;

	UserInformationNotifier &m_rUserInformationNotifier;

	INotificationAreaRequestsListener *m_pNotificationAreaListener;
	bool m_fCurrentlyNotifyingGuess;
	int m_NotificationAreaEvents;

	std::vector<Corpora *> m_CorporaList;

	HKL m_hTypedTextKL;
	HKL m_hGuessedLanguage;
	HWND m_hCaretWindow;

	ITextConverter *m_pTextConverter;
};

#endif // _GUESS_LANGUAGE_HANDLER_HEADER_