#include "Pch.h"
#include "UserInformationNotifier.h"
#include "AquaLangConfiguration.h"
#include "VersionInfo.h"
#include "StringConvert.h"
#include "TimeHelpers.h"
#include "Log.h"

#define MINUTES_TO_MSEC_UNITS (60 * 1000)
#define DEFAULT_NOTIFICATION_INTERVAL_MINUTES (60 * 24 * 7) // A week
#define POLLING_TIME (1000 * 3600 * 24)

UserInformationNotifier::UserInformationNotifier()
	: m_nNotificationIntervalMinutes(-1),
	m_Configuration(),
	m_WebSender(0)
{
}

UserInformationNotifier::~UserInformationNotifier()
{
	Close();
}

Result UserInformationNotifier::Open(LPCTSTR UserName, const AquaLangUserInfoNotification &rConfig)
{
	Close();

	if(strlen(rConfig.ServerUrl) == 0)
		return Result_InvalidParameter;
	m_ListenerUrl = rConfig.ServerUrl;
	m_nNotificationIntervalMinutes = rConfig.NotificationIntervalMinutes;
	if(m_nNotificationIntervalMinutes == 0)
	{
		m_nNotificationIntervalMinutes = DEFAULT_NOTIFICATION_INTERVAL_MINUTES;
	}

	m_Configuration.Open(UserName);

	m_Configuration.Load();

	if(!BuildUserString(UserName))
		return Result_InternalError;
	
	if(!OpenThread())
		return Result_ResourceAllocFailed;

	return Result_Success;
}

void UserInformationNotifier::Close()
{
	CloseThread();
}

void UserInformationNotifier::IncreaseNumberOfFocusEvents()
{
	m_Configuration.IncreaseFocusEvents();
}

void UserInformationNotifier::IncreaseNumberOfLanguageChangeEvents()
{
	m_Configuration.IncreaseLanguageChangeEvents();
}

void UserInformationNotifier::IncreaseLanguageManualChangeEvents()
{
	m_Configuration.IncreaseLanguageManualChangeEvents();
}

void UserInformationNotifier::IncreaseLanguageManualFixEvents()
{
	// aqualang automatically changed the language, but the user changed it to another. This is a parameter to a non-quality of the application
	m_Configuration.IncreaseLanguageManualFixEvents();
}

void UserInformationNotifier::IncreaseTextLanguageChangeEvents()
{
	m_Configuration.IncreaseTextLanguageChangeEvents();
}

void UserInformationNotifier::IncreaseWebSearchEvents()
{
	m_Configuration.IncreaseWebSearchEvents();
}

void UserInformationNotifier::IncreaseCalcRequestEvents()
{
	m_Configuration.IncreaseCalcRequestEvents();
}

void UserInformationNotifier::NotifyWatchdogRecovery()
{
	m_Configuration.NotifyWatchdogRecovery();
}

void UserInformationNotifier::IncreaseGuessLanguageAdviseEvents()
{
	m_Configuration.IncreaseGuessLanguageAdviseEvents();
}

void UserInformationNotifier::IncreaseGuessLanguageAcceptEvents()
{
	m_Configuration.IncreaseGuessLanguageAcceptEvents();
}

bool UserInformationNotifier::FillStatistics(AquaLangStatistics &rStatistics)
{
	return m_Configuration.FillStatistics(rStatistics);
}

bool UserInformationNotifier::BuildUserString(LPCTSTR UserName)
{
	// <ComputerName>:<UserName>:<DomainName>: Message
	TCHAR Name[256];
	DWORD Size = sizeof(Name) / sizeof(Name[0]);
	if(!::GetComputerName(Name, &Size))
		return false;
	m_UserString = Name;
	m_UserString += _T(":");

	if(UserName && _tcslen(UserName) > 0)
	{
		m_UserString += UserName;
	}
	else
	{
		Size = sizeof(Name) / sizeof(Name[0]);
		if(!::GetUserName(Name, &Size))
			return false;
		m_UserString += Name;
	}

	Size = sizeof(Name) / sizeof(Name[0]);
	if(::GetComputerNameEx(ComputerNameDnsDomain, Name, &Size))
	{
		if(_tcslen(Name) > 0)
		{
			m_UserString += _T(":");
			m_UserString += Name;
		}
	}

	return true;
}

