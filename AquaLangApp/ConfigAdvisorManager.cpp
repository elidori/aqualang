#include "Pch.h"
#include "ConfigAdvisorManager.h"
#include "ConfigAdvisorDialog.h"
#include "ApplicationDataPath.h"
#include "AquaLangImpl.h"
#include "ConfigurationManager.h"
#include "Log.h"

#define ADVISOR_WAIT_MSEC (1000 * 3600 * 24)

ConfigAdvisorManager::ConfigAdvisorManager()
{
}

ConfigAdvisorManager::~ConfigAdvisorManager()
{
	Close();
}

bool ConfigAdvisorManager::Open(LPCTSTR ConfigWindowName, LPCTSTR UserName)
{
	CloseThread();

	assert(ConfigWindowName);
	assert(UserName);

	m_ConfigWindowName = ConfigWindowName;
	m_UserName = UserName;

	if(!OpenThread())
	{
		Log(_T("ConfigAdvisorManager::Open - Failed opening thread\n"));
		return false;
	}
	return true;
}

void ConfigAdvisorManager::Close()
{
	CloseThread();
}

DWORD ConfigAdvisorManager::RunInThread()
{
	m_hWaitEvent.Wait(ADVISOR_WAIT_MSEC);
	if(IsBreaking())
		return 0;

	if(CheckIfNeedToAdvise())
	{
		ConfigAdvisorDialog AdvisorDialog;
		bool fShouldRunConfig = false;
		AdvisorDialog.Run(m_UserName.c_str(), fShouldRunConfig);
		if(fShouldRunConfig)
		{
			ApplicationDataPath PathFinder(m_UserName.c_str());
			_tstring ApplicationName;
			PathFinder.GetApplicationName(ApplicationName);

			_tstring Args = _T("-config");
			Args += _T(" \"");
			Args += m_UserName;
			Args += _T("\"");

			Process::Create(ApplicationName.c_str(), Args.c_str(), _T("AquaLang Configuration"));
		}
	}
	return 0;
}

void ConfigAdvisorManager::BreakThread()
{
	m_hWaitEvent.Set();
}

bool ConfigAdvisorManager::CheckIfNeedToAdvise() const
{
	// if configuration file exist then no need to advise
	bool fNewFile = false;
	ConfigurationManager::RefreshFile(m_UserName.c_str(), fNewFile);
	if(!fNewFile)
		return false;

	// if config dialog is already open then no need to advise
	if(DialogObject::FindAquaLangWindow(m_ConfigWindowName.c_str(), m_UserName.c_str()) != NULL)
		return false;

	return true;
}