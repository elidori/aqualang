#include "Pch.h"
#include "HookDispatcher.h"
#include "WinHookListenerMap.h"
#include "KeyboardHookDispatcher.h"
#include "WinHookListener.h"
#include "WinHookResult.h"

// global variable definition
WinHook::KeyboardHookDispatcher g_KeyboardHookDispatcher;

namespace WinHook
{

KeyboardHookDispatcher::KeyboardHookDispatcher()
{
}

KeyboardHookDispatcher::~KeyboardHookDispatcher()
{
}

Result KeyboardHookDispatcher::Register(IKeyboardListener &rListener)
{
	return m_KeyboardLisenerList.Add(rListener);
}

Result KeyboardHookDispatcher::Unregister(const IKeyboardListener &rListener)
{
	if(m_KeyboardLisenerList.GetCount() > 0)
	{
		m_KeyboardLisenerList.Remove(rListener);
	}

	return Result_Success;
}

Result KeyboardHookDispatcher::OpenHook()
{
	if(m_KeyboardLisenerList.GetCount() == 0)
		return Result_Success;

	return DoOpenHook(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc);
}

LRESULT KeyboardHookDispatcher::Notify(IKeyboardListener &rListener, int UNUSED(nCode), WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT *pKBStruct = (KBDLLHOOKSTRUCT *)lParam;
	if(!pKBStruct)
		return TRUE;
	return rListener.OnKeyboardHookEvent((UINT)wParam, pKBStruct->vkCode);
}

LRESULT CALLBACK KeyboardHookDispatcher::LowLevelKeyboardProc(
	int nCode,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return g_KeyboardHookDispatcher.HandleEvent(nCode, wParam, lParam);
}

LRESULT KeyboardHookDispatcher::HandleEvent(
	int nCode,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if(nCode == HC_ACTION)
	{
		ListenerList<IKeyboardListener> ListenerListCopy = m_KeyboardLisenerList;

		// dispatch the list
		Dispatch(ListenerListCopy, nCode, wParam, lParam);
	}
	return CompletedHandlingHook(nCode, wParam, lParam);
}

} // namespace WinHook
