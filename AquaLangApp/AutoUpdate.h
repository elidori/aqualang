#ifndef _AUTO_UPDATE_HEADER_
#define _AUTO_UPDATE_HEADER_

#include "ThreadInstance.h"
#include "SyncHelpers.h"
#include "AutoUpdateConfig.h"
#include "UpgraderDialog.h"

struct AquaLangUpdate;

class AutoUpdate : public ThreadInstance
{
public:
	AutoUpdate();
	virtual ~AutoUpdate();

	bool Open(LPCTSTR UserName, const AquaLangUpdate &rVersionUpdateSettings);
	void Close();
private:
	// ThreadInstance pure virtuals {
	virtual DWORD RunInThread();
	virtual void BreakThread();
	// }
	void UpdateNextTimeToNotify();
	bool CheckIfNeedToNotify() const;
private:
	_tstring m_UserName;
	std::string m_DownloadServerUrl;
	int m_nUpdateCheckIntervalMinutes;
	bool m_fCheckForBetaVersion;

	ThreadEvent m_hWaitEvent;

	AutoUpdateConfig m_Configuration;

	UpgraderDialog m_UpgraderDlg;
};

#endif // _AUTO_UPDATE_HEADER_