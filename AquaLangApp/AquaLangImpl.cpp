#include "Pch.h"
#include "AquaLangImpl.h"
#include "AquaLangConfiguration.h"
#include "ConfigurationManager.h"
#include "ApplicationDataPath.h"
#include "VersionUpdater.h"
#include "ConfigAdvisorManager.h"
#include "UserNameIndicator.h"
#include "UpgraderDialog.h"
#include "ConfigurationMessages.h"
#include "UtilityWindowParams.h"
#include "resource.h"
#include "Log.h"

#define STOP_WAIT_TIMEOUT 400
#define STOP_RETRIES_COUNT 20

#define AQUALANG_CONFIGURATION_WINDOW_CAPTION _T("AquaLang Configuration")

#pragma warning (disable:4355)

AquaLangImpl::AquaLangImpl(LPCTSTR UserName)
	: m_RemoteCommand(m_UtilityWindow),
	m_AppCommand(AppCommand_None),
	m_NotificationArea(*this, *this, m_UtilityWindow),
	m_WatchdogProcess(*this),
	m_Watchdog(m_AquaLangLib),
	m_MessagesManager()
{
	if(UserName)
	{
		m_UserName = UserName;
	}

	if(m_UserName.length() == 0)
	{
		BuildUserName();
	}

	m_MessagesManager.SetUserName(m_UserName.c_str()); // not using UserName from the constructor parameters, as m_UserName is most likely different
	m_WatchdogProcess.SetUserName(m_UserName.c_str());
	m_Watchdog.SetUserName(m_UserName.c_str());
}

AquaLangImpl::~AquaLangImpl()
{
	m_UtilityWindow.Close();
	m_MessagesManager.CloseMessages();
}

