#include "Pch.h"
#include "MessageHookDispatcher.h"
#include "WinHookListener.h"
#include "WinHookResult.h"

// global variable definition
WinHook::MessageHookDispatcher g_MessageHookDispatcher;

namespace WinHook
{
MessageHookDispatcher::MessageHookDispatcher()
{
}

MessageHookDispatcher::~MessageHookDispatcher()
{
}

Result MessageHookDispatcher::RegisterOnWindow(HWND hwnd, IWindowListener &rListener)
{
	if(hwnd == NULL)
		return Result_InvalidParameter;

	return m_WindowLisenerMap.Add(hwnd, rListener);
}

Result MessageHookDispatcher::RegisterOnMessage(UINT uMsg, IWindowListener &rListener)
{
	return m_MessageLisenerMap.Add(uMsg, rListener);
}

Result MessageHookDispatcher::Unregister(const IWindowListener &rListener)
{
	m_WindowLisenerMap.Remove(rListener);
	m_MessageLisenerMap.Remove(rListener);

	return Result_Success;
}

Result MessageHookDispatcher::OpenHook()
{
	if(m_WindowLisenerMap.GetCount() == 0 && m_MessageLisenerMap.GetCount() == 0)
		return Result_Success;

	return DoOpenHook(WH_CALLWNDPROC, (HOOKPROC)CallWndProc);
}

LRESULT MessageHookDispatcher::Notify(IWindowListener &rListener, int UNUSED(nCode), WPARAM UNUSED(wParam), LPARAM lParam)
{
	MSG *pMsg = (MSG *)lParam;
	if(!pMsg)
		return TRUE;
	return rListener.OnMessageHookEvent(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
}

LRESULT CALLBACK MessageHookDispatcher::CallWndProc(
	int nCode,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return g_MessageHookDispatcher.HandleMessage(nCode, wParam, lParam);
}

LRESULT MessageHookDispatcher::HandleMessage(
	int nCode,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if(nCode == HC_ACTION && wParam == PM_REMOVE)
	{
		MSG *pMsg = (MSG *)lParam;

		if(pMsg)
		{
			ListenerList<IWindowListener> ListenerListCopy;
			// list listeners that subscribed on the window
			m_WindowLisenerMap.CopyList(pMsg->hwnd, ListenerListCopy);
			// list listeners that subscribed on the message
			m_MessageLisenerMap.CopyList(pMsg->message, ListenerListCopy);

			// dispatch the list
			Dispatch(ListenerListCopy, nCode, wParam, lParam);
		}
	}
	return CompletedHandlingHook(nCode, wParam, lParam);
}

} // namespace WinHook
