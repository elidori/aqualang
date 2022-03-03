#include "Pch.h"
#include "AsyncHookDispatcher.h"
#include "WinHookListenerList.h"
#include "WinHookListenerMap.h"
#include "HookDispatcher.h"
#include "KeyboardHookDispatcher.h"
#include "MouseHookDispatcher.h"
#include "UtilityWindowParams.h"
#include "IThreadReleatedObject.h"
#include "Log.h"

enum
{
	MAX_PENDING_KEYBOARD_EVENTS = 1000,
	MAX_PENDING_MOUSE_EVENTS = 1000
};

#define OBJECT_OPEN_TIMEOUT 5000

AsyncHookDispatcher::AsyncHookDispatcher()
	: m_KeyboardInfoPool(MAX_PENDING_KEYBOARD_EVENTS),
	m_MouseInfoPool(MAX_PENDING_MOUSE_EVENTS)
{
}

AsyncHookDispatcher::~AsyncHookDispatcher()
{
	Close();
}

void AsyncHookDispatcher::AddThreadReleatedObject(IThreadReleatedObject &rObject)
{
	m_ThreadRelatedObjectList.push_back(&rObject);
}

bool AsyncHookDispatcher::Open()
{
	if(!m_KeyboardInfoPool.Open())
		return false;
	if(!m_MouseInfoPool.Open())
		return false;
	if(!m_UtilityWindow.Register(*this))
		return false;

	if(!OpenThread())
		return false;
	if(!m_ObjectOpenComplete.Wait(OBJECT_OPEN_TIMEOUT))
	{
		Log(_T("AsyncHookDispatcher::Open() - timeout waiting for thread open to complete"));
		return false;
	}
	return true;
}

void AsyncHookDispatcher::Close()
{
	CloseThread();
	m_UtilityWindow.Unregister(*this);
	m_UtilityWindow.Close(); // closing thread does that, but in any case ...
	m_MouseInfoPool.Close();
	m_KeyboardInfoPool.Close();
}

Result AsyncHookDispatcher::RegisterOnKeyboard(IKeyboardListener &rListener)
{
	KeyboardHookListenerAsyncWrapper *pWrapper = GetKeyboardWrapper(rListener);
	if(pWrapper == NULL)
		return Result_MemAllocFailed;

	Result res = g_KeyboardHookDispatcher.Register(*pWrapper);
	if(res != Result_Success)
		return res;

	return Result_Success;
}

Result AsyncHookDispatcher::UnregisterOnKeyboard(const IKeyboardListener &rListener)
{
	Result res = Result_Success;
	KeyboardListenerMap::iterator Iterator = m_KeyboardListenerMap.find((int)&rListener);
	if(Iterator != m_KeyboardListenerMap.end())
	{
		KeyboardHookListenerAsyncWrapper *pWrapper = (*Iterator).second;
		res = g_KeyboardHookDispatcher.Unregister(*pWrapper);
		m_KeyboardListenerMap.erase(Iterator);
		delete pWrapper;
	}
	return res;
}

Result AsyncHookDispatcher::RegisterOnWindowMouseEvents(HWND hwnd, IMouseListener &rListener)
{
	MouseHookListenerAsyncWrapper *pWrapper = GetMouseWrapper(rListener);
	if(pWrapper == NULL)
		return Result_MemAllocFailed;

	Result res = g_MouseHookDispatcher.RegisterOnWindowMouseEvents(hwnd, *pWrapper);
	if(res != Result_Success)
		return res;

	return Result_Success;
}

Result AsyncHookDispatcher::RegisterOnMouseEvent(UINT uMsg, IMouseListener &rListener)
{
	MouseHookListenerAsyncWrapper *pWrapper = GetMouseWrapper(rListener);
	if(pWrapper == NULL)
		return Result_MemAllocFailed;

	Result res = g_MouseHookDispatcher.RegisterOnMouseEvent(uMsg, *pWrapper);
	if(res != Result_Success)
		return res;

	return Result_Success;
}

