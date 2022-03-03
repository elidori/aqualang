#ifndef _CONFIGURATION_MANAGER_HEADER_
#define _CONFIGURATION_MANAGER_HEADER_

#include "ConfigurationXml.h"

struct AquaLangConfiguration;
struct AquaLangUpdate;

class ConfigurationManager
{
public:
	ConfigurationManager(LPCTSTR UserName);
	virtual ~ConfigurationManager();

	void RunDialog(LPCTSTR WindowCaption, LPCTSTR UserName, bool &rfAlreadyRunning, bool &rfSettingsWereUpdated, bool &rfCheckForUpdates, AquaLangConfiguration &rUpdatedSettings);
	bool StopRunningDialog(LPCTSTR WindowCaption, LPCTSTR UserName); // by any process
	void LoadSettings(AquaLangConfiguration &rConfig);
	void LoadUpdatedSettings(AquaLangConfiguration &rConfig);

	static void SendNotificationIfDialogOpened(LPCTSTR WindowCaption, LPCTSTR UserName, int NotificationType);

	static void RefreshFile(LPCTSTR UserName, bool &rfNew);
private:
	bool IsInAutoStart() const;
	void SetAutoStart(bool fAutoStart);
	bool GetAutoStartLink(LPTSTR Path, DWORD dwSize) const;
	static bool CheckIfFileExists(LPCTSTR File);
	bool MakeShortcut(LPCTSTR szPath, LPCTSTR TargetPath);
	bool ReplaceString(LPBYTE Buffer, DWORD dwSize, LPCTSTR SourceString, LPCTSTR TargetString) const;
private:
	ConfigurationXml m_ConfigXml;
};

#endif // _CONFIGURATION_MANAGER_HEADER_