#ifndef _TEXT_CONVERT_ADVISOR_HEADER_
#define _TEXT_CONVERT_ADVISOR_HEADER_

#include "IKeyboardEvent.h"
#include "WinHookListener.h"
#include "TextAdvisorTip.h"
#include "IFocusChangeListener.h"
#include "ITextChangeClient.h"
#include "ICompactTipResult.h"

class KeyboardSequenceEvent;
class AsyncHookDispatcher;
class ILanguageModifier;
class FocusInfo;

using namespace WinHook;

class TextConvertAdvisor :
	public IKeyboardEvent,
	public IKeyboardListener,
	public IFocusChangeListener,
	public ITextChangeClient,
	public ICompactTipResult
{
public:
	TextConvertAdvisor(
				TipCoordinator &rTipCoordinator,
				KeyboardSequenceEvent &rKeyboardSequenceEvent,
				AsyncHookDispatcher &rAsyncHookDispatcher,
				ILanguageModifier &rLanguageModifier
				);
	virtual ~TextConvertAdvisor();

	void ResetLanguageHistory();

	bool Open(LPCTSTR UserName);
	void Close();
private:
	// IKeyboardEvent pure virtuals {
	virtual bool CanOperate(KeyboardEventInfo &Info, bool &fExclusive);
	virtual void Operate(const KeyboardEventInfo &Info, IKeyboardEventCallback &rCallback);
	// }

	void ActivateIfTextMatches(const _tstring &rText, HKL hCurrentKL, int nBackspacesAfterLanguageChanged);

	// IKeyboardListener pure virtuals {
	virtual LRESULT OnKeyboardHookEvent(UINT uMsg, int Code);
	// }

	// IFocusChangeListener pure virtuals {
	virtual void FocusChanged(HWND hFocusWindow);
	// }

	// ITextChangeClient pure virtuals {
	virtual void NotifyOnTextChangeTrigger(const KeyboardEventInfo &Info, bool &fUsed);
	// }

	// ICompactTipResult pure virtuals {
	virtual void TipShouldClose(int Operation);	
	// }

	bool DoTextsMatch(LPCTSTR SuggestedText, size_t SuggestedTextLength, LPCTSTR TypedText, size_t TypedTextLength) const;
private:
	KeyboardSequenceEvent &m_rKeyboardSequenceEvent;
	AsyncHookDispatcher &m_rAsyncHookDispatcher;
	ILanguageModifier &m_rLanguageModifier;

	HWND m_hCurrentWindow;
	HKL m_hCurrentLanguage;

	bool m_fActive;
	bool m_fWaitingForEndOfBackspaces;

	_tstring m_SuggestedText;
	int m_nBackspacesInWait;

	TextAdvisorTip m_TextAdvisorTip;
};

#endif // _TEXT_CONVERT_ADVISOR_HEADER_