Result AsyncHookDispatcher::UnregisterOnMouseEvents(const IMouseListener &rListener)
{
	Result res = Result_Success;
	MouseListenerMap::iterator Iterator = m_MouseListenerMap.find((int)&rListener);
	if(Iterator != m_MouseListenerMap.end())
	{
		MouseHookListenerAsyncWrapper *pWrapper = (*Iterator).second;
		res = g_MouseHookDispatcher.Unregister(*pWrapper);
		m_MouseListenerMap.erase(Iterator);
		delete pWrapper;
	}
	return res;
}

DWORD AsyncHookDispatcher::RunInThread()
{
	if(!m_UtilityWindow.Open(_T("AsyncHookDispatcher message window")))
	{
		Log(_T("AsyncHookDispatcher::RunInThread - failed opening utility window\n"));
		return (DWORD)-1;
	}

	for(unsigned int i = 0; i < m_ThreadRelatedObjectList.size(); i++)
	{
		m_ThreadRelatedObjectList.at(i)->OpenInThreadContext();
	}
	m_ObjectOpenComplete.Set();

	MSG msg;
	while(!IsBreaking())
	{
		if(::GetMessage(&msg, NULL, NULL, NULL))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	m_UtilityWindow.Close();
	return 0;
}

void AsyncHookDispatcher::BreakThread()
{
	m_UtilityWindow.PostMessage(WM_CLOSEMESSAGEWINDOW, 0, 0);
}

void AsyncHookDispatcher::Message(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_KEYBOARDASYNCEVENT:
		if(wParam != 0)
		{
			KeyboardHookListenerAsyncWrapper *pKeyboardWrapper = (KeyboardHookListenerAsyncWrapper *)wParam;
			pKeyboardWrapper->Notify(lParam);
		}
		break;
	case WM_MOUSEASYNCEVENT:
		if(wParam != 0)
		{
			MouseHookListenerAsyncWrapper *pMouseWrapper = (MouseHookListenerAsyncWrapper *)wParam;
			pMouseWrapper->Notify(lParam);
		}
		break;
	case WM_CLOSEMESSAGEWINDOW:
		break;
	}
}

KeyboardHookListenerAsyncWrapper *AsyncHookDispatcher::GetKeyboardWrapper(IKeyboardListener &rListener)
{
	KeyboardListenerMap::iterator Iterator = m_KeyboardListenerMap.find((int)&rListener);
	KeyboardHookListenerAsyncWrapper *pWrapper = NULL;
	if(Iterator == m_KeyboardListenerMap.end())
	{
		pWrapper = new KeyboardHookListenerAsyncWrapper(m_UtilityWindow.GetHandle(), WM_KEYBOARDASYNCEVENT, rListener, m_KeyboardInfoPool);
		if(pWrapper == NULL)
			return NULL;
		m_KeyboardListenerMap.insert(KeyboardListenerMap::value_type((int)&rListener, pWrapper));
	}
	else
	{
		pWrapper = (*Iterator).second;
	}
	return pWrapper;
}

MouseHookListenerAsyncWrapper *AsyncHookDispatcher::GetMouseWrapper(IMouseListener &rListener)
{
	MouseListenerMap::iterator Iterator = m_MouseListenerMap.find((int)&rListener);
	MouseHookListenerAsyncWrapper *pWrapper = NULL;
	if(Iterator == m_MouseListenerMap.end())
	{
		pWrapper = new MouseHookListenerAsyncWrapper(m_UtilityWindow.GetHandle(), WM_MOUSEASYNCEVENT, rListener, m_MouseInfoPool);
		if(pWrapper == NULL)
			return NULL;
		m_MouseListenerMap.insert(MouseListenerMap::value_type((int)&rListener, pWrapper));
	}
	else
	{
		pWrapper = (*Iterator).second;
	}
	return pWrapper;
}