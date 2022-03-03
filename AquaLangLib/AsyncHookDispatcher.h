#ifndef _ASYNC_HOOK_DISPATCHER_HEADER_
#define _ASYNC_HOOK_DISPATCHER_HEADER_

#include "ThreadInstance.h"
#include "WindowHookListenerAsyncWrapper.h"
#include "ObjectPool.h"
#include "UtilityWindow.h"
#include "WinHookResult.h"

class IThreadReleatedObject;

using namespace WinHook;

class AsyncHookDispatcher :
	public ThreadInstance,
	public IWindowMessageListener
{
public:
	AsyncHookDispatcher();
	virtual ~AsyncHookDispatcher();

	UtilityWindow &GetUtilityWindow()	{ return m_UtilityWindow; }
	
	void AddThreadReleatedObject(IThreadReleatedObject &rObject);

	bool Open();
	void Close();

	Result RegisterOnKeyboard(IKeyboardListener &rListener);
	Result UnregisterOnKeyboard(const IKeyboardListener &rListener);

	Result RegisterOnWindowMouseEvents(HWND hwnd, IMouseListener &rListener);
	Result RegisterOnMouseEvent(UINT uMsg, IMouseListener &rListener);
	Result UnregisterOnMouseEvents(const IMouseListener &rListener);
private:
	// ThreadInstance pure virtuals {
	virtual DWORD RunInThread();
	virtual void BreakThread();
	// }

	// IWindowMessageListener pure virtuals {
	virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// }
	KeyboardHookListenerAsyncWrapper *GetKeyboardWrapper(IKeyboardListener &rListener);
	MouseHookListenerAsyncWrapper *GetMouseWrapper(IMouseListener &rListener);
private:
	UtilityWindow m_UtilityWindow;

	std::vector<IThreadReleatedObject *> m_ThreadRelatedObjectList;
	ThreadEvent m_ObjectOpenComplete;

	typedef std::map<int/*IKeyboardListener*/, KeyboardHookListenerAsyncWrapper *> KeyboardListenerMap;
	KeyboardListenerMap m_KeyboardListenerMap;
	LockedObjectPool<KeyboardInfo> m_KeyboardInfoPool;

	typedef std::map<int/*IMouseListener*/, MouseHookListenerAsyncWrapper *> MouseListenerMap;
	MouseListenerMap m_MouseListenerMap;
	LockedObjectPool<MouseInfo> m_MouseInfoPool;
};

#endif // _ASYNC_HOOK_DISPATCHER_HEADER_