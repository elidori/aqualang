#ifndef _CONFIGURATION_DIALOG_HEADER_
#define _CONFIGURATION_DIALOG_HEADER_

#include "AquaLangConfiguration.h"
#include "TextDialog.h"
#include "DialogObject.h"
#include "DialogTab.h"

class ConfigPage;

class ConfigurationDialog : public DialogObject
{
public:
	ConfigurationDialog();
	virtual ~ConfigurationDialog();

	void Run(
		/*in, out*/AquaLangConfiguration &rConfig,
		/*in, out*/ bool &rfAutoStart,
		/*out*/ bool &rfDefaultsRestored,
		/*out*/ bool &rfCheckForUpdates,
		bool fDefaultAutoStartSetting,
		LPCTSTR Caption, 
		LPCTSTR UserName,
		bool fFullMode,
		HWND &rhAlreadyRunningWnd
		);
	bool StopRunning(
		LPCTSTR Caption, 
		LPCTSTR UserName
		);
	static bool ParamsChanged(const AquaLangConfiguration &rInitialConfig, const AquaLangConfiguration &rUpdatedConfig);

	void SendNotificationIfDialogOpened(LPCTSTR WindowCaption, LPCTSTR UserName, int NotificationType);
private:
	bool OpenTheDialog(LPCTSTR Caption, LPCTSTR UserName, bool fFullMode, AquaLangConfiguration &rConfig, bool &rfAutoStart);
	void CloseTheDialog();

	bool AddPage(ConfigPage *pPage, LPCTSTR Name);

	void LoadDialogFields();
	void StoreDialogFields();

	bool OpenAboutDialog();

	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault);

	bool GetLicenseTerms(_tstring &rLicense);

	bool CheckIfParamsChanged() const;
	bool CheckParamsValidity(_tstring &ErrorString);
	void ResetConfigValues(AquaLangConfiguration &rConfig, bool &rfAutoStart);

	void ConfigFileWasUpdate();
	void SyncWithConfigFileBeforeClose();
private:
	DialogTab m_DialogTab;
	std::vector<ConfigPage *> m_PageList;

	bool m_fExit;
	bool m_fSaveOnExit;
	bool m_fDefaultRestored;
	bool m_fCheckForUpdates;

	TextDialog m_AboutDialog;

	AquaLangConfiguration m_InitialConfig;
	bool m_fInitialAutoStartSetting;
	bool m_fDefaultAutoStartSetting;

	AquaLangConfiguration m_CurrentConfig;
	bool m_fCurrentAutoStartSetting;

	// for window search
	_tstring m_WindowCaption;
	_tstring m_UserName;
	HWND m_hRunningDialog;
};

#endif // _CONFIGURATION_DIALOG_HEADER_