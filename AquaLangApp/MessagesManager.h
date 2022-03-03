#ifndef _MESSAGES_MANAGER_HEADER_
#define _MESSAGES_MANAGER_HEADER_

#include "BalloonTip.h"

enum
{
	AquaLangRunning = 0,
	AquaLangStopped = 1,
	AquaLangAlreadyRunning = 2,
	AquaLangRunningError = 3
};

class MessagesManager
{
public:
	MessagesManager();
	virtual ~MessagesManager();

	void SetUserName(LPCTSTR UserName);

	bool PresentMessage(int MessageId, ITipListener *pListener = NULL);
	void CloseMessages();
private:
	_tstring m_UserName;
	typedef std::map<int, BalloonTip *> MessageMap;
	MessageMap m_MessageMap;

	BalloonTip m_AppStartedMessage;
	BalloonTip m_AppEndedMessage;
	BalloonTip m_AppAlreadyRunnningMessage;
	BalloonTip m_AppRunningError;
};

#endif // _MESSAGES_MANAGER_HEADER_