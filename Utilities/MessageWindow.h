#ifndef _MESSAGE_WINDOW_HEADER_
#define _MESSAGE_WINDOW_HEADER_

class MessageWindow
{
public:
	MessageWindow();
	virtual ~MessageWindow();

	bool OpenMessageWindow(LPCTSTR Title = _T(""));
	void CloseMessageWindow();

	void PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &rfContinue);

	HWND GetHandle() const	{ return m_hMessageWnd; }
private:
	static LRESULT CALLBACK ThunkWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	HWND m_hMessageWnd;
	WNDPROC m_OldWndProc;
};

#endif // _MESSAGE_WINDOW_HEADER_