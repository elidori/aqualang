#ifndef _USER_INFORMATION_NOTIFIER_HEADER_
#define _USER_INFORMATION_NOTIFIER_HEADER_

#include "UserInfoConfiguration.h"
#include "WinHookResult.h"
#include "ThreadInstance.h"
#include "SyncHelpers.h"
#include "WebMessageSender.h"

using namespace WinHook;
struct AquaLangUserInfoNotification;
struct AquaLangStatistics;

class UserInformationNotifier : public ThreadInstance
{
public:
	UserInformationNotifier();
	virtual ~UserInformationNotifier();

	Result Open(LPCTSTR UserName, const AquaLangUserInfoNotification &rConfig);
	void Close();

	void IncreaseNumberOfFocusEvents();
	void IncreaseNumberOfLanguageChangeEvents();
	void IncreaseLanguageManualChangeEvents();
	void IncreaseLanguageManualFixEvents();
	void IncreaseTextLanguageChangeEvents();
	void IncreaseWebSearchEvents();
	void IncreaseCalcRequestEvents();
	void NotifyWatchdogRecovery();
	void IncreaseGuessLanguageAdviseEvents();
	void IncreaseGuessLanguageAcceptEvents();

	bool FillStatistics(AquaLangStatistics &rStatistics);
private:
	bool BuildUserString(LPCTSTR UserName);

	void UpdateNextTimeToNotify();
	bool CheckIfNeedToNotify() const;

	bool SendNotification(LPCTSTR lpFormat, ...);

	// ThreadInstance pure virtuals {
	virtual DWORD RunInThread();
	virtual void BreakThread();
	// }
private:
	WebMessageSender m_WebSender;

	__int64 m_nNotificationIntervalMinutes;

	UserInfoConfiguration m_Configuration;

	std::string m_ListenerUrl;
	_tstring m_UserString;

	ThreadEvent m_hWaitEvent;
};

#endif // _USER_INFORMATION_NOTIFIER_HEADER_