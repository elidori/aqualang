#include "Pch.h"
#include "ConfigurationDialog.h"
#include "resource.h"
#include "VersionInfo.h"
#include "StringConvert.h"
#include "ConfigurationDialogPages.h"
#include "HotKeyValidator.h"
#include "ConfigurationXml.h"
#include "ConfigurationMessages.h"
#include "AquaLangLibLoader.h"
#include "Log.h"

extern HMODULE g_hModule;

#define STOP_WAIT_TIMEOUT 500
#define STOP_RETRIES_COUNT 20

ConfigurationDialog::ConfigurationDialog()
	: DialogObject(IDI_ICON_DROP, false),
	m_AboutDialog(IDD_ABOUT_DIALOG, IDC_EDIT_LICENSE_TERMS),
	m_fExit(false),
	m_fSaveOnExit(false),
	m_fDefaultRestored(false),
	m_fCheckForUpdates(false)
{
}

ConfigurationDialog::~ConfigurationDialog()
{
}

void ConfigurationDialog::Run(
					/*in, out*/AquaLangConfiguration &rConfig,
					/*in, out*/ bool &rfAutoStart,
					/*out*/ bool &rfDefaultsRestored,
					/*out*/ bool &rfCheckForUpdates,
					bool fDefaultAutoStartSetting,
					LPCTSTR Caption, 
					LPCTSTR UserName,
					bool fFullMode,
					HWND &rhAlreadyRunningWnd
					)
{
	m_UserName = UserName;
	rfDefaultsRestored = false;
	rfCheckForUpdates = false;
	rhAlreadyRunningWnd = FindAquaLangWindow(Caption, UserName);
	if(rhAlreadyRunningWnd == NULL)
	{
		m_InitialConfig = rConfig;
		m_fInitialAutoStartSetting = rfAutoStart;
		m_CurrentConfig = rConfig;
		m_fCurrentAutoStartSetting = rfAutoStart;

		m_fDefaultAutoStartSetting = fDefaultAutoStartSetting;

		if(!OpenTheDialog(Caption, UserName, fFullMode, m_CurrentConfig, m_fCurrentAutoStartSetting))
			return;
		
		LoadDialogFields();

		m_fExit = false;
		m_fSaveOnExit = false;
		m_fCheckForUpdates = false;

		MSG msg;
		while(!m_fExit)
		{
			if(::GetMessage(&msg, NULL, NULL, NULL))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if(m_fSaveOnExit)
		{
			rConfig = m_CurrentConfig;
			rfAutoStart = m_fCurrentAutoStartSetting;
			rfDefaultsRestored = m_fDefaultRestored;
		}
		rfCheckForUpdates = m_fCheckForUpdates;

		CloseTheDialog();
	}
}

bool ConfigurationDialog::StopRunning(
								LPCTSTR Caption, 
								LPCTSTR UserName
								)
{
	HWND hAlreadyRunningWnd = FindAquaLangWindow(Caption, UserName);
	if(hAlreadyRunningWnd == NULL)
		return true;

	::PostMessage(hAlreadyRunningWnd, WM_AQUALANG_MSG, WPARAM_EXIT, 0);

	Sleep(STOP_WAIT_TIMEOUT);
	int Retries = STOP_RETRIES_COUNT;

	while(FindAquaLangWindow(Caption, UserName) != NULL)
	{
		if(Retries > 0)
		{
			Retries--;
			Sleep(STOP_WAIT_TIMEOUT);
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool ConfigurationDialog::ParamsChanged(const AquaLangConfiguration &rInitialConfig, const AquaLangConfiguration &rUpdatedConfig)
{
	if(rUpdatedConfig.BalloonTipSettings.MinTimeBetweenTwoSameTipsInMin != rInitialConfig.BalloonTipSettings.MinTimeBetweenTwoSameTipsInMin)
		return true;
	if(rUpdatedConfig.BalloonTipSettings.ApearanceDurationIn100msec != rInitialConfig.BalloonTipSettings.ApearanceDurationIn100msec)
		return true;
	if(rUpdatedConfig.BalloonTipSettings.fActive != rInitialConfig.BalloonTipSettings.fActive)
		return true;
	if(rUpdatedConfig.BalloonTipSettings.ShowMode != rInitialConfig.BalloonTipSettings.ShowMode)
		return true;
	if(rUpdatedConfig.TextConversionSettings.fActive != rInitialConfig.TextConversionSettings.fActive)
		return true;
	if(strcmp(rUpdatedConfig.TextConversionSettings.HotKey, rInitialConfig.TextConversionSettings.HotKey) != 0)
		return true;
	if(rUpdatedConfig.WebBrowseSettings.fActive != rInitialConfig.WebBrowseSettings.fActive)
		return true;
	if(rUpdatedConfig.WebBrowseSettings.nWebSearchOptionCode != rInitialConfig.WebBrowseSettings.nWebSearchOptionCode)
		return true;
	if(rUpdatedConfig.WebBrowseSettings.nWebTranslateOptionCode != rInitialConfig.WebBrowseSettings.nWebTranslateOptionCode)
		return true;
	if(strcmp(rUpdatedConfig.WebBrowseSettings.HotKey, rInitialConfig.WebBrowseSettings.HotKey) != 0)
		return true;
	if(rUpdatedConfig.CalcSettings.fActive != rInitialConfig.CalcSettings.fActive)
		return true;
	if(rUpdatedConfig.CalcSettings.ApearanceDurationIn100msec != rInitialConfig.CalcSettings.ApearanceDurationIn100msec)
		return true;
	if(rUpdatedConfig.CalcSettings.KbPauseBeforePopupIn100msec != rInitialConfig.CalcSettings.KbPauseBeforePopupIn100msec)
		return true;
	if(strcmp(rUpdatedConfig.CalcSettings.HotKey, rInitialConfig.CalcSettings.HotKey) != 0)
		return true;
	if(strcmp(rUpdatedConfig.CalcSettings.PopupHotKey, rInitialConfig.CalcSettings.PopupHotKey) != 0)
		return true;
	if(rUpdatedConfig.GuessLanguageSettings.fActive != rInitialConfig.GuessLanguageSettings.fActive)
		return true;
	if(rUpdatedConfig.GuessLanguageSettings.ApearanceDurationIn100msec != rInitialConfig.GuessLanguageSettings.ApearanceDurationIn100msec)
		return true;
	if(rUpdatedConfig.GuessLanguageSettings.KbPauseBeforePopupIn100msec != rInitialConfig.GuessLanguageSettings.KbPauseBeforePopupIn100msec)
		return true;
	if(rUpdatedConfig.GuessLanguageSettings.MinimalNumberOfCharacters != rInitialConfig.GuessLanguageSettings.MinimalNumberOfCharacters)
		return true;
	if(rUpdatedConfig.GuessLanguageSettings.MaximalNumberOfCharacters != rInitialConfig.GuessLanguageSettings.MaximalNumberOfCharacters)
		return true;

	if(rUpdatedConfig.SwapSettings.fActive != rInitialConfig.SwapSettings.fActive)
		return true;
	if(strcmp(rUpdatedConfig.SwapSettings.HotKey, rInitialConfig.SwapSettings.HotKey) != 0)
		return true;
	if(rUpdatedConfig.UserInfoSettings.NotificationIntervalMinutes != rInitialConfig.UserInfoSettings.NotificationIntervalMinutes)
		return true;
	if(strcmp(rUpdatedConfig.UserInfoSettings.ServerUrl, rInitialConfig.UserInfoSettings.ServerUrl) != 0)
		return true;
	if(strcmp(rUpdatedConfig.VersionUpdateSettings.ServerUrl, rInitialConfig.VersionUpdateSettings.ServerUrl) != 0)
		return true;
	if(rUpdatedConfig.VersionUpdateSettings.UpdateCheckIntervalMinutes != rInitialConfig.VersionUpdateSettings.UpdateCheckIntervalMinutes)
		return true;
	if(rUpdatedConfig.VersionUpdateSettings.fCheckForBetaVersion != rInitialConfig.VersionUpdateSettings.fCheckForBetaVersion)
		return true;
	if(rUpdatedConfig.NotificationArea.fShowIcon != rInitialConfig.NotificationArea.fShowIcon)
		return true;
	if(rUpdatedConfig.NotificationArea.fShowTipOfTheDay != rInitialConfig.NotificationArea.fShowTipOfTheDay)
		return true;
	return false;
}

void ConfigurationDialog::SendNotificationIfDialogOpened(LPCTSTR WindowCaption, LPCTSTR UserName, int NotificationType)
{
	HWND hAlreadyRunningWnd = FindAquaLangWindow(WindowCaption, UserName);
	if(hAlreadyRunningWnd != NULL)
	{
		::PostMessage(hAlreadyRunningWnd, WM_AQUALANG_MSG, (WPARAM)NotificationType, 0);
	}
}

bool ConfigurationDialog::OpenTheDialog(LPCTSTR Caption, LPCTSTR UserName, bool fFullMode, AquaLangConfiguration &rConfig, bool &rfAutoStart)
{
	HWND hDialog = OpenDialog(UserName, IDD_DIALOG_AQUALANG_CONFIG);
	if(hDialog == NULL)
		return false;

	SetTitle(Caption);

	// Add property pages
	HWND hPageWnd = ::GetDlgItem(hDialog, IDC_TAB_PROPERTIES);
	m_DialogTab.Open(hPageWnd);

	ConfigPage *pPage = NULL;
	bool fSuccess = true;
	
	pPage = WaterDropConfigPage::Create(hPageWnd, rConfig.BalloonTipSettings);
	fSuccess &= AddPage(pPage, _T("Water drops"));
	HWND hWaterDropDlg = pPage ? pPage->GetDialogHandle() : NULL;

	pPage = HotKeyConfigPage::Create(hPageWnd, rConfig);
	fSuccess &= AddPage(pPage, _T("Hot Keys"));

	pPage = CalculatorConfigPage::Create(hPageWnd, rConfig.CalcSettings);
	fSuccess &= AddPage(pPage, _T("Calc"));

	pPage = GuessLanguageConfigPage::Create(hPageWnd, rConfig.GuessLanguageSettings);
	fSuccess &= AddPage(pPage, _T("Guess Lang"));

	pPage = VersionConfigPage::Create(hPageWnd, rConfig.VersionUpdateSettings, hDialog);
	fSuccess &= AddPage(pPage, _T("Version"));

	pPage = SystemPage::Create(hPageWnd, rConfig.NotificationArea, rfAutoStart);
	fSuccess &= AddPage(pPage, _T("System"));

	if(fFullMode)
	{
		pPage = DebugConfigPage::Create(hPageWnd, rConfig);
		fSuccess &= AddPage(pPage, _T("Debug mode"));
	}
	else
	{
		::ShowWindow(::GetDlgItem(hWaterDropDlg, IDC_RADIO_BALLOON_DISPLAY_DEBUG), SW_HIDE);
	}

	// show dialog
	::ShowWindow(hDialog, SW_SHOW);
	return true;
}

void ConfigurationDialog::CloseTheDialog()
{
	m_AboutDialog.Close();

	CloseDialog();

	m_DialogTab.Close();
	for(unsigned int i = 0; i < m_PageList.size(); i++)
	{
		m_PageList.at(i)->Release();
	}
	m_PageList.erase(m_PageList.begin(), m_PageList.end());
}

bool ConfigurationDialog::AddPage(ConfigPage *pPage, LPCTSTR Name)
{
	if(!m_DialogTab.AddPage(pPage, Name))
		return false;

	m_PageList.push_back(pPage);
	return true;
}

void ConfigurationDialog::LoadDialogFields()
{
	for(unsigned int i = 0; i < m_PageList.size(); i++)
	{
		m_PageList.at(i)->Load();
	}
}

void ConfigurationDialog::StoreDialogFields()
{
	for(unsigned int i = 0; i < m_PageList.size(); i++)
	{
		m_PageList.at(i)->Store();
	}
}

bool ConfigurationDialog::OpenAboutDialog()
{
	_tstring LicenseText;
	GetLicenseTerms(LicenseText);

	HWND hDialog = GetDialogHandle();
	assert(hDialog);
	if(!m_AboutDialog.Open(m_UserName.c_str(), LicenseText.c_str(), hDialog))
		return false;

	_tstring WindowText = _T("AquaLang version ");
	WindowText += _T(VERSION_STRING);
	::SetWindowText(m_AboutDialog.GetItem(IDC_STATIC_VERSION), WindowText.c_str());

	return true;
}

LRESULT ConfigurationDialog::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault)
{
	fSkipDefault = false;
	LRESULT lResult = 0;

	switch(uMsg)
	{
	case WM_NOTIFY:
		// Tab change
		if(wParam == IDC_TAB_PROPERTIES)
		{
			m_DialogTab.OnNotifySelectChange(*(NMHDR *)lParam);
		}
		break;
	case WM_COMMAND:
		// On Cancel
		if((HWND)lParam == ::GetDlgItem(hwnd, IDCANCEL))
		{
			m_fSaveOnExit = false;
			m_fExit = true;
		}
		// On Ok
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDOK))
		{
			StoreDialogFields();
			_tstring ErrorMessage;
			if(CheckParamsValidity(ErrorMessage))
			{
				if(CheckIfParamsChanged() || m_fDefaultRestored)
				{
					SyncWithConfigFileBeforeClose();
					m_fSaveOnExit = true;
				}
				m_fExit = true;
			}
			else
			{
				::MessageBox(hwnd, ErrorMessage.c_str(), _T("AquaLang Settings Error"), MB_OK | MB_ICONEXCLAMATION);
			}
		}
		// restore defaults
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_RESTORE_DEFAULTS))
		{
			m_fDefaultRestored = true;
			ResetConfigValues(m_CurrentConfig, m_fCurrentAutoStartSetting);
		}
		break;
	case WM_AQUALANG_MSG:
		switch(wParam)
		{
		case WPARAM_EXIT:
			if(!m_fExit)
			{
				::MessageBox(hwnd, _T("Please close dialog, to allow the upgrade to continue"), _T("Exit AquaLang configuration"), MB_ICONASTERISK | MB_OK);
			}
			break;
		case WPARAM_ABOUT:
			OpenAboutDialog();
			break;
		case WPARAM_VERSION_UPDATE:
			StoreDialogFields();
			{
				bool fContinueUpdate = true;

				if(CheckIfParamsChanged() || m_fDefaultRestored)
				{
					int Ret = ::MessageBox(hwnd, _T("Settings were modified. Save new settings before continuing?"), _T("AquaLang Update check"), MB_YESNOCANCEL | MB_ICONQUESTION);
					switch(Ret)
					{
					case IDCANCEL:
						fContinueUpdate = false;
						break;
					case IDYES:
						{
							_tstring ErrorMessage;
							if(CheckParamsValidity(ErrorMessage))
							{
								SyncWithConfigFileBeforeClose();
								m_fSaveOnExit = true;
							}
							else
							{
								::MessageBox(hwnd, ErrorMessage.c_str(), _T("AquaLang Settings Error"), MB_OK | MB_ICONEXCLAMATION);
								fContinueUpdate = false;
							}
						}
						break;
					case IDNO:
						break;
					}
				}

				if(fContinueUpdate)
				{
					::ShowWindow(hwnd, SW_HIDE);
					m_AboutDialog.Close();
					m_fCheckForUpdates = true;
					m_fExit = true;
				}
			}
			break;
		case WPARARM_CONFIG_EXTERNALLY_CHANGED:
			ConfigFileWasUpdate();
			break;
		}
		break;
	// this is for handling log-off messages (log-off, shut down, restart)
	case WM_ENDSESSION:
		m_fSaveOnExit = false;
		m_fExit = true;
		break;
	}

	return lResult;
}

