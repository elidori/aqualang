#ifndef _WINDOW_HOOK_LISTENER_ASYNC_WRAPPER_HEADER_
#define _WINDOW_HOOK_LISTENER_ASYNC_WRAPPER_HEADER_

#include "ObjectPool.h"
#include "WinHookListener.h"

using namespace WinHook;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Keyboard wrapper
////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct KeyboardInfo
{
	UINT uMsg;
	int Code;
};


class KeyboardHookListenerAsyncWrapper : public IKeyboardListener
{
public:
	KeyboardHookListenerAsyncWrapper(
							HWND hDispatcher,
							UINT uKeyboardMessage,
							IKeyboardListener &rListener,
							ObjectPool<KeyboardInfo> &rInfoPool
							);
	virtual ~KeyboardHookListenerAsyncWrapper();

	void Notify(LPARAM lParam);
private:
	// IKeyboardListener pure virtuals {
	virtual LRESULT OnKeyboardHookEvent(UINT uMsg, int Code);
	// }
private:
	const HWND m_hDispatcher;
	const UINT m_uKeyboardMessage;

	IKeyboardListener &m_rListener;
	ObjectPool<KeyboardInfo> &m_rInfoPool;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mouse wrapper
////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct MouseInfo
{
	HWND hwnd;
	UINT uMsg;
	int x;
	int y;
};


class MouseHookListenerAsyncWrapper : public IMouseListener
{
public:
	MouseHookListenerAsyncWrapper(
							HWND hDispatcher,
							UINT uMouseMessage,
							IMouseListener &rListener,
							ObjectPool<MouseInfo> &rInfoPool
							);
	virtual ~MouseHookListenerAsyncWrapper();

	void Notify(LPARAM lParam);
private:
	// IKeyboardListener pure virtuals {
	virtual LRESULT OnMouseHookEvent(HWND hwnd, UINT uMsg, int x, int y);
	// }
private:
	const HWND m_hDispatcher;
	const UINT m_uMouseMessage;

	IMouseListener &m_rListener;
	ObjectPool<MouseInfo> &m_rInfoPool;
};

#endif // _WINDOW_HOOK_LISTENER_ASYNC_WRAPPER_HEADER_