#include "Pch.h"
#include "ConfigurationManager.h"
#include "ConfigurationDialog.h"
#include "StringConvert.h"
#include "ComHelper.h"
#include "Log.h"

#define AQUALANG_STARTUP_NAME _T("AquaLang.lnk")

ConfigurationManager::ConfigurationManager(LPCTSTR UserName)
	: m_ConfigXml(UserName)
{
}

ConfigurationManager::~ConfigurationManager()
{
}

void ConfigurationManager::RunDialog(LPCTSTR WindowCaption, LPCTSTR UserName, bool &rfAlreadyRunning, bool &rfSettingsWereUpdated, bool &rfCheckForUpdates, AquaLangConfiguration &rUpdatedSettings)
{
	rfSettingsWereUpdated = false;
	rfCheckForUpdates = false;
	rfAlreadyRunning = false;

	AquaLangConfiguration CurrentConfig;
	__int64 CurrentFileVersion;
	bool fDebug;
	bool fDebugModeValid;
	m_ConfigXml.Load(CurrentConfig, CurrentFileVersion, fDebug, fDebugModeValid);

	rUpdatedSettings = CurrentConfig;
	const bool fCurrentlyAutoStart = IsInAutoStart();
	const bool fDefaultAutoStart = true;
	bool fAutoStart = fCurrentlyAutoStart;
	bool fDefaultsRestored = false;

	HWND hAlreadyRunningWnd = NULL;
	ConfigurationDialog Dialog;
	Dialog.Run(
			rUpdatedSettings,
			fAutoStart,
			fDefaultsRestored,
			rfCheckForUpdates,
			fDefaultAutoStart,
			WindowCaption,
			UserName,
			fDebug,
			hAlreadyRunningWnd
			);

	if(hAlreadyRunningWnd != NULL)
	{
		rfAlreadyRunning = true;
		::SetForegroundWindow(hAlreadyRunningWnd);
	}
	else
	{
		if(fAutoStart != fCurrentlyAutoStart)
		{
			SetAutoStart(fAutoStart);
		}
		
		if(ConfigurationDialog::ParamsChanged(rUpdatedSettings, CurrentConfig))
		{
			rfSettingsWereUpdated = true;
			m_ConfigXml.Store(rUpdatedSettings, CurrentFileVersion + 1, fDebug, fDebugModeValid, fDefaultsRestored);
		}
		else if(fDefaultsRestored)
		{
			m_ConfigXml.Store(rUpdatedSettings, CurrentFileVersion, fDebug, fDebugModeValid, fDefaultsRestored);
		}
		else
		{
			// this is done in purpose of marking that the user has entered the config dialog at least once
			bool fNewFile = false;
			RefreshFile(UserName, fNewFile);
		}
	}
}

bool ConfigurationManager::StopRunningDialog(LPCTSTR WindowCaption, LPCTSTR UserName)
{
	ConfigurationDialog Dialog;
	return Dialog.StopRunning(WindowCaption, UserName);
}

void ConfigurationManager::LoadSettings(AquaLangConfiguration &rConfig)
{
	__int64 CurrentFileVersion;
	bool fDebug;
	bool fDebugModeValid;
	m_ConfigXml.Load(rConfig, CurrentFileVersion, fDebug, fDebugModeValid);
}

void ConfigurationManager::LoadUpdatedSettings(AquaLangConfiguration &rConfig)
{
	AquaLangConfiguration CurrentConfig;
	__int64 CurrentFileVersion;
	bool fDebug;
	bool fDebugModeValid;
	m_ConfigXml.GetCurrentConfig(CurrentConfig, CurrentFileVersion, fDebug, fDebugModeValid);

	__int64 UpdatedFileVersion;
	m_ConfigXml.Load(rConfig, UpdatedFileVersion, fDebug, fDebugModeValid);
}

