#include "Pch.h"
#include "CalculatorHandler.h"
#include "KeyboardSequenceEvent.h"
#include "UserInformationNotifier.h"
#include "infixString.h"
#include "KeyboardEventInfo.h"
#include "StringConvert.h"
#include "FocusInfo.h"
#include "StringSizeLimits.h"
#include "Log.h"

#pragma warning(disable:4355)

static const bool fExcludeOlderText = true;

CalculatorHandler::CalculatorHandler(TipCoordinator &rTipCoordinator, UtilityWindow &rUtilityWindow, FocusInfo &rFocusInfo, KeyboardSequenceEvent &rKeyboardSequenceEvent, UserInformationNotifier &rUserInformationNotifier)
	: PeriodicEvaluationHandler(TIMER_ID_CALCULATOR_EVAL, rUtilityWindow, rFocusInfo, rKeyboardSequenceEvent),
	m_KeyboardTrigger(),
	m_rUserInformationNotifier(rUserInformationNotifier),
	m_CompactCalculatorTip(*this, rTipCoordinator),
	m_CalculatorTip(*this, MAX_CALC_EVAL_STRING_SIZE),
	m_fUseDegrees(true)
{
}

CalculatorHandler::~CalculatorHandler()
{
	Close();
}

bool CalculatorHandler::Configure(LPCTSTR UserName, int ApearanceDurationIn100msec, LPCTSTR KeyboardTrigger, int MinKbPauseBeforeTipPopupIn100msec)
{
	DoConfigure(UserName, ApearanceDurationIn100msec, MinKbPauseBeforeTipPopupIn100msec, 1, 0);

	m_KeyboardTrigger = KeyboardTrigger;

	if(!m_ExpressionFilter.Open())
		return false;

	return true;
}

bool CalculatorHandler::RegisterOnHooks()
{
	Result res = m_rKeyboardSequenceEvent.Register(m_KeyboardTrigger.c_str(), *this, false);
	if(res != Result_Success)
		return false;
	return true;
}

bool CalculatorHandler::OpenTip(LPCTSTR UserName, int ApearanceDurationIn100msec)
{
	if(!m_CompactCalculatorTip.Open(UserName, ApearanceDurationIn100msec))
		return false;
	if(!m_CalculatorTip.Open(UserName))
		return false;
	return true;
}

void CalculatorHandler::ShowTip(LPCTSTR ProposedText, const RECT &rCaretRect, HWND UNUSED(hCaretWindow))
{
	m_CalculatorTip.SetCaretRect(rCaretRect);
	m_CompactCalculatorTip.SetCaretRect(rCaretRect);
	m_CompactCalculatorTip.Init(ProposedText);
}

bool CalculatorHandler::CanEvaluate()
{
	if(m_CalculatorTip.IsVisible())
		return false;
	return true;
}

bool CalculatorHandler::EvalString(const _tstring &rInString, HKL UNUSED(hTextKL), _tstring &ProposedText)
{
	int FilterCharacters = 0;
	bool fSuccess = DoEvalString(rInString.c_str(), m_ResultString, m_PresentableString, FilterCharacters, true);
	ProposedText = m_ResultString;
	return fSuccess;
}

void CalculatorHandler::NotifyUserTriggersCalculator()
{
	m_rUtilityWindow.UserMessage(WPARAM_CALC_POPUP, 0);
}

void CalculatorHandler::Close()
{
	m_CompactCalculatorTip.Close();
	m_CalculatorTip.Close();
	m_rKeyboardSequenceEvent.Unregister(*this);

	DoClose();
}

void CalculatorHandler::TipShouldClose(int Operation)
{
	switch(Operation)
	{
	case CALC_OP_EXPAND:
		m_CompactCalculatorTip.Term();
		m_rUtilityWindow.UserMessage(WPARAM_CALC_OP_EXPAND, 0);
		break;
	case CALC_OP_REDUCE:
		m_CalculatorTip.Term();
		m_rUtilityWindow.UserMessage(WPARAM_CALC_OP_REDUCE, 0);
		break;
	case CALC_OP_CLOSE:
		m_CompactCalculatorTip.Term();
		m_CalculatorTip.Term();
		break;
	}
}

void CalculatorHandler::TipShouldReevaluate(LPCTSTR Expression)
{
	bool fSuccess = false;
	int FilterCharacters = 0;
	if(DoEvalString(Expression, m_ResultString, m_PresentableString, FilterCharacters, false))
	{
		if(FilterCharacters == 0)
		{
			fSuccess = true;
			m_CalculatorTip.Reevaluate(m_ResultString.c_str());
		}
	}
	if(!fSuccess)
	{
		m_CalculatorTip.Reevaluate(_T("0"));
	}
}

void CalculatorHandler::AngleUnitModeChanged(bool fUseDegrees)
{
	m_fUseDegrees = fUseDegrees;
}

