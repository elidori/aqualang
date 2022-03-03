#ifndef _GUESS_LANGUAGE_TIP_HEADER_
#define _GUESS_LANGUAGE_TIP_HEADER_

#include "CompactTip.h"
#include "WinHookListener.h"

class TipCoordinator;
class AsyncHookDispatcher;

using namespace WinHook;

class GuessLanguageTip : 
	public CompactTip,
	public IMouseListener
{
public:
	GuessLanguageTip(
				ICompactTipResult &rTipResult, 
				TipCoordinator &rTipCoordinator,
				AsyncHookDispatcher &rAsyncHookDispatcher
				);
	virtual ~GuessLanguageTip();

	bool Open(LPCTSTR UserName, int ApearanceDurationIn100msec);

	bool Init(LPCTSTR TextString, const RECT &rCaretRect);

	void Term();

	void Close();
private:
	// IMouseListener pure virtual {
	virtual LRESULT OnMouseHookEvent(HWND hwnd, UINT uMsg, int x, int y);
	// }

	bool RegisterOnMouseClicks();
	void UnregisterOnMouseClicks();
private:
	AsyncHookDispatcher &m_rAsyncHookDispatcher;
};

#endif // _GUESS_LANGUAGE_TIP_HEADER_