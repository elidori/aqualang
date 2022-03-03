#ifndef _THREAD_INSTANCE_HEADER_
#define _THREAD_INSTANCE_HEADER_

#define THREAD_BREAK_TIMEOUT 3000

class ThreadInstance
{
public:
	ThreadInstance();
	virtual ~ThreadInstance();

	bool OpenThread();
	void CloseThread(DWORD dwTimeout = THREAD_BREAK_TIMEOUT);
protected:
	virtual DWORD RunInThread() = 0;
	virtual void BreakThread() = 0;

	bool IsThreadOpen() const { return m_hThread != NULL; }
	bool IsBreaking() const	{ return m_fBreak; }
private:
	static DWORD ThreadRoutine(LPVOID lpThreadParameter);
	DWORD RunThread();
private:
	HANDLE m_hThread;
	bool m_fBreak;
};

#endif // _THREAD_INSTANCE_HEADER_