bool AquaLangImpl::Run(bool fFirstRunAfterInstall, bool fStartUpRun, bool fRunningFromWatchdog)
{
	if(fFirstRunAfterInstall)
	{
		RestoreUserNameAfterUpgrade();
		// delete installation files
		VersionUpdater::DeleteInstallationFolder();
		// delete old versions (up to 1.1.1)
		ApplicationDataPath PathFinder(NULL);
		_tstring ApplicatioPath;
		PathFinder.ComposeRelativePath(_T(""), ApplicatioPath);
		PathFinder.DeleteDirectory((ApplicatioPath + _T("\\AquaLang\\")).c_str(), 1);
		PathFinder.DeleteDirectory(ApplicatioPath.c_str(), 1, _T("\\AquaLang *"));
	}

	if(!m_AquaLangLib.Open())
	{
		m_MessagesManager.PresentMessage(AquaLangRunningError);
		return false;
	}

	if(m_RemoteCommand.Open(m_UserName.c_str()))
	{
		if(!fStartUpRun)
		{
			m_MessagesManager.PresentMessage(AquaLangAlreadyRunning);
		}
		return false;
	}
	if(!m_RemoteCommand.Create(m_UserName.c_str()))
	{
		m_MessagesManager.PresentMessage(AquaLangRunningError);
		return false;
	}

	AquaLangConfiguration Config;

	ConfigurationManager Configurator(m_UserName.c_str());
	Configurator.LoadSettings(Config);

	ConfigAdvisorManager ConfigAdvisor;
	// not using this feature anymore
//	ConfigAdvisor.Open(AQUALANG_CONFIGURATION_WINDOW_CAPTION, m_UserName.c_str());

	_tstring Title;
	GetMainMessageWindowTitle(Title);
	m_UtilityWindow.Open(Title.c_str());
	m_UtilityWindow.Register(*this);

	if(!m_UtilityWindow.StartTimer(WATCHDOG_CHECK_CYCLE / 2, TIMER_ID_APP_WATCHDOG))
	{
		Log(_T("AquaLangImpl::Run - failed starting watchdog timer\n"));
	}

	if(!fRunningFromWatchdog)
	{
		if(!m_Watchdog.CreateWatchdog())
		{
			Log(_T("AquaLangImpl::Run - failed creating watchdog process\n"));
		}
	}

	bool fEnabled = true;
	bool fFirstRun = true;
	bool fContinue = true;
	bool fWatchdogRecovery = fRunningFromWatchdog;
	bool fPresentStopMessage = true;

	while(fContinue)
	{
		InitializeNotificationArea(m_NotificationArea, Config.NotificationArea, Config.CalcSettings.fActive);

		bool fRecovery = fWatchdogRecovery;
		fWatchdogRecovery = false;

		if(fEnabled)
		{
			if(fRecovery)
			{
				Log(_T("AquaLangImpl::Run - recovering by watchdog\n"));
			}
			if(!m_AquaLangLib.OpenAquaLang(m_UserName.c_str(), Config, m_NotificationArea, fRecovery))
			{
				m_MessagesManager.PresentMessage(AquaLangRunningError);
				m_UtilityWindow.Unregister(*this);
				m_UtilityWindow.Close();
				return false;
			}
		}

		if(fFirstRun && !fRunningFromWatchdog)
		{
			m_MessagesManager.PresentMessage(AquaLangRunning, this);
			fFirstRun = false;
		}

#if false
		if(!m_VersionUpdater.Open(m_UserName.c_str(), Config.VersionUpdateSettings))
		{
			Log(_T("AquaLangImpl::Run - failed opening version updater\n"));
		}
#endif

		// main loop
		MSG msg;
		while(m_AppCommand == AppCommand_None)
		{
			if(::GetMessage(&msg, NULL, NULL, NULL))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		
		AppCommand CurrentAppCommand = m_AppCommand;
		m_AppCommand = AppCommand_None;

		if(CurrentAppCommand == AppCommand_LogOff)
			return true; // return immediately

		m_AquaLangLib.CloseAquaLang();
		m_AquaLangLib.Close();
		m_VersionUpdater.Close();

		switch(CurrentAppCommand)
		{
		case AppCommand_None:
			Log(_T("AquaLangImpl::Run - warning: received command event with no command parameter. Assuming stop was requested\n"));
			fContinue = false;
			break;
		case AppCommand_Stop:
			m_Watchdog.DestroyWatchdog();
			fContinue = false;
			break;
		case AppCommand_StopFromWatchdog:
			fContinue = false;
			fPresentStopMessage = false;
			break;
		case AppCommand_UpdateConfig:
			Configurator.LoadUpdatedSettings(Config);
			break;
		case AppCommand_Disable:
			fEnabled = false;
			m_Watchdog.Enable(false);
			break;
		case AppCommand_Enable:
			fEnabled = true;
			m_Watchdog.Enable(true);
			break;
		}
	}
	m_UtilityWindow.StopTimer(TIMER_ID_APP_WATCHDOG);

	m_RemoteCommand.Close();

	m_NotificationArea.Close();

	m_UtilityWindow.Unregister(*this);
	m_UtilityWindow.Close();

	ConfigAdvisor.Close();

	if(fPresentStopMessage)
	{
		m_MessagesManager.PresentMessage(AquaLangStopped);
	}
	return true;
}

void AquaLangImpl::Config()
{
	ConfigurationManager Configurator(m_UserName.c_str());

	bool fAlreadyRunning = false;
	bool fSettingsWereUpdated = false;
	bool fCheckForUpdates = false;
	AquaLangConfiguration UpdatedSettings;
	Configurator.RunDialog(AQUALANG_CONFIGURATION_WINDOW_CAPTION, m_UserName.c_str(), fAlreadyRunning, fSettingsWereUpdated, fCheckForUpdates, UpdatedSettings);

	if(!fAlreadyRunning)
	{
		if(fSettingsWereUpdated)
		{
			if(m_RemoteCommand.Open(m_UserName.c_str()))
			{
				m_RemoteCommand.SendCommand(AppCommand_UpdateConfig);
			}
		}

		if(fCheckForUpdates)
		{
			UpgraderDialog VersionUpdateDlg;
			const AquaLangUpdate &rSettings = UpdatedSettings.VersionUpdateSettings;
			bool fShouldRunUpgrade;

			VersionUpdateDlg.Run(m_UserName.c_str(), rSettings.ServerUrl, rSettings.fCheckForBetaVersion, fShouldRunUpgrade, false);
			if(fShouldRunUpgrade)
			{
				VersionUpdater::RunUpgrader(m_UserName.c_str());
			}
		}
	}
}

bool AquaLangImpl::Stop(bool fIncludingWatchdog)
{
	int Retries = STOP_RETRIES_COUNT;

	while(m_RemoteCommand.Open(m_UserName.c_str()))
	{
		m_RemoteCommand.SendCommand(fIncludingWatchdog ? AppCommand_Stop : AppCommand_StopFromWatchdog);
		m_RemoteCommand.Close();

		if(Retries > 0)
		{
			Retries--;
			Sleep(STOP_WAIT_TIMEOUT);
		}
		else
		{
#ifdef _DEBUG
			MessageBox(NULL, _T("Failed stopping application"), _T("AquaLang stop"), MB_ICONASTERISK | MB_OK);
#endif
			return false;
		}
	}
	return true;
}

bool AquaLangImpl::Install()
{
	Stop(true);

	ApplicationDataPath PathFinder(m_UserName.c_str());
	_tstring ApplicationName;
	PathFinder.GetApplicationName(ApplicationName);

	_tstring Args = _T("-firstrun");
	Args += _T(" \"");
	Args += m_UserName;
	Args += _T("\"");

	if(!Process::Create(ApplicationName.c_str(), Args.c_str(), _T("AquaLang installation")))
		return false;
	return true;
}

bool AquaLangImpl::Upgrade()
{
	Stop(true);

	// stop config dialog if running
	ConfigurationManager Configurator(m_UserName.c_str());
	if(!Configurator.StopRunningDialog(AQUALANG_CONFIGURATION_WINDOW_CAPTION, m_UserName.c_str()))
	{
		MessageBox(NULL, _T("Cannot upgrade when configuration window is open"), _T("Error upgrading"), MB_ICONASTERISK | MB_OK);
		return false;
	}

	if(!VersionUpdater::Upgrade(m_UserName.c_str()))
		return false;

	return true;
}

bool AquaLangImpl::Uninstall()
{
	Stop(true);
	ConfigurationManager Configurator(m_UserName.c_str());
	Configurator.StopRunningDialog(AQUALANG_CONFIGURATION_WINDOW_CAPTION, m_UserName.c_str());

	ApplicationDataPath PathFinder(m_UserName.c_str());
	_tstring Path;
	PathFinder.ComposePath(_T(""), Path);

	ApplicationDataPath::DeleteDirectory(Path.c_str(), 1);
	return true;
}

bool AquaLangImpl::RunBrowser()
{
	if(!m_AquaLangLib.Open())
	{
		Log(_T("AquaLangImpl::RunBrowser() - failed loading library"));
		return false;
	}
	if(!m_AquaLangLib.RunBrowser(m_UserName.c_str()))
	{
		Log(_T("AquaLangImpl::RunBrowser() - failed running browser"));
		return false;
	}
	m_AquaLangLib.Close();
	return true;
}

bool AquaLangImpl::TriggerCalculator()
{
	Log(_T("AquaLangImpl::TriggerCalculator\n"));
	_tstring Title;
	GetMainMessageWindowTitle(Title);
	HWND hwnd = ::FindWindow(NULL, Title.c_str());
	if(hwnd == NULL)
	{
		Log(_T("AquaLangImpl::TriggerCalculator - failed finding window with title of %s\n"), Title.c_str());
		return false;
	}
	::PostMessage(hwnd, WM_MESSAGEWINDOW_TYPE, WPARAM_TRIGGER_CALC, 0);
	return true;
}

bool AquaLangImpl::RunWatchdog()
{
	return m_WatchdogProcess.Run();
}

void AquaLangImpl::TipShouldClose(BalloonTip &rTip)
{
	rTip.Term();
}

void AquaLangImpl::TipWasClicked(BalloonTip &rTip, bool UNUSED(fLeftClick))
{
	TipShouldClose(rTip);
}

void AquaLangImpl::OptionSelected(int OptionId)
{
	switch(OptionId)
	{
	case ID_MENU_HELP:
		{
			ApplicationDataPath PathFinder(m_UserName.c_str());
			_tstring HelpFileName;
			PathFinder.ComposeRelativePath(_T("Aqualang-HowTo.htm"), HelpFileName);

			::ShellExecute(NULL, _T("open"), HelpFileName.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		break;
	case ID_MENU_CONFIGURATION:
		{
			ApplicationDataPath PathFinder(m_UserName.c_str());
			_tstring ApplicationName;
			PathFinder.GetApplicationName(ApplicationName);
			_tstring Args = _T("-config");
			Args += _T(" \"");
			Args += m_UserName;
			Args += _T("\"");
			if(!Process::Create(ApplicationName.c_str(), Args.c_str(), _T("AquaLang Configuration Call")))
			{
				Log(_T("AquaLangImpl::ActionRequested - failed creating process\n"));
			}
		}
		break;
	case ID_MENU_CALCULATOR:
		m_AquaLangLib.TriggerCalculator();
		break;
	case ID_MENU_DISABLE:
		m_RemoteCommand.SendCommand(AppCommand_Disable);
		break;
	case ID_MENU_ENABLE:
		m_RemoteCommand.SendCommand(AppCommand_Enable);
		break;
	case ID_MENU_EXIT:
		m_RemoteCommand.SendCommand(AppCommand_Stop);
		break;
	}
}

void AquaLangImpl::ConfigurationChanged()
{
	ConfigurationManager::SendNotificationIfDialogOpened(AQUALANG_CONFIGURATION_WINDOW_CAPTION, m_UserName.c_str(), WPARARM_CONFIG_EXTERNALLY_CHANGED);
	m_RemoteCommand.SendCommand(AppCommand_UpdateConfig);
}

void AquaLangImpl::Message(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_MESSAGEWINDOW_TYPE:
		if(wParam == WPARAM_TRIGGER_CALC)
		{
			m_AquaLangLib.TriggerCalculator();
		}
		break;
	case WM_UTILITY_USER_MESSAGE:
		if(wParam == WPARAM_REMOTE_COMMAND)
		{
			m_AppCommand = (AppCommand)lParam;
		}
		break;
	// this is for handling log-off messages (log-off, shut down, restart)
	case WM_ENDSESSION:
		m_MessagesManager.CloseMessages();
		m_Watchdog.DestroyWatchdog();
		m_AquaLangLib.CloseAquaLang();
		m_VersionUpdater.Close();
		m_AquaLangLib.Close();
		m_NotificationArea.Close();
		m_AppCommand = AppCommand_LogOff;
		break;
	case WM_TIMER:
		if(wParam == TIMER_ID_APP_WATCHDOG)
		{
			if(!m_Watchdog.KeepAlive())
			{
				Log(_T("AquaLangImpl::Message - watchdog found a problem, not sending keep alive to the watchdog\n"));
			}
		}
		break;
	}
}

void AquaLangImpl::InitializeNotificationArea(NotificationAreaHandler &NotificationAreaIcon, const AquaLangNotificationArea &NotificationAreaSettings, bool fCalculatorEnabled)
{
	if(NotificationAreaSettings.fShowIcon)
	{
		if(!NotificationAreaIcon.Open(fCalculatorEnabled))
		{
			Log(_T("AquaLangImpl::InitializeNotificationArea - failed opening notification area icon\n"));
		}
		else
		{
			if(NotificationAreaSettings.fShowTipOfTheDay)
			{
				if(!NotificationAreaIcon.OpenTipOfTheDay(m_UserName.c_str()))
				{
					Log(_T("AquaLangImpl::InitializeNotificationArea - failed opening tip of the day\n"));
				}
			}
			else
			{
				NotificationAreaIcon.CloseTipOfTheDay();
			}
		}
	}
	else
	{
		NotificationAreaIcon.Close();
	}
}

bool AquaLangImpl::BuildUserName()
{
	Log(_T("AquaLangImpl::BuildUserName:\n"));
	TCHAR UserName[MAX_PATH];
	_tcscpy_s(UserName, sizeof(UserName) / sizeof(UserName[0]), _T(""));
	DWORD Size = sizeof(UserName) / sizeof(UserName[0]);
	if(::GetUserName(UserName, &Size) && (_tcscmp(UserName, _T("SYSTEM")) != 0) && (_tcscmp(UserName, _T("Default User")) != 0))
	{
		Log(_T("AquaLangImpl::BuildUserName - user name is successfully rerieved by ::GetUserName (%s)\n"), UserName);
		m_UserName = UserName;
	}
	else
	{
		// build according to the environment variables - a little patchy, but could not find anything better
		// first get user application data
		TCHAR szPath[MAX_PATH];
		HRESULT hr = ::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath);
		if(FAILED(hr))
		{
			Log(_T("AquaLangImpl::BuildUserName - failed calling SHGetFolderPath, remaining with (%s)\n"), m_UserName.c_str());
			return false;
		}
		Log(_T("AquaLangImpl::BuildUserName - Application Data Folder is (%s)\n"), szPath);

		// remove '\'
		if(szPath[_tcslen(szPath) - 1] == _T('\\'))
		{
			szPath[_tcslen(szPath) - 1] = _T('\0');
		}

		// remove '\Application Data' in WinXP or \AppData\Roaming in Vista
		LPCTSTR SuffixToRemove = NULL;
		if(IsVistaOrHigher())
		{
			SuffixToRemove = _T("\\AppData\\Roaming");
		}
		else
		{
			SuffixToRemove = _T("\\Application Data");
		}
		if(_tcslen(szPath) < _tcslen(SuffixToRemove) || _tcscmp(szPath + _tcslen(szPath) - _tcslen(SuffixToRemove), SuffixToRemove) != 0)
		{
			Log(_T("AquaLangImpl::BuildUserName - did not find the string \'%s\' at the end of %s\n"), SuffixToRemove, szPath);
			return false;
		}
		szPath[_tcslen(szPath) - _tcslen(SuffixToRemove)] = _T('\0');

		// get the user name
		LPTSTR DetectedUserName = _tcsrchr(szPath, _T('\\'));
		if(!DetectedUserName)
		{
			Log(_T("AquaLangImpl::BuildUserName - failed extracting user name from %s\n"), szPath);
			return false;
		}
		m_UserName = DetectedUserName + 1;
		Log(_T("AquaLangImpl::BuildUserName - extracted user name is %s\n"), m_UserName.c_str());
	}
	return true;
}

void AquaLangImpl::GetMainMessageWindowTitle(_tstring &rTitle)
{
	rTitle = _T("AquaLang Main Message Window for ");
	rTitle += m_UserName;
}

bool AquaLangImpl::IsVistaOrHigher()
{
	OSVERSIONINFO VersionInfo;
	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);

	if(!::GetVersionEx(&VersionInfo))
		return false;
	if(VersionInfo.dwMajorVersion <= 5)
		return false;
	return true;
}

bool AquaLangImpl::RestoreUserNameAfterUpgrade()
{
	if(!UserNameIndicator::FindUserName(m_UserName))
		return false;
	if(m_UserName.length() == 0)
	{
		BuildUserName();
	}
	return true;
}
