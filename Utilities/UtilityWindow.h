#ifndef _UTILITY_WINDOW_HEADER_
#define _UTILITY_WINDOW_HEADER_

#include "MessageWindow.h"
class IWindowMessageListener
{
protected:
	virtual ~IWindowMessageListener() {}
public:
	virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};

class UtilityWindow : public MessageWindow
{
public:
	UtilityWindow();
	virtual ~UtilityWindow();

	bool Open(LPCTSTR Title = _T(""));
	void Close();

	bool Register(IWindowMessageListener &rListener);
	bool Unregister(IWindowMessageListener &rListener);

	bool StartTimer(DWORD dwTimeout, int TimerId);
	void StopTimer(int TimerId);

	void UserMessage(WPARAM wParam, LPARAM lParam);
private:
    virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &rfContinue);
private:
	typedef std::map<int, int> TimerIdMap;
	TimerIdMap m_TimerIdMap;

	typedef std::map<int, IWindowMessageListener *> ListenerMap;
	ListenerMap m_ListenerMap;
};

#endif // _UTILITY_WINDOW_HEADER_