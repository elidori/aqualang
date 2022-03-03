#include "Pch.h"
#include "AsyncEventHandler.h"
#include "UtilityWindowParams.h"
#include "Log.h"

AsyncEventHandler::AsyncEventHandler(IAsyncEventHandlerTarget &rTarget)
	: m_rTarget(rTarget),
	m_PendingAsyncRequests(0)
{
}

AsyncEventHandler::~AsyncEventHandler()
{
	CloseEventHandler();
}

bool AsyncEventHandler::OpenEventHandler()
{
	if(!m_UtilityWindow.Register(*this))
		return false;
	if(!OpenThread())
		return false;
	return true;
}

void AsyncEventHandler::CloseEventHandler()
{
	CloseThread();
	m_UtilityWindow.StopTimer(TIMER_ID_ASYNC_FOCUS_HANDLE);
	m_UtilityWindow.Unregister(*this);
	m_UtilityWindow.Close(); // closing thread does that, but in any case ...
	m_PendingAsyncRequests.Set(0);
}

void AsyncEventHandler::EventRequestAsynchronous(int DelayMsec)
{
	m_PendingAsyncRequests.Increment();
	m_UtilityWindow.PostMessage(WM_ASYNCEVENTREQUEST, (WPARAM)DelayMsec, 0);
}

void AsyncEventHandler::EventRequestCritical()
{
	m_UtilityWindow.SendMessage(WM_CRITICALEVENTREQUEST, 0, 0);
}

DWORD AsyncEventHandler::RunInThread()
{
	if(!m_UtilityWindow.Open(_T("AsyncEventHandlers message window")))
	{
		Log(_T("AsyncEventHandler::RunInThread - failed opening utility window\n"));
		return (DWORD)-1;
	}

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

void AsyncEventHandler::BreakThread()
{
	m_UtilityWindow.PostMessage(WM_CLOSEEVENTHANDLER, 0, 0);
}

void AsyncEventHandler::Message(UINT uMsg, WPARAM wParam, LPARAM UNUSED(lParam))
{
	switch(uMsg)
	{
	case WM_ASYNCEVENTREQUEST:
		m_UtilityWindow.StopTimer(TIMER_ID_ASYNC_FOCUS_HANDLE);
		if(m_PendingAsyncRequests.Decrement() == 0) // if there are additional pending delayed requests, don't handle this one
		{
			m_UtilityWindow.StartTimer((int)wParam, TIMER_ID_ASYNC_FOCUS_HANDLE);
		}
		break;
	case WM_CRITICALEVENTREQUEST:
		m_rTarget.HandleCritical();
		break;
	case WM_CLOSEEVENTHANDLER:
		break;
	case WM_TIMER:
		if(wParam == TIMER_ID_ASYNC_FOCUS_HANDLE)
		{
			m_UtilityWindow.StopTimer(TIMER_ID_ASYNC_FOCUS_HANDLE);
			m_rTarget.HandleAsynchronous();
		}
		break;
	}
}
