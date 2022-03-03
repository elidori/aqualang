#ifndef _FLOATING_MENU_HEADER_
#define _FLOATING_MENU_HEADER_

class IMenuEventHandler
{
protected:
	virtual ~IMenuEventHandler() {}
public:
	virtual void OptionSelected(int OptionId) = 0;
};

class FloatingMenu
{
public:
	FloatingMenu(IMenuEventHandler &rEventHandler);
	virtual ~FloatingMenu();

	bool Open(int MenuId);
	
	bool Popup(const POINT &rSubjectPoint);
	bool EnableOption(int OptionId, bool fEnable);
	
	void Close();
private:
	static LRESULT CALLBACK ThunkWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool OpenWindow();
	void CloseWindow();
private:
	IMenuEventHandler &m_rEventHandler;

	HWND m_hWindow;
	WNDPROC m_OldWndProc;

	HMENU m_hMenu;
};

#endif // _FLOATING_MENU_HEADER_