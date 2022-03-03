#include "Pch.h"
#include "ThreadInstance.h"
#include "Log.h"

ThreadInstance::ThreadInstance()
	: m_hThread(NULL),
	m_fBreak(false)
{
}

ThreadInstance::~ThreadInstance()
{
	CloseThread();
}

bool ThreadInstance::OpenThread()
{
	CloseThread();

	DWORD dwThreadID;
	m_hThread = CreateThread(
					NULL,
					0x1000,
					(LPTHREAD_START_ROUTINE)ThreadRoutine,
					this,
					0,
					&dwThreadID
					);
	if(!m_hThread)
		return false;
	return true;
}

void ThreadInstance::CloseThread(DWORD dwTimeout /*= THREAD_BREAK_TIMEOUT*/)
{
	if(m_hThread != NULL)
	{
		m_fBreak = true;

		BreakThread();

		if(::WaitForSingleObject(m_hThread, dwTimeout) != WAIT_OBJECT_0)
		{
			Log(_T("ThreadInstance::CloseThread - Error: Thread could not be closed in time\n"));
			::TerminateThread(m_hThread, 0);
		}
		CloseHandle(m_hThread);
		m_hThread = NULL;
		m_fBreak = false;
	}
}

DWORD ThreadInstance::ThreadRoutine(LPVOID lpThreadParameter)
{
	ThreadInstance *This = (ThreadInstance *)lpThreadParameter;
	return This->RunThread();
}

DWORD ThreadInstance::RunThread()
{
	return RunInThread();
}
