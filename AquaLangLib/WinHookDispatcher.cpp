#include "Pch.h"
#include "WinHookDispatcher.h"
#include "WinHookListener.h"

using namespace WinHook;

HookDispatcher g_HookDispatcher;


HookDispatcher::HookDispatcher()
{
}

HookDispatcher::~HookDispatcher()
{
	CloseWindowHook();
	CloseMouseHook();
	CloseKeyboardHook();
}

Result HookDispatcher::RegisterOnWindow(HWND hwnd, IWindowListener &rListener)
{
	Result res = OpenWindowHook();
	if(Result != Result_Success)
		return res;

	WindowMessageLisenerMap
}

Result HookDispatcher::RegisterOnMessage(UINT uMsg, IWindowListener &rListener)
{
}

Result HookDispatcher::RegisterOnWindowMouseEvents(HWND hwnd, IMouseListener &rListener)
{
}

Result HookDispatcher::RegisterOnMouseEvent(UINT uMsg, IMouseListener &rListener)
{
}

Result HookDispatcher::RegisterOnKeyboardEvent(int Code, IKeyboardListener &rListener)
{
}

Result HookDispatcher::Unregister(const IListener &rListener)
{
}

Result HookDispatcher::OpenWindowHook()
{
}

Result HookDispatcher::CloseWindowHook()
{
}

Result HookDispatcher::OpenMouseHook()
{
}

Result HookDispatcher::CloseMouseHook()
{
}

Result HookDispatcher::OpenKeyboardHook()
{
}

Result HookDispatcher::CloseKeyboardHook()
{
}

LRESULT CALLBACK HookDispatcher::CallWndProc(
			int nCode,
			WPARAM wParam,
			LPARAM lParam
			)
{
}

LRESULT CALLBACK HookDispatcher::MouseProc(
			int nCode,
			WPARAM wParam,
			LPARAM lParam
			)
{
}

LRESULT CALLBACK HookDispatcher::LowLevelKeyboardProc(
			int code,
			WPARAM wParam,
			LPARAM lParam
			)
{
}


















KeyboardHook::KeyboardHook()
	: m_KeyboardHook(NULL),
	m_pListener(NULL)
{
}

KeyboardHook::~KeyboardHook()
{
	Close();
}

void KeyboardHook::Set(IKeyboardHookListener &rListener)
{
	Open();
	m_pListener = &rListener;
}

void KeyboardHook::Clear()
{
	Close();
}

bool KeyboardHook::Open()
{
	if(!m_KeyboardHook)
	{
		m_KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, ::GetModuleHandle(NULL), 0);
		if(!m_KeyboardHook)
			return false;
	}
	return true;
}

void KeyboardHook::Close()
{
	if(m_KeyboardHook)
	{
		UnhookWindowsHookEx(m_KeyboardHook);
		m_KeyboardHook = NULL;
	}
	m_pListener = NULL;
}

void KeyboardHook::HandleKeyboardEvent(
		int nCode,      // hook code
		WPARAM wParam,  // current-process flag
		LPARAM lParam   // message data
		)
{
	if(nCode == HC_ACTION)
	{
		KBDLLHOOKSTRUCT *pKBStruct = (KBDLLHOOKSTRUCT *)lParam;
		
		if(m_pListener)
		{
			m_pListener->OnKeyboardHookEvent(wParam, pKBStruct->vkCode);
		}
	}
}

LRESULT CALLBACK KeyboardHook::LowLevelKeyboardProc(
		int nCode,      // hook code
		WPARAM wParam,  // current-process flag
		LPARAM lParam   // message data
		)
{
	g_KeyboardHook.HandleKeyboardEvent(nCode, wParam, lParam);
	LRESULT lResult = CallNextHookEx(g_KeyboardHook.m_KeyboardHook, nCode, wParam, lParam);

	return lResult;
}
