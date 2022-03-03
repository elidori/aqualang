#include "Pch.h"
#include "ClipboardSwapper.h"
#include "KeyboardSequenceEvent.h"
#include "KeyboardEventInfo.h"
#include "ClipboardAccess.h"
#include "Log.h"

ClipboardSwapper::ClipboardSwapper(UtilityWindow &rUtilityWindow, KeyboardSequenceEvent &rKeyboardSequenceEvent)
	: m_rUtilityWindow(rUtilityWindow),
	m_rKeyboardSequenceEvent(rKeyboardSequenceEvent),
	m_pCallback(NULL)
{
}

ClipboardSwapper::~ClipboardSwapper()
{
	Close();
}

bool ClipboardSwapper::Open(LPCTSTR KeyboardTrigger)
{
	Result res = m_rKeyboardSequenceEvent.Register(KeyboardTrigger, *this, false);
	if(res != Result_Success)
		return false;
	if(!m_rUtilityWindow.Register(*this))
		return false;
	return true;
}

void ClipboardSwapper::Close()
{
	m_rKeyboardSequenceEvent.Unregister(*this);
	m_rUtilityWindow.Unregister(*this);
	m_pCallback = NULL;
}

bool ClipboardSwapper::CanOperate(KeyboardEventInfo & UNUSED(Info), bool &fExclusive)
{
	fExclusive = false;
	return true;
}

void ClipboardSwapper::Operate(const KeyboardEventInfo &Info, IKeyboardEventCallback &rCallback)
{
	m_pCallback = &rCallback;
	m_rUtilityWindow.UserMessage(WPARAM_CLIPBOARD_SWAP, (LPARAM)Info.GetCurrentWindow());
}

void ClipboardSwapper::Message(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_UTILITY_USER_MESSAGE:
		switch(wParam)
		{
		case WPARAM_CLIPBOARD_SWAP:
			ClipboardAccess::CutAndToggelWithClipboard((HWND)lParam);

			assert(m_pCallback != NULL);
			m_pCallback->EventOperationEnded(true);
			m_pCallback = NULL;
			break;
		}
		break;
	}
}