void UserInformationNotifier::UpdateNextTimeToNotify()
{
	FileTime NextTime;
	NextTime.Add(m_nNotificationIntervalMinutes * MINUTES_TO_MSEC_UNITS);

	m_Configuration.m_NextTimeToNotify.SetTime(NextTime.GetFILETIME());
	m_Configuration.m_Version.SetString(_T(VERSION_STRING));
}

bool UserInformationNotifier::CheckIfNeedToNotify() const
{
	FILETIME NotifyTime;
	if(!m_Configuration.m_NextTimeToNotify.GetTime(NotifyTime))
	{
		assert(0);
		return true;
	}

	FileTime CurrentTime;
	__int64 Diff = CurrentTime.GetDiffMsec(NotifyTime);

	if(Diff >= 0)
		return true;
	return false;
}

bool UserInformationNotifier::SendNotification(LPCTSTR lpFormat, ...)
{
	TCHAR Message[2048];
    va_list args;
    va_start(args, lpFormat);
    _vstprintf_s(Message, sizeof(Message) / sizeof(Message[0]), lpFormat, args);
    va_end(args);

	_tstring ComposedMessage = m_UserString + _T(": ") + Message;
	Log(_T("UserInformationNotifier::SendNotification - %s\n"), ComposedMessage.c_str());

	int Result = m_WebSender.SendHTTPMessage(m_ListenerUrl.c_str(), ComposedMessage.c_str());
	if(Result != HTTP_NO_ERROR)
	{
		Log(_T("UserInformationNotifier::SendNotification - failed sending (result=%d). Will try later\n"), Result);
//		return false;
	}
	return true;
}

DWORD UserInformationNotifier::RunInThread()
{
	// calculate polling interval
	assert(m_nNotificationIntervalMinutes > 0);
	__int64 PollingTimeMsec = POLLING_TIME;
	if(PollingTimeMsec > m_nNotificationIntervalMinutes * 60 * 1000)
	{
		PollingTimeMsec = m_nNotificationIntervalMinutes * 60 * 1000 / 2;
	}

	// initialize notification status
	bool fShouldNotifyOnStart = false;
	if(!m_Configuration.VerifyVersion(_T(VERSION_STRING)) || !m_Configuration.m_NextTimeToNotify.IsValid())
	{
		fShouldNotifyOnStart = true;
	}

	while(!IsBreaking())
	{
		bool fSent = false;
		if(fShouldNotifyOnStart)
		{
			OSVERSIONINFO VersionInfo;
			VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);

			if(::GetVersionEx(&VersionInfo))
			{
				fSent = SendNotification(_T("Usage of version %s started (WinVer=%d.%d)"), _T(VERSION_STRING), VersionInfo.dwMajorVersion, VersionInfo.dwMinorVersion);
			}
			else
			{
				fSent = SendNotification(_T("Usage of version %s started"), _T(VERSION_STRING));
			}
		}
		else if(CheckIfNeedToNotify())
		{
			fSent = SendNotification(
					_T("F=%I64d,AL=%I64d,ML=%I64d,MF=%I64d,TX=%I64d,WS=%I64d,C=%I64d,WD=%I64d,GL=%I64d,GLA=%I64d,SEQ=%I64d"),
					m_Configuration.m_FocusEvents.GetInt(),
					m_Configuration.m_LanguageChangeEvents.GetInt(),
					m_Configuration.m_ManualLanguageChangeEvents.GetInt(),
					m_Configuration.m_ManualLanguageFixEvents.GetInt(),
					m_Configuration.m_TextLanguageChangeEvents.GetInt(),
					m_Configuration.m_WebSearchEvents.GetInt(),
					m_Configuration.m_CalcRequestEvents.GetInt(),
					m_Configuration.m_WatchdogRecoveryEvents.GetInt(),
					m_Configuration.m_GuessLanguageAdviseEvents.GetInt(),
					m_Configuration.m_GuessLanguageAcceptEvents.GetInt(),
					m_Configuration.m_SequenceNumber.GetInt()
					);

			m_Configuration.m_SequenceNumber.SetInt(m_Configuration.m_SequenceNumber.GetInt(0) + 1);
		}

		if(fSent)
		{
			fShouldNotifyOnStart = false;
			UpdateNextTimeToNotify();
			m_Configuration.Store();
		}
		if(IsBreaking())
			break;

		m_hWaitEvent.Wait((int)PollingTimeMsec);
	}
	m_Configuration.Store();
	return 0;
}

void UserInformationNotifier::BreakThread()
{
	m_WebSender.EndSession();
	m_hWaitEvent.Set();
}
