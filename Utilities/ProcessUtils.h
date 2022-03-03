#ifndef _PROCESS_UTILS_HEADER_
#define _PROCESS_UTILS_HEADER_

class Process
{
public:
	static bool Create(LPCTSTR ProcessFullPathName, LPCTSTR CommandSwitches, LPCTSTR DebugString, HANDLE *phProcess = NULL);
};

class ProcessEvent
{
public:
	ProcessEvent(LPCTSTR Name);
	virtual ~ProcessEvent();

	bool UpdateName(LPCTSTR Name);

	bool Create();
	bool Open();
	bool IsOpen() const;
	void Set();
	void Reset();
	bool Wait(DWORD dwTimeout = INFINITE);
	void Destroy();
private:
	HANDLE m_hEvent;
	TCHAR m_Name[256];
};

class ProcessSharedMem
{
public:
	ProcessSharedMem();
	virtual ~ProcessSharedMem();

	LPBYTE Create(LPCTSTR Name, int Size);
	LPBYTE Open(LPCTSTR Name);

	void Destroy();
private:
	HANDLE m_hSharedMem;
	LPBYTE m_pMemory;
};

#endif // _PROCESS_UTILS_HEADER_