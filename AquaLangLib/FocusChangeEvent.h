#ifndef _FOCUS_CHANGE_EVENT_HEADER_
#define _FOCUS_CHANGE_EVENT_HEADER_

#include "IUIEventListener.h"
#include "WinHookResult.h"
#include "WinHookListener.h"
#include "WinHookListenerList.h"
#include "FocusInfo.h"
#include "AsyncEventHandler.h"
#include "UtilityWindow.h"

using namespace WinHook;

class FocusChangeEvent :
	public IKeyboardListener,
	public IMouseListener,
	public IWindowMessageListener,
	public IAsyncEventHandlerTarget
{
public:
	FocusChangeEvent();
	virtual ~FocusChangeEvent();

	UtilityWindow &GetUtilityWindow();

	Result Register(IUIEventListener &rListener);
	Result Unregister(const IUIEventListener &rListener);
private:
	Result OpenHandles();
	void CloseHandles();

	// IKeyboardListener pure virtuals {
	virtual LRESULT OnKeyboardHookEvent(UINT uMsg, int Code);
	// }

	// IMouseListener pure virtuals {
	virtual LRESULT OnMouseHookEvent(HWND hwnd, UINT uMsg, int x, int y);
	// }

	// IWindowMessageListener pure virtuals {
	virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// }

	// IAsyncEventHandlerTarget pure virtuals {
	virtual void HandleAsynchronous();
	virtual void HandleCritical();
	// }

	void CheckFocus(bool fSynchronous);
	void Dispatch(EventType Type, HWND hwnd, LPCTSTR SubObjectId, const RECT *pRect, UIActionType Action);
private:
	ListenerList<IUIEventListener> m_FocusChangeLisenerList;

	FocusInfo m_FocusInfo;

	AsyncEventHandler m_AsyncEventHandler;
};

#endif // _FOCUS_CHANGE_EVENT_HEADER_