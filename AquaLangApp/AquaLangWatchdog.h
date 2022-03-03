#ifndef _AQUALANG_WATCHDOG_HEADER_
#define _AQUALANG_WATCHDOG_HEADER_

#include "AquaLangStatistics.h"
#include "UtilityWindow.h"
#include "ProcessUtils.h"
#include "TimeHelpers.h"

#define WATCHDOG_CHECK_CYCLE (60 * 1000) // 60 seconds

class AquaLangLibLoader;
class AquaLangImpl;

class AquaLangWatchdogProcess : public IWindowMessageListener
{
public:
	AquaLangWatchdogProcess(AquaLangImpl &rAquaLangStopper);
	virtual ~AquaLangWatchdogProcess();
	
	void SetUserName(LPCTSTR UserName)	{ m_UserName = UserName; }

	bool Run();
private:
	// IWindowMessageListener pure virtuals {
	virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// }

	bool TimeForPeriodicRecovery() const;
	bool Recover();
private:
	AquaLangImpl &m_rAquaLangStopper;

	_tstring m_UserName;
	bool m_fBreak;
	bool m_fAlive;

	Timer m_LastRecoverTime;
};


class AquaLangWatchdogTester
{
public:
	AquaLangWatchdogTester(AquaLangLibLoader &rAquaLangLib);
	virtual ~AquaLangWatchdogTester();

	bool CheckKeyboardEvents();
private:
	AquaLangLibLoader &m_rAquaLangLib;

	bool m_fTestInjectedKbEvent;

	AquaLangStatistics m_PreviousStatistics;
};


class AquaLangWatchdogActivation
{
public:
	AquaLangWatchdogActivation(AquaLangLibLoader &rAquaLangLib);
	virtual ~AquaLangWatchdogActivation();

	void SetUserName(LPCTSTR UserName);

	bool CreateWatchdog();
	bool DestroyWatchdog();

	void Enable(bool fEnable)	{ m_fEnabled = fEnable; }

	bool KeepAlive();
private:
	bool NotifyAlive();
private:
	_tstring m_UserName;
	ProcessEvent m_WatchdogResponseEvent;

	bool m_fEnabled;
	AquaLangWatchdogTester m_Tester;

	Timer m_LastTimeKbWasTested;
};


#endif // _AQUALANG_WATCHDOG_HEADER_