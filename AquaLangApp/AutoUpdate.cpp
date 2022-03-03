#include "Pch.h"
#include "AutoUpdate.h"
#include "AquaLangConfiguration.h"
#include "VersionUpdater.h"
#include "TimeHelpers.h"
#include "Log.h"

#define MINUTES_TO_MSEC_UNITS (60 * 1000)
#define DEFAULT_CHECK_INTERVAL_MINUTES  (60 * 24 * 7) // A week
#define POLLING_TIME (1000 * 3600 * 24)

AutoUpdate::AutoUpdate()
	: m_nUpdateCheckIntervalMinutes(-1),
	m_fCheckForBetaVersion(false),
	m_Configuration()
{
}

AutoUpdate::~AutoUpdate()
{
	Close();
}

bool AutoUpdate::Open(LPCTSTR UserName, const AquaLangUpdate &rVersionUpdateSettings)
{
	Close();

	if(UserName)
	{
		m_UserName = UserName;
	}

	m_DownloadServerUrl = rVersionUpdateSettings.ServerUrl;
	m_nUpdateCheckIntervalMinutes = rVersionUpdateSettings.UpdateCheckIntervalMinutes;
	m_fCheckForBetaVersion = rVersionUpdateSettings.fCheckForBetaVersion;
	if(m_nUpdateCheckIntervalMinutes == 0)
	{
		m_nUpdateCheckIntervalMinutes = DEFAULT_CHECK_INTERVAL_MINUTES;
	}
	m_Configuration.Open(UserName);
	m_Configuration.Load();

	if(!OpenThread())
	{
		Log(_T("AutoUpdate::Open - Failed opening thread\n"));
		return false;
	}
	return true;
}

void AutoUpdate::Close()
{
	CloseThread();
	m_UserName = _T("");
}

DWORD AutoUpdate::RunInThread()
{
	// calculate polling interval
	assert(m_nUpdateCheckIntervalMinutes > 0);
	int PollingTimeMsec = POLLING_TIME;
	if((__int64)PollingTimeMsec > m_nUpdateCheckIntervalMinutes * 60 * 1000)
	{
		PollingTimeMsec = (int)m_nUpdateCheckIntervalMinutes * 60 * 1000 / 2;
	}

	// main loop
	while(!IsBreaking())
	{
		if(CheckIfNeedToNotify())
		{
			UpdateNextTimeToNotify();

			bool fShouldRunUpgrade;
			m_UpgraderDlg.Run(m_UserName.c_str(), m_DownloadServerUrl.c_str(), m_fCheckForBetaVersion, fShouldRunUpgrade, true);
			if(fShouldRunUpgrade)
			{
				VersionUpdater::RunUpgrader(m_UserName.c_str());
			}
		}
		if(IsBreaking())
			break;

		m_hWaitEvent.Wait(PollingTimeMsec);
	}
	m_Configuration.Store();
	return 0;
}

void AutoUpdate::BreakThread()
{
	m_UpgraderDlg.BreakRun();
	m_hWaitEvent.Set();
}

void AutoUpdate::UpdateNextTimeToNotify()
{
	FileTime NextTime;
	NextTime.Add(m_nUpdateCheckIntervalMinutes * MINUTES_TO_MSEC_UNITS);
	m_Configuration.m_NextTimeToCheck.SetTime(NextTime.GetFILETIME());
}

bool AutoUpdate::CheckIfNeedToNotify() const
{
	if(!m_Configuration.m_NextTimeToCheck.IsValid())
		return true;

	FILETIME CheckTime;
	if(!m_Configuration.m_NextTimeToCheck.GetTime(CheckTime))
	{
		assert(0);
		return true;
	}

	FileTime CurrentTime;
	__int64 Diff = CurrentTime.GetDiffMsec(CheckTime);

	if(Diff >= 0)
		return true;
	return false;
}
