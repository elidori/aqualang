#ifndef _AQUALANG_IMPL_HEADER_
#define _AQUALANG_IMPL_HEADER_

#include "AquaLangLibLoader.h"
#include "AutoUpdate.h"
#include "MessagesManager.h"
#include "AquaLangRemoteCommand.h"
#include "NotificationAreaHandler.h"
#include "IConfigUpdateListener.h"
#include "UtilityWindow.h"
#include "AquaLangWatchdog.h"

class NotificationAreaHandler;
struct AquaLangNotificationArea;

class AquaLangImpl :
	public ITipListener,
	public IMenuEventHandler,
	public IConfigUpdateListener,
	public IWindowMessageListener
{
public:
	AquaLangImpl(LPCTSTR UserName);
	virtual ~AquaLangImpl();

	bool Run(bool fFirstRunAfterInstall, bool fStartUpRun, bool fRunningFromWatchdog);
	void Config();
	bool Stop(bool fIncludingWatchdog);

	bool Install();
	bool Upgrade();
	bool Uninstall();

	bool RunBrowser();

	bool TriggerCalculator();

	bool RunWatchdog();

	static bool IsVistaOrHigher();
private:
	// ITipListener pure virtuals {
	virtual void TipShouldClose(BalloonTip &rTip);
	virtual void TipWasClicked(BalloonTip &rTip, bool fLeftClick);
	// }

	// IMenuEventHandler pure virtuals {
	virtual void OptionSelected(int OptionId);
	// }

	// IConfigUpdateListener pure virtuals {
	virtual void ConfigurationChanged();
	// }

	// IWindowMessageListener pure virtuals {
	virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// }

	void InitializeNotificationArea(NotificationAreaHandler &NotificationAreaIcon, const AquaLangNotificationArea &NotificationAreaSettings, bool fCalculatorEnabled);

	bool BuildUserName();
	bool RestoreUserNameAfterUpgrade();

	void GetMainMessageWindowTitle(_tstring &rTitle);
private:
	UtilityWindow m_UtilityWindow;

	AquaLangRemoteCommand m_RemoteCommand;

	_tstring m_UserName;

	AquaLangLibLoader m_AquaLangLib;
	AutoUpdate m_VersionUpdater;

	NotificationAreaHandler m_NotificationArea;

	AquaLangWatchdogProcess m_WatchdogProcess;
	AquaLangWatchdogActivation m_Watchdog;

	AppCommand m_AppCommand;

	MessagesManager m_MessagesManager;
};

#endif // _AQUALANG_IMPL_HEADER_