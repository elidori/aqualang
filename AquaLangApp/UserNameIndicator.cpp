#include "Pch.h"
#include "UserNameIndicator.h"
#include "resource.h"
#include "Log.h"

extern HMODULE g_hModule;

#define UPGRADE_INDICATOR_NAME _T("AquaLang Upgrader Window")
#define UPGRADE_INDICATOR_EVENT_NAME _T("AquaLang_Upgrade_Indicator_Event")

#define USERNAME_INDICATOR_CLOSE_RETRIES_COUNT 5
#define USERNAME_INDICATOR_POLLING_INTERVAL 100

UserNameIndicator::UserNameIndicator()
	: DialogObject(-1, false),
	m_UserNameQueryEvent(UPGRADE_INDICATOR_EVENT_NAME)
{
}

UserNameIndicator::~UserNameIndicator()
{
	Close();
}

bool UserNameIndicator::Open(LPCTSTR UserName)
{
	Log(_T("UserNameIndicator::Open - user name is %s\n"), UserName);

	HWND hDialog = OpenDialog(UserName, IDD_DIALOG_USERNAME_INDICATOR);
	if(hDialog == NULL)
		return false;

	if(!SetTitle(UPGRADE_INDICATOR_NAME))
	{
		Log(_T("UserNameIndicator::Open - failed setting dialog text\n"));
		Close();
		return false;
	}
	if(!::SetWindowText(::GetDlgItem(hDialog, IDC_STATIC_USERNAME_INDICATOR), UserName))
	{
		Log(_T("UserNameIndicator::Open - failed setting static text\n"));
		Close();
		return false;
	}

	if(!m_UserNameQueryEvent.Create())
	{
		Log(_T("UserNameIndicator::Open - failed creating event\n"));
		Close();
		return false;
	}

	return true;
}

bool UserNameIndicator::WaitForUserNameQuery(DWORD dwTimeout)
{
	if(!m_UserNameQueryEvent.Wait(dwTimeout))
	{
		Log(_T("UserNameIndicator::WaitForUserNameQuery - timeout waiting for event\n"));
		return false;
	}
	Log(_T("UserNameIndicator::WaitForUserNameQuery - Received acknowledge!\n"));
	return true;
}

bool UserNameIndicator::FindUserName(_tstring &rUserName)
{
	Log(_T("UserNameIndicator::FindUserName\n"));
	HWND hUpgraderWindow = ::FindWindow(NULL, UPGRADE_INDICATOR_NAME);
	if(hUpgraderWindow == NULL)
	{
		Log(_T("UserNameIndicator::FindUserName - did not find indicator window\n"));
		return false;
	}

	HWND hIndicator = ::GetDlgItem(hUpgraderWindow, IDC_STATIC_USERNAME_INDICATOR);
	if(hIndicator == NULL)
	{
		Log(_T("UserNameIndicator::FindUserName - did not find static control\n"));
		return false;
	}

	TCHAR RestoredUserName[256];
	if(::GetWindowText(hIndicator, RestoredUserName, sizeof(RestoredUserName) / sizeof(RestoredUserName[0])) == 0)
	{
		Log(_T("UserNameIndicator::FindUserName - could not retrieve text\n"));
		return false;
	}

	Log(_T("UserNameIndicator::FindUserName - user name is %s\n"), RestoredUserName);
	rUserName = RestoredUserName;

	UserNameIndicator IndicatorClient;
	if(!IndicatorClient.SignalForUserNameQueryComplete())
		return false;

	return true;
}

bool UserNameIndicator::SignalForUserNameQueryComplete()
{
	if(!m_UserNameQueryEvent.Open())
	{
		Log(_T("UserNameIndicator::SignalForUserNameQueryComplete - failed opening event\n"));
		return false;
	}
	m_UserNameQueryEvent.Set();

	int Retries = USERNAME_INDICATOR_CLOSE_RETRIES_COUNT;

	while(::FindWindow(NULL, UPGRADE_INDICATOR_NAME) != NULL)
	{
		if(Retries > 0)
		{
			Retries--;
			Sleep(USERNAME_INDICATOR_POLLING_INTERVAL);
		}
		else
		{
			Log(_T("UserNameIndicator::SignalForUserNameQueryComplete - timeout waiting for indicator to get down\n"));
			return false;
		}
	}
	Log(_T("UserNameIndicator::SignalForUserNameQueryComplete - indicator is sucessfully down\n"));
	return true;
}

void UserNameIndicator::Close()
{
	m_UserNameQueryEvent.Destroy();
	CloseDialog();
}