void ConfigurationManager::SendNotificationIfDialogOpened(LPCTSTR WindowCaption, LPCTSTR UserName, int NotificationType)
{
	ConfigurationDialog Dialog;
	Dialog.SendNotificationIfDialogOpened(WindowCaption, UserName, NotificationType);
}

void ConfigurationManager::RefreshFile(LPCTSTR UserName, bool &rfNew)
{
	rfNew = false;

	// if file exists, do nothing
	_tstring FileName;
	ConfigurationXml::GetFileName(UserName, FileName);
	if(CheckIfFileExists(FileName.c_str()))
		return;

	// read and then write settings
	rfNew = true;
	ConfigurationXml ConfigXml(UserName);

	AquaLangConfiguration Config;
	__int64 FileVersion;
	bool fDebug;
	bool fDebugModeValid;

	ConfigXml.Load(Config, FileVersion, fDebug, fDebugModeValid);
	// dont increase FileVersion!
	ConfigXml.Store(Config, FileVersion, fDebug, fDebugModeValid, false);
}

bool ConfigurationManager::IsInAutoStart() const
{
	TCHAR szPath[MAX_PATH];
	if(!GetAutoStartLink(szPath, sizeof(szPath) / sizeof(szPath[0])))
		return false;

	return CheckIfFileExists(szPath);
}

void ConfigurationManager::SetAutoStart(bool fAutoStart)
{
	TCHAR szPath[MAX_PATH];
	if(GetAutoStartLink(szPath, sizeof(szPath) / sizeof(szPath[0])))
	{
		// delete the old link anyway, as it may point to a different path
		::DeleteFile(szPath);

		if(fAutoStart)
		{
			TCHAR TargetPath[MAX_PATH];
			::GetModuleFileName (
				NULL,
				TargetPath,
				sizeof(TargetPath)/sizeof(TargetPath[0])
				);

			MakeShortcut(szPath, TargetPath);
		}
	}
}

bool ConfigurationManager::GetAutoStartLink(LPTSTR Path, DWORD dwSize) const
{
	if(dwSize < MAX_PATH)
		return false;
	HRESULT hr = SHGetFolderPath(NULL, /*CSIDL_COMMON_STARTUP*/CSIDL_STARTUP, NULL, 0, Path);
	//HRESULT hr = SHGetFolderPath(NULL, CSIDL_COMMON_STARTUP/*CSIDL_STARTUP*/, NULL, 0, Path);
	if(hr != S_OK)
		return false;
	_tcscat_s(Path, dwSize, _T("\\"));
	_tcscat_s(Path, dwSize, AQUALANG_STARTUP_NAME);
	return true;
}

bool ConfigurationManager::CheckIfFileExists(LPCTSTR File)
{
	WIN32_FIND_DATA FileData;
	HANDLE hFindFile = ::FindFirstFile(File, &FileData);
	bool fFound = false;
	if(hFindFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFindFile);
		fFound = true;
	}
	return fFound;
}

bool ConfigurationManager::MakeShortcut(LPCTSTR szPath, LPCTSTR TargetPath)
{
	ComInitializer ComInit;
	if(!ComInit.IsInitialized())
		return false;

	ReleasableItem<IShellLink> pShellLink;
	HRESULT hr = CoCreateInstance(
						CLSID_ShellLink,
						NULL,
						CLSCTX_INPROC_SERVER,
                        IID_IShellLink,
                        (LPVOID*)&pShellLink
						);
	if(FAILED(hr))
		return false;
	hr = pShellLink->SetPath(TargetPath);
	if(FAILED(hr))
		return false;
//	hr = pShellLink->SetIconLocation();

	ReleasableItem<IPersistFile> pPersistFile;
	hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
	if(FAILED(hr))
		return false;

	std::wstring wPath;
	SetString(wPath, szPath);

	hr = pPersistFile->Save(wPath.c_str(), TRUE);
	if(FAILED(hr))
		return false;
	return true;
}
