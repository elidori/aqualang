#ifndef _SYNC_HELPERS_HEADER_
#define _SYNC_HELPERS_HEADER_

class Scope
{
public:
	Scope(bool &rfInScope) : m_rfInScope(rfInScope)	{ m_rfInScope = true; }
	virtual ~Scope()		{ m_rfInScope = false; }
private:
	bool &m_rfInScope;
};

class CriticalSec
{
public:
	CriticalSec()			{ InitializeCriticalSection(&m_Lock); }
	virtual ~CriticalSec()	{ DeleteCriticalSection(&m_Lock); }

	void Lock()				{ EnterCriticalSection(&m_Lock); }
	void Unlock()			{ LeaveCriticalSection(&m_Lock); }
private:
	CRITICAL_SECTION m_Lock;
};

class LockScope
{
public:
	LockScope(CriticalSec &rCriticalSec) : m_rCriticalSec(rCriticalSec) { m_rCriticalSec.Lock(); }
	virtual ~LockScope()	{ m_rCriticalSec.Unlock(); }
private:
	CriticalSec &m_rCriticalSec;
};

class ThreadEvent
{
public:
	ThreadEvent()	{ 	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL); }
	virtual ~ThreadEvent()
	{
		if(m_hEvent)
		{
			CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}
	}

	void Set()					{ SetEvent(m_hEvent); }
	bool Wait(DWORD dwTimeout)	{ return ::WaitForSingleObject(m_hEvent, dwTimeout) == WAIT_OBJECT_0; }
private:
	HANDLE m_hEvent;
};

class Interlocked
{
public:
	Interlocked(long lValue) : m_lValue(lValue) {}
	virtual ~Interlocked()		{}

	long Set(long lValue)		{ return ::InterlockedExchange(&m_lValue, lValue); }
	long Get() const			{ return m_lValue; }
	long Add(long lDelta)		{ return ::InterlockedExchangeAdd(&m_lValue, lDelta); }
	long Subtract(long lDelta)	{ return ::InterlockedExchangeAdd(&m_lValue, -lDelta); }
	long Increment()			{ return ::InterlockedIncrement(&m_lValue); }
	long Decrement()			{ return ::InterlockedDecrement(&m_lValue); }
private:
	long m_lValue;
};

#endif // _SYNC_HELPERS_HEADER_