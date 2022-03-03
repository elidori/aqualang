#ifndef _PERIODIC_EVALUATION_HANDLER_HEADER_
#define _PERIODIC_EVALUATION_HANDLER_HEADER_

#include "UtilityWindow.h"
#include "IThreadReleatedObject.h"

class KeyboardSequenceEvent;
class FocusInfo;

class PeriodicEvaluationHandler :
	public IWindowMessageListener,
	public IThreadReleatedObject
{
public:
	PeriodicEvaluationHandler(
						int nTimerId,
						UtilityWindow &rUtilityWindow, 
						FocusInfo &rFocusInfo, 
						KeyboardSequenceEvent &rKeyboardSequenceEvent
						);
	virtual ~PeriodicEvaluationHandler();

	bool IsOpen() const;
protected:
	void DoConfigure(
				LPCTSTR UserName, 
				int ApearanceDurationIn100msec,
				int MinKbPauseBeforeTipPopupIn100msec,
				int MinimalNumberOfCharacters,
				int MaximalNumberOfCharacters
				);
	void DoClose();

	// IThreadReleatedObject pure virtuals {
	virtual bool OpenInThreadContext();
	// }

	// IWindowMessageListener pure virtuals {
	virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// }

	int GetMinimalNumberOfCharacters() const	{ return m_MinimalNumberOfCharacters; }
	int GetMaximalNumberOfCharacters() const	{ return m_MaximalNumberOfCharacters; }
	bool EvalWhileClick();

	virtual bool OpenTip(LPCTSTR UserName, int ApearanceDurationIn100msec) = 0;
	virtual void ShowTip(LPCTSTR ProposedText, const RECT &rCaretRect, HWND hCaretWindow) = 0;
	virtual bool CanEvaluate() = 0;
	virtual bool EvalString(const _tstring &rInString, HKL hTextKL, _tstring &ProposedText) = 0;
private:
	const int m_nTimerId;

	bool m_fIsOpen;

	int m_MinExpressionTestIntervalMsec;
	int m_MaxExpressionTestIntervalMsec;
	int m_AvgExpressionTestIntervalMsec;
	int m_MinimalNumberOfCharacters;
	int m_MaximalNumberOfCharacters;

	int m_ApearanceDurationIn100msec;
	_tstring m_UserName;
protected:
	UtilityWindow &m_rUtilityWindow;
	FocusInfo &m_rFocusInfo;
	KeyboardSequenceEvent &m_rKeyboardSequenceEvent;
};

#endif // _PERIODIC_EVALUATION_HANDLER_HEADER_