// collect needed information before deciding
bool CalculatorHandler::CanOperate(KeyboardEventInfo &Info, bool &rfExclusive)
{
	rfExclusive = true;

	if(m_CalculatorTip.IsOwnerOf(Info.GetCurrentWindow()))
		return false;

	Info.PeekAndLoadMarkedText();

	_tstring EvaluatedText;
	HKL hTextKL = NULL;
	if(Info.GetMarkedOrRecentText(EvaluatedText, hTextKL, fExcludeOlderText))
	{
		if(EvaluatedText.length() >= MAX_CALC_EVAL_STRING_SIZE)
			return false;

		int FilterCharacters = 0;
		if(DoEvalString(EvaluatedText.c_str(), m_ResultString, m_PresentableString, FilterCharacters, true))
		{
			if(FilterCharacters == 0)
				return true;
		}
	}

	return false;
}

void CalculatorHandler::Operate(const KeyboardEventInfo & UNUSED(Info), IKeyboardEventCallback &rCallback)
{
	bool fComplete = false;

	RECT CaretRect;
	int RectType;
	if(!m_rFocusInfo.QueryWorkingRect(CaretRect, RectType))
		return;
	m_CalculatorTip.SetCaretRect(CaretRect);
	m_CompactCalculatorTip.SetCaretRect(CaretRect);

	if(m_CalculatorTip.IsVisible())
	{
		m_CompactCalculatorTip.Term();
		m_CompactCalculatorTip.StoreResult(m_ResultString.c_str());

		if(m_CalculatorTip.Init(m_PresentableString.c_str(), m_ResultString.c_str(), false))
		{
			fComplete = true;
			m_rUserInformationNotifier.IncreaseCalcRequestEvents();
		}
	}
	else
	{
		m_CalculatorTip.Term();
		if(m_CompactCalculatorTip.Init(m_ResultString.c_str()))
		{
			fComplete = true;
			m_rUserInformationNotifier.IncreaseCalcRequestEvents();
		}
	}
	rCallback.EventOperationEnded(fComplete);
}

void CalculatorHandler::Message(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_UTILITY_USER_MESSAGE:
		switch(wParam)
		{
		case WPARAM_CALC_OP_EXPAND:
			m_CalculatorTip.Init(m_PresentableString.c_str(), m_ResultString.c_str(), false);
			break;
		case WPARAM_CALC_OP_REDUCE:
			m_CompactCalculatorTip.Init(NULL);
			break;
		case WPARAM_CALC_POPUP:
			UserTriggersCalculator();
			break;
		}
		break;
	}

	PeriodicEvaluationHandler::Message(uMsg, wParam, lParam);
}

bool CalculatorHandler::DoEvalString(LPCTSTR InString, _tstring &Result, _tstring &rPresentableString, int &rFilteredCharacters, bool fFilterPatterns)
{
	double dResult = evalInfixString(InString, m_fUseDegrees, &rFilteredCharacters);
	if(dResult == 0)
		return false;
	TCHAR ResultStr[256];
	_stprintf_s(ResultStr, sizeof(ResultStr) / sizeof(ResultStr[0]), _T("%.12lG"), dResult); // lG presents a double value either in ddd.ddd or in E format. Whichever is more compact
	Result = ResultStr;

	// build a presentable string, based on rFilteredCharacters, and after cleaning out initial spaces
	assert(rFilteredCharacters < (int)_tcslen(InString));
	if(rFilteredCharacters >= (int)_tcslen(InString))
		return false;

	int InitialCharPosition = rFilteredCharacters;
	while(InString[InitialCharPosition] == _T(' '))
	{
		InitialCharPosition++;
	}
	rPresentableString = &InString[InitialCharPosition];
	Log(_T("CalculatorHandler::DoEvalString - presentable string is \'%s\'\n"), rPresentableString.c_str());

	if(fFilterPatterns)
	{
		LPCTSTR FilterName = m_ExpressionFilter.FindFilter(rPresentableString.c_str());
		if(FilterName != NULL)
		{
			Log(_T("CalculatorHandler::DoEvalString - Evaluated string matched \'%s\' filter\n"), FilterName);
			return false;
		}
	}
	return true;
}

bool CalculatorHandler::UserTriggersCalculator()
{
	if(!m_CalculatorTip.IsVisible())
	{
		m_CompactCalculatorTip.Term();
		m_PresentableString = _T("");
		m_ResultString = _T("0");

		bool fFocusChanged;
		if(!m_rFocusInfo.UpdateCurrentInfo(false, fFocusChanged))
			return false;

		RECT CaretRect;
		int RectType;
		if(!m_rFocusInfo.QueryWorkingRect(CaretRect, RectType))
			return false;
		m_CalculatorTip.SetCaretRect(CaretRect);

		if(!m_CalculatorTip.Init(m_PresentableString.c_str(), m_ResultString.c_str(), true))
			return false;
	}
	m_CalculatorTip.SetFocus();
	return true;
}
