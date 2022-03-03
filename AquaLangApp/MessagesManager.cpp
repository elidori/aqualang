#include "Pch.h"
#include "MessagesManager.h"
#include "resource.h"

#define TIP_FADE_OUT_DURATION_MSEC 500
#define TIP_FADE_OUT_STEPS 20

MessagesManager::MessagesManager()
	: m_AppStartedMessage(
			TIP_FADE_OUT_DURATION_MSEC,
			TIP_FADE_OUT_STEPS,
			IDB_BITMAP_APP_RUNNING,
			IDB_BITMAP_APP_RUNNING,
			IDB_BITMAP_APP_RUNNING,
			IDB_BITMAP_APP_RUNNING,
			RGB(0, 0, 255)
			),
	m_AppEndedMessage(
			TIP_FADE_OUT_DURATION_MSEC,
			TIP_FADE_OUT_STEPS,
			IDB_BITMAP_APP_STOPPED,
			IDB_BITMAP_APP_STOPPED,
			IDB_BITMAP_APP_STOPPED,
			IDB_BITMAP_APP_STOPPED,
			RGB(0, 0, 255)
			),
	m_AppAlreadyRunnningMessage(
			TIP_FADE_OUT_DURATION_MSEC,
			TIP_FADE_OUT_STEPS,
			IDB_BITMAP_APP_ALREADY_RUNNING,
			IDB_BITMAP_APP_ALREADY_RUNNING,
			IDB_BITMAP_APP_ALREADY_RUNNING,
			IDB_BITMAP_APP_ALREADY_RUNNING,
			RGB(146, 208, 80)
			),
	m_AppRunningError(
			TIP_FADE_OUT_DURATION_MSEC,
			TIP_FADE_OUT_STEPS,
			IDB_BITMAP_APP_START_ERROR,
			IDB_BITMAP_APP_START_ERROR,
			IDB_BITMAP_APP_START_ERROR,
			IDB_BITMAP_APP_START_ERROR,
			RGB(149, 55, 53)
			)
{
	m_MessageMap.insert(MessageMap::value_type(AquaLangRunning, &m_AppStartedMessage));
	m_MessageMap.insert(MessageMap::value_type(AquaLangStopped, &m_AppEndedMessage));
	m_MessageMap.insert(MessageMap::value_type(AquaLangAlreadyRunning, &m_AppAlreadyRunnningMessage));
	m_MessageMap.insert(MessageMap::value_type(AquaLangRunningError, &m_AppRunningError));
}

void MessagesManager::SetUserName(LPCTSTR UserName)
{
	m_UserName = UserName;
}

MessagesManager::~MessagesManager()
{
	CloseMessages();
}

bool MessagesManager::PresentMessage(int MessageId, ITipListener *pListener /*= NULL*/)
{
	MessageMap::iterator Iterator = m_MessageMap.find(MessageId);
	if(Iterator == m_MessageMap.end())
		return false;
	return (*Iterator).second->Init(m_UserName.c_str(), _T("AquaLang message drop"), NULL, NULL, IDD_BALLOONTIPWINDOW_DIALOG, 200, 2000, false, pListener);
}

void MessagesManager::CloseMessages()
{
	MessageMap::iterator Iterator = m_MessageMap.begin();
	for(; Iterator != m_MessageMap.end(); Iterator++)
	{
		(*Iterator).second->Term();
	}
}
