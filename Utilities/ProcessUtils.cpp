#include "Pch.h"
#include "ProcessUtils.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Process
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Process::Create(LPCTSTR ProcessFullPathName, LPCTSTR CommandSwitches, LPCTSTR DebugString, HANDLE *phProcess /*= NULL*/)
{
	if(phProcess)
	{
		*phProcess = NULL;
	}

	LPTSTR CommandSwitchesPtr = NULL;
	TCHAR CommandSwitchesStr[1024];

	if(CommandSwitches != NULL && _tcslen(CommandSwitches) > 0)
	{
		_stprintf_s(
				CommandSwitchesStr, sizeof(CommandSwitchesStr) / sizeof(CommandSwitchesStr[0]),
				_T("\"%s\" %s"),
				ProcessFullPathName,
				CommandSwitches
				);
		CommandSwitchesPtr = CommandSwitchesStr;
	}

	STARTUPINFO StartupInfo;
	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);

	PROCESS_INFORMATION ProcessInfo;

	// run application after installlation
	if(!::CreateProcess(
				ProcessFullPathName,
				CommandSwitchesPtr,
				NULL,
				NULL,
				FALSE,
				0,
				NULL,
				NULL,
				&StartupInfo,
				&ProcessInfo
				))
	{
#ifdef _DEBUG
		MessageBox(NULL, _T("Failed running"), DebugString, MB_ICONASTERISK | MB_OK);
#endif
		return false;
	}
	if(ProcessInfo.hProcess != NULL)
	{
		if(phProcess)
		{
			*phProcess = ProcessInfo.hProcess;
		}
		else
		{
			CloseHandle(ProcessInfo.hProcess);
		}
	}
	if(ProcessInfo.hThread != NULL)
	{
		CloseHandle(ProcessInfo.hThread);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ProcessEvent
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ProcessEvent::ProcessEvent(LPCTSTR Name)
	: m_hEvent(NULL)
{
	UpdateName(Name);
}

ProcessEvent::~ProcessEvent()
{
	Destroy();
}

bool ProcessEvent::UpdateName(LPCTSTR Name)
{
	if(m_hEvent != NULL)
		return false;
	if(Name != NULL)
	{
		assert(_tcslen(Name) < sizeof(m_Name)/sizeof(m_Name[0]));
		_tcscpy_s(m_Name, sizeof(m_Name)/sizeof(m_Name[0]), Name);
	}
	else
	{
		_tcscpy_s(m_Name, _T(""));
	}
	return true;
}

bool ProcessEvent::Create()
{
	if(m_hEvent != NULL)
		return false;

	SECURITY_ATTRIBUTES SecAttr;
	SecAttr.nLength = sizeof(SecAttr);
	SecAttr.bInheritHandle = FALSE;
	TCHAR *szSD =	TEXT("D:")				 // Discretionary ACL
					TEXT("(A;OICI;GA;;;BU)")  // Allow all access to built in users
					TEXT("(A;OICI;GA;;;SY)")  // Allow all access to built in local system
					TEXT("(A;OICI;GA;;;BA)"); // Allow all access to built in administrator
 
	//	SecAttr.lpSecurityDescriptor = SDDL_NULL_ACL;
	if(!::ConvertStringSecurityDescriptorToSecurityDescriptor(
				szSD,
                SDDL_REVISION_1,
                &(SecAttr.lpSecurityDescriptor),
                NULL))
	{
		return false;
	}

	m_hEvent = ::CreateEvent(&SecAttr, FALSE, FALSE, m_Name);
	if(m_hEvent == NULL)
		return false;
	return true;
}
bool ProcessEvent::Open()
{
	if(m_hEvent != NULL)
		return false;
	m_hEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, m_Name);
	if(m_hEvent == NULL)
		return false;
	return true;
}

bool ProcessEvent::IsOpen() const
{
	return (m_hEvent != NULL);
}

void ProcessEvent::Set()
{
	if(m_hEvent != NULL)
	{
		::SetEvent(m_hEvent);
	}
}

void ProcessEvent::Reset()
{
	if(m_hEvent != NULL)
	{
		::ResetEvent(m_hEvent);
	}
}

bool ProcessEvent::Wait(DWORD dwTimeout /*= INFINITE*/)
{
	if(m_hEvent == NULL)
		return false;
	return ::WaitForSingleObject(m_hEvent, dwTimeout) == WAIT_OBJECT_0;
}

void ProcessEvent::Destroy()
{
	if(m_hEvent)
	{
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ProcessSharedMem
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ProcessSharedMem::ProcessSharedMem()
	: m_hSharedMem(NULL),
	m_pMemory(NULL)
{
}

ProcessSharedMem::~ProcessSharedMem()
{
	Destroy();
}

LPBYTE ProcessSharedMem::Create(LPCTSTR Name, int Size)
{
	if(m_hSharedMem != NULL)
		return NULL;

	SECURITY_ATTRIBUTES SecAttr;
	SecAttr.nLength = sizeof(SecAttr);
	SecAttr.bInheritHandle = FALSE;
	TCHAR *szSD =	TEXT("D:")				 // Discretionary ACL
					TEXT("(A;OICI;GA;;;BU)")  // Allow all access to built in users
					TEXT("(A;OICI;GA;;;SY)")  // Allow all access to built in local system
					TEXT("(A;OICI;GA;;;BA)"); // Allow all access to built in administrator
 
	//	SecAttr.lpSecurityDescriptor = SDDL_NULL_ACL;
	if(!::ConvertStringSecurityDescriptorToSecurityDescriptor(
				szSD,
                SDDL_REVISION_1,
                &(SecAttr.lpSecurityDescriptor),
                NULL))
	{
		return NULL;
	}

	m_hSharedMem = ::CreateFileMapping(
						INVALID_HANDLE_VALUE,
						&SecAttr,
						PAGE_READWRITE,
						0,
						Size,
						Name
						);

	if(m_hSharedMem == NULL)
		return NULL;

	m_pMemory = (LPBYTE)::MapViewOfFile(
						m_hSharedMem,
						FILE_MAP_ALL_ACCESS,
						(DWORD) 0,
						(DWORD) 0,
						(DWORD) 0
						);

	if(!m_pMemory)
	{
		Destroy();
	}

	memset(m_pMemory, 0, Size);
	return m_pMemory;
}

LPBYTE ProcessSharedMem::Open(LPCTSTR Name)
{
	if(m_hSharedMem != NULL)
		return NULL;

	m_hSharedMem = ::OpenFileMapping(
					FILE_MAP_ALL_ACCESS,
					FALSE,
					Name
					);
	if(m_hSharedMem == NULL)
		return NULL;

	m_pMemory = (LPBYTE)::MapViewOfFile(
					m_hSharedMem,
					FILE_MAP_ALL_ACCESS,
					(DWORD) 0,
					(DWORD) 0,
					(DWORD) 0
					);

	if(!m_pMemory)
	{
		Destroy();
	}
	return m_pMemory;
}

void ProcessSharedMem::Destroy()
{
	if(m_pMemory != NULL)
	{
		::UnmapViewOfFile((void *)m_pMemory);
		m_pMemory = NULL;
	}
	if(m_hSharedMem != NULL)
	{
		::CloseHandle(m_hSharedMem);
		m_hSharedMem = NULL;
	}
}
