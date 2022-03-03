#ifndef _CONFIG_ADVISOR_MANAGER_HEADER_
#define _CONFIG_ADVISOR_MANAGER_HEADER_

#include "ThreadInstance.h"
#include "SyncHelpers.h"

class ConfigAdvisorManager : public ThreadInstance
{
public:
	ConfigAdvisorManager();
	virtual ~ConfigAdvisorManager();

	bool Open(LPCTSTR ConfigWindowName, LPCTSTR UserName);
	void Close();
private:
	// ThreadInstance pure virtuals {
	virtual DWORD RunInThread();
	virtual void BreakThread();
	// }
	bool CheckIfNeedToAdvise() const;
private:
	_tstring m_ConfigWindowName;
	_tstring m_UserName;

	ThreadEvent m_hWaitEvent;
};

#endif // _CONFIG_ADVISOR_MANAGER_HEADER_