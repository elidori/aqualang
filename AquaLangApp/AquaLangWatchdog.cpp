#include "Pch.h"
#include "AquaLangWatchdog.h"
#include "AquaLangLibLoader.h"
#include "UtilityWindowParams.h"
#include "AquaLangWaterMark.h"
#include "ApplicationDataPath.h"
#include "DialogObject.h"
#include "AquaLangImpl.h"
#include "Log.h"

#define INJECT_KB_EVENT_TEST_INTERVAL 10

#define WATCHDOG_RESPONSE_EVENT_NAME _T("AquaLangWatchdogResponseEvent_")
#define WATCHDOG_WINDOW_NAME _T("AquaLangWatchdogWindow_")

#define WATCHDOG_CREATION_TIMEOUT 10000
#define WATCHDOG_DESTRUCTION_TIMEOUT 5000

#define KB_EVENTS_CHECK_PERIOD (3600 * 1000)

enum
{
	WATCHDOG_TIMER_ID = 1,
};

enum
{
	WPARAM_WATCHDOG_DESTROY = 0,
	WPARAM_WATCHDOG_ALIVE = 1,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
// AquaLangWatchdogProcess
////////////////////////////////////////////////////////////////////////////////////////////////////////
AquaLangWatchdogProcess::AquaLangWatchdogProcess(AquaLangImpl &rAquaLangStopper)
	: m_rAquaLangStopper(rAquaLangStopper),
	m_UserName(),
	m_fBreak(false),
	m_fAlive(false)
{
}

AquaLangWatchdogProcess::~AquaLangWatchdogProcess()
{
}

bool AquaLangWatchdogProcess::Run()
{
	m_fBreak = false;

	_tstring WatchdogResponseEventName = WATCHDOG_RESPONSE_EVENT_NAME;
	WatchdogResponseEventName += m_UserName;
	ProcessEvent WatchdogResponseEvent(WatchdogResponseEventName.c_str());
	if(!WatchdogResponseEvent.Open())
	{
		Log(_T("AquaLangWatchdogProcess::Run - failed opening event\n"));
		return false;
	}

	_tstring WatchdogWindowName = WATCHDOG_WINDOW_NAME;
	WatchdogWindowName += m_UserName;
	UtilityWindow _UtilityWindow;
	if(!_UtilityWindow.Open(WatchdogWindowName.c_str()))
	{
		Log(_T("AquaLangWatchdogProcess::Run - failed opening utility window\n"));
		return false;
	}
	AquaLangWaterMark WaterMark;
	if(!WaterMark.Open(m_UserName.c_str(), _UtilityWindow.GetHandle()))
	{
		Log(_T("AquaLangWatchdogProcess::Run - failed creating watermark\n"));
		return false;
	}

	if(!_UtilityWindow.Register(*this))
	{
		Log(_T("AquaLangWatchdogProcess::Run - failed registering on utility window\n"));
		return false;
	}
	if(!_UtilityWindow.StartTimer(WATCHDOG_CHECK_CYCLE, WATCHDOG_TIMER_ID))
	{
		Log(_T("AquaLangWatchdogProcess::Run - failed starting timer in utility window\n"));
		return false;
	}

	WatchdogResponseEvent.Set();

	MSG msg;
	while(!m_fBreak)
	{
		if(::GetMessage(&msg, NULL, NULL, NULL))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	_UtilityWindow.StopTimer(WATCHDOG_TIMER_ID);
	_UtilityWindow.Unregister(*this);
	WaterMark.Close();
	_UtilityWindow.Close();

	WatchdogResponseEvent.Set();

	return true;
}

void AquaLangWatchdogProcess::Message(UINT uMsg, WPARAM wParam, LPARAM UNUSED(lParam))
{
	switch(uMsg)
	{
	case WM_UTILITY_USER_MESSAGE:
		switch(wParam)
		{
		case WPARAM_WATCHDOG_DESTROY:
			m_fBreak = true;
			break;
		case WPARAM_WATCHDOG_ALIVE:
			m_fAlive = true;
			break;
		}
		break;
	case WM_TIMER:
		if(wParam == WATCHDOG_TIMER_ID)
		{
			if(!m_fAlive || TimeForPeriodicRecovery())
			{
				Recover();
			}
			m_fAlive = false;
		}
		break;
	}
}

bool AquaLangWatchdogProcess::TimeForPeriodicRecovery() const
{
	if(m_LastRecoverTime.GetDiff() < (12 * 3600 * 1000))
		return false;

	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	if(LocalTime.wHour == 0) // between 12:00AM to 1:00AM
	{
		Log(_T("AquaLangWatchdogProcess::TimeForPeriodicRecovery() - time for periodic recovery\n"));
		return true;
	}

	return false;
}

bool AquaLangWatchdogProcess::Recover()
{
	Log(_T("AquaLangWatchdogProcess::Recover - Enter\n"));

	if(!m_rAquaLangStopper.Stop(false))
	{
		Log(_T("AquaLangWatchdogProcess::Recover - failed to stop aqualang\n"));
		return false;
	}

	ApplicationDataPath PathFinder(NULL);
	_tstring ApplicationName;
	PathFinder.GetApplicationName(ApplicationName);

	if(!Process::Create(ApplicationName.c_str(), _T("-runfromwatchdog"), _T("AquaLang recovered application")))
	{
		Log(_T("AquaLangWatchdogProcess::Recover - failed to start aqualang\n"));
		return false;
	}

	m_LastRecoverTime.Restart();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// AquaLangWatchdogActivation
////////////////////////////////////////////////////////////////////////////////////////////////////////
AquaLangWatchdogActivation::AquaLangWatchdogActivation(AquaLangLibLoader &rAquaLangLib)
	: m_UserName(),
	m_WatchdogResponseEvent(_T("")),
	m_fEnabled(true),
	m_Tester(rAquaLangLib)
{
}

AquaLangWatchdogActivation::~AquaLangWatchdogActivation()
{
}

void AquaLangWatchdogActivation::SetUserName(LPCTSTR UserName)
{
	m_UserName = UserName;
	m_WatchdogResponseEvent.UpdateName((_tstring(WATCHDOG_RESPONSE_EVENT_NAME) + UserName).c_str());
}

bool AquaLangWatchdogActivation::CreateWatchdog()
{
	DestroyWatchdog();

	if(!m_WatchdogResponseEvent.IsOpen())
	{
		if(!m_WatchdogResponseEvent.Create())
		{
			Log(_T("AquaLangWatchdogActivation::CreateWatchdog - failed creating event\n"));
			return false;
		}
	}
	m_WatchdogResponseEvent.Reset();

	ApplicationDataPath PathFinder(NULL);
	_tstring ApplicationName;
	PathFinder.GetApplicationName(ApplicationName);

	if(!Process::Create(ApplicationName.c_str(), _T("-watchdog"), _T("AquaLang Watchdog process")))
		return false;

	if(!m_WatchdogResponseEvent.Wait(WATCHDOG_CREATION_TIMEOUT))
	{
		Log(_T("AquaLangWatchdogActivation::CreateWatchdog - timeout waiting for process to start\n"));
		return false;
	}

	_tstring WatchdogWindowName = WATCHDOG_WINDOW_NAME;
	WatchdogWindowName += m_UserName;
	HWND hWatchdogWindow = DialogObject::FindAquaLangWindow(WatchdogWindowName.c_str(), m_UserName.c_str());
	if(hWatchdogWindow == NULL)
	{
		Log(_T("AquaLangWatchdogActivation::CreateWatchdog - did not find window\n"));
		return false;
	}

	m_LastTimeKbWasTested.Restart();

	return true;
}

bool AquaLangWatchdogActivation::DestroyWatchdog()
{
	_tstring WatchdogWindowName = WATCHDOG_WINDOW_NAME;
	WatchdogWindowName += m_UserName;
	HWND hWatchdogWindow = DialogObject::FindAquaLangWindow(WatchdogWindowName.c_str(), m_UserName.c_str());
	if(hWatchdogWindow == NULL)
		return true;

	if(!m_WatchdogResponseEvent.IsOpen())
	{
		if(!m_WatchdogResponseEvent.Create())
		{
			Log(_T("AquaLangWatchdogActivation::DestroyWatchdog - failed creating event\n"));
			return false;
		}
	}
	m_WatchdogResponseEvent.Reset();

	::PostMessage(hWatchdogWindow, WM_UTILITY_USER_MESSAGE, WPARAM_WATCHDOG_DESTROY, 0);

	if(!m_WatchdogResponseEvent.Wait(WATCHDOG_DESTRUCTION_TIMEOUT))
	{
		Log(_T("AquaLangWatchdogActivation::DestroyWatchdog - timeout waiting for process to stop\n"));
		return false;
	}
	return true;
}

bool AquaLangWatchdogActivation::KeepAlive()
{
	bool fOK = true;

	if(m_LastTimeKbWasTested.GetDiff() >= KB_EVENTS_CHECK_PERIOD)
	{
		m_LastTimeKbWasTested.Restart();

		if(m_fEnabled)
		{
			fOK = m_Tester.CheckKeyboardEvents();
		}
	}

	if(fOK)
	{
		NotifyAlive();
	}
	return fOK;
}

bool AquaLangWatchdogActivation::NotifyAlive()
{
	_tstring WatchdogWindowName = WATCHDOG_WINDOW_NAME;
	WatchdogWindowName += m_UserName;
	HWND hWatchdogWindow = DialogObject::FindAquaLangWindow(WatchdogWindowName.c_str(), m_UserName.c_str());
	if(hWatchdogWindow == NULL)
		return false;

	::PostMessage(hWatchdogWindow, WM_UTILITY_USER_MESSAGE, WPARAM_WATCHDOG_ALIVE, 0);
	return true;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////
// AquaLangWatchdogTester
////////////////////////////////////////////////////////////////////////////////////////////////////////
AquaLangWatchdogTester::AquaLangWatchdogTester(AquaLangLibLoader &rAquaLangLib)
	: m_rAquaLangLib(rAquaLangLib),
	m_fTestInjectedKbEvent(false)
{
	memset(&m_PreviousStatistics, 0, sizeof(m_PreviousStatistics));
}

AquaLangWatchdogTester::~AquaLangWatchdogTester()
{
}

bool AquaLangWatchdogTester::CheckKeyboardEvents()
{
	// clear m_fTestInjectedKbEvent
	bool fTestInjectedKbEvent = m_fTestInjectedKbEvent;
	m_fTestInjectedKbEvent = false;

	// check event counter
	AquaLangStatistics PresentStatistics;
	if(!m_rAquaLangLib.GetAquaLangStatistics(PresentStatistics))
		return true;

	__int64 AdditionalKbEvents = PresentStatistics.KeyboardEvents - m_PreviousStatistics.KeyboardEvents;
	__int64 AdditionalBlockedKbEvents = PresentStatistics.BlockedKeyboardEvents - m_PreviousStatistics.BlockedKeyboardEvents;
	m_PreviousStatistics = PresentStatistics;

	if(AdditionalKbEvents == 0)
	{
		// no new events were found since last check. The reasons for that are as follows:
		// - the block of events is set for too long (AdditionalBlockedKbEvents > 0) - Error
		// - real kb events are not identified by the program (this is tested by injection of kb events and retesting the statistics) - Error
		// - kb events were really not present for that period of time - a legal state
		if(AdditionalBlockedKbEvents > 0)
			return false;

		if(fTestInjectedKbEvent)
			return false;

		// inject test keyboard event, and try to see its affect next time
		m_rAquaLangLib.PrintKey(VK_LSHIFT, false);
		m_fTestInjectedKbEvent = true;
	}
	return true;
}
