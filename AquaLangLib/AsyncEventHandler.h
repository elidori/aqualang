#ifndef _ASYNC_EVENT_HANDLER_HEADER_
#define _ASYNC_EVENT_HANDLER_HEADER_

#include "ThreadInstance.h"
#include "UtilityWindow.h"
#include "SyncHelpers.h"

class IAsyncEventHandlerTarget
{
protected:
	virtual ~IAsyncEventHandlerTarget() {}
public:
	virtual void HandleAsynchronous() = 0;
	virtual void HandleCritical() = 0;
};

class AsyncEventHandler :
	public ThreadInstance,
	public IWindowMessageListener
{
public:
	AsyncEventHandler(IAsyncEventHandlerTarget &rTarget);
	virtual ~AsyncEventHandler();

	UtilityWindow &GetUtilityWindow()	{ return m_UtilityWindow; }

	bool OpenEventHandler();
	void CloseEventHandler();

	void EventRequestAsynchronous(int DelayMsec);
	void EventRequestCritical();
private:
	// ThreadInstance pure virtuals {
	virtual DWORD RunInThread();
	virtual void BreakThread();
	// }

	// IWindowMessageListener pure virtuals {
    virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// }
private:
	IAsyncEventHandlerTarget &m_rTarget;

	UtilityWindow m_UtilityWindow;

	Interlocked m_PendingAsyncRequests;
};

#endif // _ASYNC_FOCUS_HANDLER_HEADER_