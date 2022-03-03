#ifndef _USER_NAME_INDICATOR_HEADER_
#define _USER_NAME_INDICATOR_HEADER_

#include "ProcessUtils.h"
#include "DialogObject.h"

class UserNameIndicator : public DialogObject
{
public:
	UserNameIndicator();
	virtual ~UserNameIndicator();

	// these two methods are called by AquaLang application before calling msiexe for updating the software.
	bool Open(LPCTSTR UserName);
	bool WaitForUserNameQuery(DWORD dwTimeout);

	// these two methods are called by AquaLang new application instance in first run after install, for detching the user name and for releasing the old instance of the application
	static bool FindUserName(_tstring &rUserName);
	bool SignalForUserNameQueryComplete();
private:
	bool AcknowledgeIndicator(DWORD dwTimeout);
	void Close();
private:
	ProcessEvent m_UserNameQueryEvent;
};

#endif // _USER_NAME_INDICATOR_HEADER_