bool ConfigurationDialog::GetLicenseTerms(_tstring &rLicense)
{
	HRSRC hrsRC = ::FindResource (g_hModule, MAKEINTRESOURCE(IDB_FILE_LICENSE_TERMS), _T("BIN"));
	if(!hrsRC)
		return false;
	DWORD dwResourceSize = SizeofResource (g_hModule, hrsRC);
	HGLOBAL hglbRC = LoadResource (g_hModule, hrsRC);
	if(!hglbRC)
		return false;
	LPVOID lpRsc = LockResource(hglbRC);
	if(!lpRsc)
		return false;

	SetString(rLicense, (LPCSTR)lpRsc, dwResourceSize);
	return true;
}

bool ConfigurationDialog::CheckIfParamsChanged() const
{
	if(ParamsChanged(m_InitialConfig, m_CurrentConfig))
		return true;

	if(m_fInitialAutoStartSetting != m_fCurrentAutoStartSetting)
		return true;
	return false;
}

bool ConfigurationDialog::CheckParamsValidity(_tstring &ErrorString)
{
	AquaLangLibLoader AquaLangLib;
	if(!AquaLangLib.Open())
	{
		Log(_T("ConfigurationDialog::CheckParamsValidity - failed loading aqualang library"));
		return false;
	}

	HotKeyValidator Validator(AquaLangLib);
	if(!Validator.AddHotKey(m_CurrentConfig.TextConversionSettings.HotKey, m_CurrentConfig.TextConversionSettings.fActive, _T("Text conversion"), ErrorString))
		return false;
	if(!Validator.AddHotKey(m_CurrentConfig.WebBrowseSettings.HotKey, m_CurrentConfig.WebBrowseSettings.fActive, _T("Web Browse"), ErrorString))
		return false;
	if(!Validator.AddHotKey(m_CurrentConfig.CalcSettings.HotKey, m_CurrentConfig.CalcSettings.fActive, _T("Calculator"), ErrorString))
		return false;
	if(!Validator.AddHotKey(m_CurrentConfig.CalcSettings.PopupHotKey, m_CurrentConfig.CalcSettings.fActive, _T("Calculator Popup"), ErrorString))
		return false;
	if(!Validator.AddHotKey(m_CurrentConfig.SwapSettings.HotKey, m_CurrentConfig.SwapSettings.fActive, _T("Clipboard Swap"), ErrorString))
		return false;

	Validator.Close();

	return true;
}

void ConfigurationDialog::ResetConfigValues(AquaLangConfiguration &rConfig, bool &rfAutoStart)
{
	ConfigurationXml ConfigXml(NULL);

	__int64 FileVersion;
	bool fDebug;
	bool fDebugModeValid;
	ConfigXml.GetCurrentConfig(rConfig, FileVersion, fDebug, fDebugModeValid);

	rfAutoStart = m_fDefaultAutoStartSetting;
	LoadDialogFields();
}

void ConfigurationDialog::ConfigFileWasUpdate()
{
	// Eli Dori: To Do - the file content has changed.  
}

void ConfigurationDialog::SyncWithConfigFileBeforeClose()
{
	// Eli Dori: To Do - the file content has changed.  
}
