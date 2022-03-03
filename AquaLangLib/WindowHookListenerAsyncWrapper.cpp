#include "Pch.h"
#include "WindowHookListenerAsyncWrapper.h"
#include "Log.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// KeyboardHookListenerAsyncWrapper
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
KeyboardHookListenerAsyncWrapper::KeyboardHookListenerAsyncWrapper(
							HWND hDispatcher,
							UINT uKeyboardMessage,
							IKeyboardListener &rListener,
							ObjectPool<KeyboardInfo> &rInfoPool
							)
	: m_hDispatcher(hDispatcher),
	m_uKeyboardMessage(uKeyboardMessage),
	m_rListener(rListener),
	m_rInfoPool(rInfoPool)
{
}

KeyboardHookListenerAsyncWrapper::~KeyboardHookListenerAsyncWrapper()
{
}

void KeyboardHookListenerAsyncWrapper::Notify(LPARAM lParam)
{
	if(lParam == 0)
		return;
	ObjectWrapper<KeyboardInfo> *pInfoWrapper = (ObjectWrapper<KeyboardInfo> *)lParam;
	KeyboardInfo LocalInfo = pInfoWrapper->Info();
	pInfoWrapper->Release();
	
	m_rListener.OnKeyboardHookEvent(LocalInfo.uMsg, LocalInfo.Code);
}

LRESULT KeyboardHookListenerAsyncWrapper::OnKeyboardHookEvent(UINT uMsg, int Code)
{
	ObjectWrapper<KeyboardInfo> *pInfoWrapper = m_rInfoPool.Get();
	if(pInfoWrapper == NULL)
	{
		Log(_T("KeyboardHookListenerAsyncWrapper::Notify - Error: out of KeyboardInfo items!\n"));
		return TRUE;
	}
	KeyboardInfo &rInfo = pInfoWrapper->Info();
	rInfo.uMsg = uMsg;
	rInfo.Code = Code;

	::PostMessage(m_hDispatcher, m_uKeyboardMessage, (WPARAM)this, (LPARAM)pInfoWrapper);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MouseHookListenerAsyncWrapper
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MouseHookListenerAsyncWrapper::MouseHookListenerAsyncWrapper(
							HWND hDispatcher,
							UINT uMouseMessage,
							IMouseListener &rListener,
							ObjectPool<MouseInfo> &rInfoPool
							)
	: m_hDispatcher(hDispatcher),
	m_uMouseMessage(uMouseMessage),
	m_rListener(rListener),
	m_rInfoPool(rInfoPool)
{
}

MouseHookListenerAsyncWrapper::~MouseHookListenerAsyncWrapper()
{
}

void MouseHookListenerAsyncWrapper::Notify(LPARAM lParam)
{
	if(lParam == 0)
		return;
	ObjectWrapper<MouseInfo> *pInfoWrapper = (ObjectWrapper<MouseInfo> *)lParam;
	MouseInfo LocalInfo = pInfoWrapper->Info();
	pInfoWrapper->Release();
	
	m_rListener.OnMouseHookEvent(LocalInfo.hwnd, LocalInfo.uMsg, LocalInfo.x, LocalInfo.y);
}

LRESULT MouseHookListenerAsyncWrapper::OnMouseHookEvent(HWND hwnd, UINT uMsg, int x, int y)
{
	ObjectWrapper<MouseInfo> *pInfoWrapper = m_rInfoPool.Get();
	if(pInfoWrapper == NULL)
	{
		Log(_T("MouseHookListenerAsyncWrapper::Notify - Error: out of MouseInfo items!\n"));
		return TRUE;
	}
	MouseInfo &rInfo = pInfoWrapper->Info();
	rInfo.hwnd = hwnd;
	rInfo.uMsg = uMsg;
	rInfo.x = x;
	rInfo.y = y;

	::PostMessage(m_hDispatcher, m_uMouseMessage, (WPARAM)this, (LPARAM)pInfoWrapper);

	return TRUE;
}
