#include "Pch.h"
#include "HookDispatcher.h"
#include "WinHookListenerMap.h"
#include "MouseHookDispatcher.h"
#include "WinHookListener.h"
#include "WinHookResult.h"
#include "Log.h"

#define LOW_LEVEL_HOOK

// global variable definition
WinHook::MouseHookDispatcher g_MouseHookDispatcher;

namespace WinHook
{

MouseHookDispatcher::MouseHookDispatcher()
{
}

MouseHookDispatcher::~MouseHookDispatcher()
{
}

Result MouseHookDispatcher::RegisterOnWindowMouseEvents(HWND hwnd, IMouseListener &rListener)
{
#ifdef LOW_LEVEL_HOOK
	UNREFERENCED_PARAMETER(hwnd);
	UNREFERENCED_PARAMETER(rListener);
	Log(_T("MouseHookDispatcher::RegisterOnWindowMouseEvents - Low level hooking does not include window information\n"));
	return Result_InternalError;
#else
	if(hwnd == NULL)
	{
		Log(_T("MouseHookDispatcher::RegisterOnWindowMouseEvents - window is NULL\n"));
		return Result_InvalidParameter;
	}

	Result res = m_WindowMouseLisenerMap.Add(hwnd, rListener);
	if(res != Result_Success)
	{
		Log(_T("MouseHookDispatcher::RegisterOnWindowMouseEvents - failed adding to map (res=%d)\n"), res);
		return res;
	}

	return Result_Success;
#endif
}

Result MouseHookDispatcher::RegisterOnMouseEvent(UINT uMsg, IMouseListener &rListener)
{
	Result res = m_MouseMessageLisenerMap.Add(uMsg, rListener);
	if(res != Result_Success)
	{
		Log(_T("MouseHookDispatcher::RegisterOnMouseEvent - failed adding to map (res=%d)\n"), res);
		return res;
	}

	return Result_Success;
}

Result MouseHookDispatcher::Unregister(const IMouseListener &rListener)
{
	m_WindowMouseLisenerMap.Remove(rListener);
	m_MouseMessageLisenerMap.Remove(rListener);

	return Result_Success;
}

Result MouseHookDispatcher::OpenHook()
{
	if(m_WindowMouseLisenerMap.GetCount() == 0 && m_MouseMessageLisenerMap.GetCount() == 0)
		return Result_Success;

#ifdef LOW_LEVEL_HOOK
	return DoOpenHook(WH_MOUSE_LL, (HOOKPROC)MouseProc);
#else
	return DoOpenHook(WH_MOUSE, (HOOKPROC)MouseProc);
#endif
}

LRESULT MouseHookDispatcher::Notify(IMouseListener &rListener, int UNUSED(nCode), WPARAM wParam, LPARAM lParam)
{
	if(lParam == NULL)
		return TRUE;

	HWND hwnd = NULL;
#ifdef LOW_LEVEL_HOOK
	MSLLHOOKSTRUCT *pMouseStruct = (MSLLHOOKSTRUCT *)lParam;
#else
	MOUSEHOOKSTRUCT *pMouseStruct = (MOUSEHOOKSTRUCT *)lParam;
	hwnd = pMouseStruct->hwnd;
#endif
	return rListener.OnMouseHookEvent(hwnd, (UINT)wParam, pMouseStruct->pt.x, pMouseStruct->pt.y);
}

LRESULT CALLBACK MouseHookDispatcher::MouseProc(
	int nCode,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return g_MouseHookDispatcher.HandleEvent(nCode, wParam, lParam);
}

LRESULT MouseHookDispatcher::HandleEvent(
	int nCode,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if(nCode == HC_ACTION && wParam != WM_MOUSEMOVE) // WM_MOUSEMOVE is filtered out to avoid performance issues
	{
		ListenerList<IMouseListener> ListenerListCopy;

#ifndef LOW_LEVEL_HOOK
		MOUSEHOOKSTRUCT *pMouseStruct = (MOUSEHOOKSTRUCT *)lParam;
		if(pMouseStruct != NULL)
		{
			// list listeners that subscribed on the window
			m_WindowMouseLisenerMap.CopyList(pMouseStruct->hwnd, ListenerListCopy);
		}
#endif
		// list listeners that subscribed on the message
		m_MouseMessageLisenerMap.CopyList((UINT)wParam, ListenerListCopy);

		// dispatch the list
		Dispatch(ListenerListCopy, nCode, wParam, lParam);
	}
	return CompletedHandlingHook(nCode, wParam, lParam);
}

} // namespace WinHook
