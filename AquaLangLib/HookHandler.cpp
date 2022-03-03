#include "Pch.h"
#include "HookHandler.h"
#include "WinHookResult.h"

extern HINSTANCE g_hModule;

namespace WinHook
{
HookHandler::HookHandler()
	: m_Hook(NULL),
	m_fInHook(false),
	m_fCloseRequest(false)
{
}

HookHandler::~HookHandler()
{
	CloseHook();
}

Result HookHandler::DoOpenHook(int idHook, HOOKPROC lpfn)
{
	if(!m_Hook)
	{
		m_Hook = ::SetWindowsHookEx(idHook, lpfn, g_hModule, 0);
		if(!m_Hook)
			return Result_ResourceAllocFailed;
	}
	return Result_Success;
}

void HookHandler::CloseHook()
{
	if(m_fInHook)
	{
		m_fCloseRequest = true;
		return;
	}
	m_fCloseRequest = false;

	if(m_Hook)
	{
		::UnhookWindowsHookEx(m_Hook);
		m_Hook = NULL;
	}
}

LRESULT HookHandler::CompletedHandlingHook(
										int nCode,
										WPARAM wParam,
										LPARAM lParam
										)
{
	StartedHandlingHook();

	LRESULT lResult = ::CallNextHookEx(m_Hook, nCode, wParam, lParam);

	EndedHandlingHook();
	
	if(m_fCloseRequest)
	{
		CloseHook();
	}
	return lResult;
}

} // namespace WinHook