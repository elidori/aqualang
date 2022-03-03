#ifndef _TRANSPARENT_EDIT_BOX_HEADER_
#define _TRANSPARENT_EDIT_BOX_HEADER_

class IEditBoxListener
{
protected:
	virtual ~IEditBoxListener() {}
public:
	virtual void NotifyKeyDown(HWND hwnd, int VirtualKey) = 0;
	virtual void NotifyKillFocus(HWND hwnd) = 0;
};

class TransparentEditBox
{
public:
	TransparentEditBox(IEditBoxListener *pListener);
	virtual ~TransparentEditBox();

	bool Open(HWND hWindow);
	void Close();
private:
	static LRESULT CALLBACK ThunkWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	HWND m_hWindow;
	WNDPROC m_OldWndProc;

	IEditBoxListener *m_pEditListener;
};

#endif // _TRANSPARENT_EDIT_BOX_HEADER_