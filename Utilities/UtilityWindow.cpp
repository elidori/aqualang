#include "Pch.h"
#include "UtilityWindow.h"
#include "UtilityWindowParams.h"
#include "Log.h"

UtilityWindow::UtilityWindow()
	: MessageWindow()
{
}

UtilityWindow::~UtilityWindow()
{
	Close();
}

bool UtilityWindow::Open(LPCTSTR Title /*= _T("")*/)
{
	return OpenMessageWindow(Title);
}

bool UtilityWindow::Register(IWindowMessageListener &rListener)
{
	if(m_ListenerMap.find((int)(&rListener)) != m_ListenerMap.end())
		return false;
	m_ListenerMap.insert(ListenerMap::value_type((int)(&rListener), &rListener));
	return true;
}

bool UtilityWindow::Unregister(IWindowMessageListener &rListener)
{
	ListenerMap::iterator Iterator = m_ListenerMap.find((int)(&rListener));
	if(Iterator == m_ListenerMap.end())
		return false;
	m_ListenerMap.erase(Iterator);
	return true;
}

void UtilityWindow::Close()
{
	while(m_TimerIdMap.size() > 0)
	{
		TimerIdMap::iterator Iterator = m_TimerIdMap.begin();
		::KillTimer(GetHandle(), (*Iterator).second);
		m_TimerIdMap.erase(Iterator);
	}
	CloseMessageWindow();
}

bool UtilityWindow::StartTimer(DWORD dwTimeout, int TimerId)
{
	if(GetHandle() == NULL)
	{
		Log(_T("UtilityWindow::StartTimer - window handle is NULL\n"));
		return false;
	}

	StopTimer(TimerId);

	if(::SetTimer(GetHandle(), TimerId, dwTimeout, NULL) == 0)
		return false;

	m_TimerIdMap.insert(TimerIdMap::value_type(TimerId, TimerId));
	return true;
}

void UtilityWindow::StopTimer(int TimerId)
{
	TimerIdMap::iterator Iterator = m_TimerIdMap.find(TimerId);
	if(Iterator != m_TimerIdMap.end())
	{
		if(GetHandle() != NULL)
		{
			::KillTimer(GetHandle(), (*Iterator).second);
		}
		m_TimerIdMap.erase(Iterator);
	}
}

void UtilityWindow::UserMessage(WPARAM wParam, LPARAM lParam)
{
	MessageWindow::PostMessage(WM_UTILITY_USER_MESSAGE, wParam, lParam);
}

LRESULT UtilityWindow::WindowProc(HWND UNUSED(hwnd), UINT uMsg, WPARAM wParam, LPARAM lParam, bool &rfContinue)
{
	rfContinue = true;

	ListenerMap::iterator Iterator = m_ListenerMap.begin();
	for(; Iterator != m_ListenerMap.end(); Iterator++)
	{
		(*Iterator).second->Message(uMsg, wParam, lParam);
	}
	return TRUE;
}
