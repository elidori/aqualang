#ifndef _CALCULATOR_HANDLER_HEADER_
#define _CALCULATOR_HANDLER_HEADER_

#include "PeriodicEvaluationHandler.h"
#include "ICalcTipResult.h"
#include "IKeyboardEvent.h"
#include "CalculatorTip.h"
#include "CompactCalculatorTip.h"
#include "KeyboardSequence.h"
#include "expressionFilter.h"

class KeyboardSequenceEvent;
class UserInformationNotifier;
class FocusInfo;
class TipCoordinator;

class CalculatorHandler :
	public PeriodicEvaluationHandler,
	public IKeyboardEvent,
	public ICalcTipResult
{
public:
	CalculatorHandler(TipCoordinator &rTipCoordinator, UtilityWindow &rUtilityWindow, FocusInfo &rFocusInfo, KeyboardSequenceEvent &rKeyboardSequenceEvent, UserInformationNotifier &rUserInformationNotifier);
	virtual ~CalculatorHandler();

	bool Configure(LPCTSTR UserName, int ApearanceDurationIn100msec, LPCTSTR KeyboardTrigger, int MinKbPauseBeforeTipPopupIn100msec);

	bool RegisterOnHooks();
	void NotifyUserTriggersCalculator();

	void Close();
private:
	// PeriodicEvaluationHandler pure virtuals {
	virtual bool OpenTip(LPCTSTR UserName, int ApearanceDurationIn100msec);
	virtual void ShowTip(LPCTSTR ProposedText, const RECT &rCaretRect, HWND hCaretWindow);
	virtual bool CanEvaluate();
	virtual bool EvalString(const _tstring &rInString, HKL hTextKL, _tstring &ProposedText);
	// }

	// IKeyboardEvent pure virtuals {
	virtual bool CanOperate(KeyboardEventInfo &Info, bool &rfExclusive);
	virtual void Operate(const KeyboardEventInfo &Info, IKeyboardEventCallback &rCallback);
	// }

	// IWindowMessageListener pure virtuals {
	virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// }

	// ICalcTipResult pure virtuals {
	virtual void TipShouldClose(int Operation);
	virtual void TipShouldReevaluate(LPCTSTR Expression);
	virtual void AngleUnitModeChanged(bool fUseDegrees);
	// }

	bool DoEvalString(LPCTSTR InString, _tstring &Result, _tstring &rPresentableString, int &rFilteredCharacters, bool fFilterPatterns);

	bool UserTriggersCalculator();
private:
	_tstring m_KeyboardTrigger;
	UserInformationNotifier &m_rUserInformationNotifier;
	
	CompactCalculatorTip m_CompactCalculatorTip;
	CalculatorTip m_CalculatorTip;

	ExpressionFilter m_ExpressionFilter; // filter out phone numbers, dates and other patterns

	bool m_fUseDegrees;

	// intermidiate results
	_tstring m_ResultString;
	_tstring m_PresentableString;
};

#endif // _CALCULATOR_HANDLER_HEADER_