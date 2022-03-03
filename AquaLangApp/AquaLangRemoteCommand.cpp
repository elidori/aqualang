#include "Pch.h"
#include "AquaLangRemoteCommand.h"
#include "UtilityWindow.h"
#include "UtilityWindowParams.h"
#include "Log.h"

#define COMMANDEVENTSUFFIX _T("_AquaLang_Process_Event")
#define COMMANDSHMEMSUFFIX _T("_AquaLang_Process_CommandBuffer")

AquaLangRemoteCommand::AquaLangRemoteCommand(UtilityWindow &rUtilityWindow)
	: m_rUtilityWindow(rUtilityWindow),
	m_CommandEvent(NULL),
	m_pCommandBuffer(NULL)
{
}

AquaLangRemoteCommand::~AquaLangRemoteCommand()
{
	Close();
}

bool AquaLangRemoteCommand::Create(LPCTSTR UserName)
{
	if(UserName == NULL)
		return false;
	_tstring UserNameStr = UserName;
	
	m_pCommandBuffer = (AppCommandSharedMemStruct *)m_CommandSharedMem.Create((UserNameStr + COMMANDSHMEMSUFFIX).c_str(), sizeof(AppCommandSharedMemStruct));
	if(!m_pCommandBuffer)
		return false;
	m_pCommandBuffer->Command = AppCommand_None;

	m_CommandEvent.UpdateName((UserNameStr + COMMANDEVENTSUFFIX).c_str());
	if(!m_CommandEvent.Create())
		return false;

	if(!OpenThread())
		return false;

	return true;
}

bool AquaLangRemoteCommand::Open(LPCTSTR UserName)
{
	if(UserName == NULL)
		return false;
	_tstring UserNameStr = UserName;
	m_CommandEvent.UpdateName((UserNameStr + COMMANDEVENTSUFFIX).c_str());
	if(!m_CommandEvent.Open())
		return false;

	m_pCommandBuffer = (AppCommandSharedMemStruct *)m_CommandSharedMem.Open((UserNameStr + COMMANDSHMEMSUFFIX).c_str());
	if(!m_pCommandBuffer)
	{
		Log(_T("AquaLangRemoteCommand::Open - an old version of this application is currently running\n"));
	}

	return true;
}

bool AquaLangRemoteCommand::SendCommand(AppCommand Command)
{
	if(!m_CommandEvent.IsOpen())
		return false;
	if(m_pCommandBuffer != NULL)
	{
		if(m_pCommandBuffer->Command != AppCommand_None)
		{
			Log(_T("AquaLangRemoteCommand::SendCommand - command is busy\n"));
			return false;
		}
		m_pCommandBuffer->Command = Command;
	}
	else
	{
		Log(_T("AquaLangRemoteCommand::SendCommand - working with an old version of the application\n"));
	}
	m_CommandEvent.Set();
	return true;
}

void AquaLangRemoteCommand::Close()
{
	CloseThread();
	m_CommandEvent.Destroy();
	m_pCommandBuffer = NULL;
	m_CommandSharedMem.Destroy();
}

DWORD AquaLangRemoteCommand::RunInThread()
{
	// the thread is opened only when the object is creating the event and shmem. Not when opening
	if(m_CommandEvent.IsOpen() && m_pCommandBuffer != NULL)
	{
		while(!IsBreaking())
		{
			m_CommandEvent.Wait();
			if(!IsBreaking())
			{
				AppCommand Command = (AppCommand)m_pCommandBuffer->Command;
				m_pCommandBuffer->Command = AppCommand_None;

				m_rUtilityWindow.UserMessage(WPARAM_REMOTE_COMMAND, (LPARAM)Command);
			}
		}
	}
	return 0;
}

void AquaLangRemoteCommand::BreakThread()
{
	m_CommandEvent.Set();
}
