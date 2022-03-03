#include "Pch.h"
#include "PeriodicEvaluationHandler.h"
#include "KeyboardSequenceEvent.h"
#include "FocusInfo.h"

static const bool fExcludeOlderText = true;

PeriodicEvaluationHandler::PeriodicEvaluationHandler(
										int nTimerId,
										UtilityWindow &rUtilityWindow, 
										FocusInfo &rFocusInfo, 
										KeyboardSequenceEvent &rKeyboardSequenceEvent
										)
	: m_nTimerId(nTimerId),
	m_fIsOpen(false),
	m_MinExpressionTestIntervalMsec(0),
	m_MaxExpressionTestIntervalMsec(0),
	m_AvgExpressionTestIntervalMsec(0),
	m_MinimalNumberOfCharacters(0),
	m_MaximalNumberOfCharacters(0),
	m_ApearanceDurationIn100msec(0),
	m_UserName(),
	m_rUtilityWindow(rUtilityWindow),
	m_rFocusInfo(rFocusInfo),
	m_rKeyboardSequenceEvent(rKeyboardSequenceEvent)
{
}

PeriodicEvaluationHandler::~PeriodicEvaluationHandler()
{
	DoClose();
}

void PeriodicEvaluationHandler::DoConfigure(
								LPCTSTR UserName, 
								int ApearanceDurationIn100msec, 
								int MinKbPauseBeforeTipPopupIn100msec,
								int MinimalNumberOfCharacters,
								int MaximalNumberOfCharacters
								)
{
	m_MinExpressionTestIntervalMsec = MinKbPauseBeforeTipPopupIn100msec * 100 / 2;
	m_MaxExpressionTestIntervalMsec = m_MinExpressionTestIntervalMsec + MinKbPauseBeforeTipPopupIn100msec * 100;
	m_AvgExpressionTestIntervalMsec = MinKbPauseBeforeTipPopupIn100msec *100;
	m_MinimalNumberOfCharacters = MinimalNumberOfCharacters;
	m_MaximalNumberOfCharacters = MaximalNumberOfCharacters;

	m_ApearanceDurationIn100msec = ApearanceDurationIn100msec;
	m_UserName = UserName;
}

bool PeriodicEvaluationHandler::OpenInThreadContext()
{
	if(!m_rUtilityWindow.Register(*this))
		return false;
	if(!m_rUtilityWindow.StartTimer(m_AvgExpressionTestIntervalMsec, m_nTimerId))
		return false;
	if(!OpenTip(m_UserName.c_str(), m_ApearanceDurationIn100msec))
		return false;
	m_fIsOpen = true;
	return true;
}

bool PeriodicEvaluationHandler::IsOpen() const
{
	return m_fIsOpen;
}

void PeriodicEvaluationHandler::DoClose()
{
	m_fIsOpen = false;
	m_rUtilityWindow.StopTimer(m_nTimerId);
	m_rUtilityWindow.Unregister(*this);
	m_MinExpressionTestIntervalMsec = 0;
	m_MaxExpressionTestIntervalMsec = 0;
	m_AvgExpressionTestIntervalMsec = 0;
	m_MinimalNumberOfCharacters = 0;
	m_MaximalNumberOfCharacters = 0;
	m_ApearanceDurationIn100msec = 0;
}

void PeriodicEvaluationHandler::Message(UINT uMsg, WPARAM wParam, LPARAM UNUSED(lParam))
{
	switch(uMsg)
	{
	case WM_TIMER:
		if(wParam == (WPARAM)m_nTimerId)
		{
			EvalWhileClick();
		}
		break;
	}
}

bool PeriodicEvaluationHandler::EvalWhileClick()
{
	if(m_rKeyboardSequenceEvent.AreHooksBlocked())
		return false;

	if(!CanEvaluate())
		return false;
	
	bool fEvaluate = false;
	_tstring Text;
	HKL hTextKL = NULL;

	__int64 TimePassedSinceEvent;
	if(m_rKeyboardSequenceEvent.IsInPrintMode(TimePassedSinceEvent))
	{
		// check if the latest printable character was pressed recently, but not too recently ...
		if(TimePassedSinceEvent < m_MaxExpressionTestIntervalMsec && TimePassedSinceEvent > m_MinExpressionTestIntervalMsec)
		{
			if(m_rKeyboardSequenceEvent.GetRecentText(Text, hTextKL, fExcludeOlderText))
			{
				fEvaluate = true;
			}
		}
	}

	if(fEvaluate)
	{
		_tstring ProposedText;
		if(EvalString(Text, hTextKL, ProposedText))
		{
			bool fFocusChanged;
			m_rFocusInfo.UpdateCurrentInfo(false, fFocusChanged);
			RECT CaretRect;
			int RectType;
			if(!m_rFocusInfo.QueryWorkingRect(CaretRect, RectType))
				return false;

			ShowTip(ProposedText.c_str(), CaretRect, m_rFocusInfo.QueryCurrentCaretWindow());
		}
	}
	return fEvaluate;
}
