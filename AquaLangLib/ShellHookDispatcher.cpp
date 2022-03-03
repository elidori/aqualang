#include "Pch.h"
#include "HookDispatcher.h"
#include "WinHookListenerMap.h"
#include "ShellHookDispatcher.h"
#include "WinHookListener.h"
#include "WinHookResult.h"

// global variable definition
WinHook::ShellHookDispatcher g_ShellHookDispatcher;

namespace WinHook
{

ShellHookDispatcher::ShellHookDispatcher()
{
}

ShellHookDispatcher::~ShellHookDispatcher()
{
}

Result ShellHookDispatcher::RegisterOnShellEvent(int Code, IShellListener &rListener)
{
	return m_ShellLisenerMap.Add(Code, rListener);
}

Result ShellHookDispatcher::Unregister(const IShellListener &rListener)
{
	m_ShellLisenerMap.Remove(rListener);

	return Result_Success;
}

Result ShellHookDispatcher::OpenHook()
{
	if(m_ShellLisenerMap.GetCount() == 0)
		return Result_Success;

	return DoOpenHook(WH_SHELL, (HOOKPROC)ShellProc);
}

LRESULT ShellHookDispatcher::Notify(IShellListener &rListener, int nCode, WPARAM wParam, LPARAM lParam)
{
	return rListener.OnShellHookEvent(nCode, wParam, lParam);
}

LRESULT CALLBACK ShellHookDispatcher::ShellProc(
	int nCode,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return g_ShellHookDispatcher.HandleEvent(nCode, wParam, lParam);
}

LRESULT ShellHookDispatcher::HandleEvent(
	int nCode,
	WPARAM wParam,
	LPARAM lParam
	)
{
	ListenerList<IShellListener> ListenerListCopy;

	// list listeners that subscribed on the code
	m_ShellLisenerMap.CopyList(nCode, ListenerListCopy);

	// dispatch the list
	Dispatch(ListenerListCopy, nCode, wParam, lParam);

	return CompletedHandlingHook(nCode, wParam, lParam);
}

} // namespace WinHook