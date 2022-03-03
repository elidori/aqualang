#include "Pch.h"
#include "FocusChangeEvent.h"
#include "WinHookListenerMap.h"
#include "HookDispatcher.h"
#include "MouseHookDispatcher.h"
#include "KeyboardHookDispatcher.h"
#include "UtilityWindowParams.h"
#include "Log.h"

#pragma warning (disable:4355)

FocusChangeEvent::FocusChangeEvent()
	: m_AsyncEventHandler(*this)
{
}

FocusChangeEvent::~FocusChangeEvent()
{
	CloseHandles();
}

UtilityWindow &FocusChangeEvent::GetUtilityWindow()
{
	return m_AsyncEventHandler.GetUtilityWindow();
}

Result FocusChangeEvent::Register(IUIEventListener &rListener)
{
	Result res = m_FocusChangeLisenerList.Add(rListener);
	if(res != Result_Success)
		return res;

	if(m_FocusChangeLisenerList.GetCount() == 1)
	{
		res = OpenHandles();
		if(res != Result_Success)
			return res;
	}
	return Result_Success;
}

Result FocusChangeEvent::Unregister(const IUIEventListener &rListener)
{
	Result res = m_FocusChangeLisenerList.Remove(rListener);
	if(res != Result_Success)
		return res;

	if(m_FocusChangeLisenerList.GetCount() == 0)
	{
		CloseHandles();
	}
	return Result_Success;
}

Result FocusChangeEvent::OpenHandles()
{
	if(!m_AsyncEventHandler.OpenEventHandler())
		return Result_ResourceAllocFailed;
	m_AsyncEventHandler.GetUtilityWindow().Register(*this);

	Result res = g_MouseHookDispatcher.RegisterOnMouseEvent(WM_LBUTTONUP, *this);
	if(res != Result_Success)
		return res;
	res = g_MouseHookDispatcher.RegisterOnMouseEvent(WM_LBUTTONDOWN, *this);
	if(res != Result_Success)
		return res;
	res = g_KeyboardHookDispatcher.Register(*this);
	if(res != Result_Success)
		return res;

//	m_FocusInfo.EnableHTMLChecking();

	return Result_Success;
}

void FocusChangeEvent::CloseHandles()
{
	g_MouseHookDispatcher.Unregister(*this);
	g_KeyboardHookDispatcher.Unregister(*this);
	m_AsyncEventHandler.GetUtilityWindow().Unregister(*this);
	m_AsyncEventHandler.CloseEventHandler();
}

LRESULT FocusChangeEvent::OnKeyboardHookEvent(UINT uMsg, int UNUSED(Code))
{
	if(uMsg == WM_KEYUP || uMsg == WM_KEYDOWN)
	{
		m_AsyncEventHandler.EventRequestCritical();
		m_AsyncEventHandler.EventRequestAsynchronous(400);
	}
	return TRUE;
}

LRESULT FocusChangeEvent::OnMouseHookEvent(HWND UNUSED(hwnd), UINT uMsg, int UNUSED(x), int UNUSED(y))
{
	if(uMsg == WM_LBUTTONUP || uMsg == WM_LBUTTONDOWN)
	{
		m_AsyncEventHandler.EventRequestCritical();
		m_AsyncEventHandler.EventRequestAsynchronous(400);
	}
	return TRUE;
}

void FocusChangeEvent::Message(UINT uMsg, WPARAM wParam, LPARAM UNUSED(lParam))
{
	if(uMsg == WM_NOTIFY_NO_FOCUS_CHANGE)
	{
		Dispatch(
			Type_StoreLanguage,
			(HWND)wParam,
			NULL,
			NULL,
			ActionType_Unknown
			);
	}
}

void FocusChangeEvent::HandleAsynchronous()
{
	CheckFocus(false);
}

void FocusChangeEvent::HandleCritical()
{
	CheckFocus(true);
}

void FocusChangeEvent::CheckFocus(bool fSynchronous)
{
	bool fFocusChanged = false;
	if(!m_FocusInfo.UpdateCurrentInfo(fSynchronous, fFocusChanged))
		return;

	if(m_FocusInfo.QueryCurrentFocusWindow() == NULL)
		return;

	if(fFocusChanged)
	{
		RECT CaretRect;
		int RectType;
		bool fRectExists = m_FocusInfo.QueryWorkingRect(CaretRect, RectType);
		UIActionType Action = ((RectType == WORKING_RECT_CARET) ? ActionType_EditWindow : ActionType_NonEditWindow);

		Dispatch(
			fFocusChanged ? Type_Focus : Type_StoreLanguage,
			m_FocusInfo.QueryCurrentFocusWindow(),
			m_FocusInfo.GetSubObjectId(),
			fRectExists ? &CaretRect : NULL,
			Action
			);
	}
	else
	{
		m_AsyncEventHandler.GetUtilityWindow().PostMessage(WM_NOTIFY_NO_FOCUS_CHANGE, (WPARAM)m_FocusInfo.QueryCurrentFocusWindow(), 0);
	}
}

void FocusChangeEvent::Dispatch(EventType Type, HWND hwnd, LPCTSTR SubObjectId, const RECT *pRect, UIActionType Action)
{
	IUIEventListener *pListener = NULL;
	Result res = m_FocusChangeLisenerList.First(pListener);

	while(res == Result_Success && pListener != NULL)
	{
		pListener->UIEvent(Type, hwnd, SubObjectId, pRect, Action);
		res = m_FocusChangeLisenerList.Next(pListener);
	}
}
