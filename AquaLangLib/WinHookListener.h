#ifndef _WIN_HOOK_LISTENER_HEADER_
#define _WIN_HOOK_LISTENER_HEADER_

namespace WinHook
{
	class IWindowListener
	{
	public:
		virtual LRESULT OnMessageHookEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	};

	class IShellListener
	{
	public:
		virtual LRESULT OnShellHookEvent(int Code, WPARAM wParam, LPARAM lParam) = 0;
	};

	class IMouseListener
	{
	public:
		virtual LRESULT OnMouseHookEvent(HWND hwnd, UINT uMsg, int x, int y) = 0;
	};

	class IKeyboardListener
	{
	public:
		virtual LRESULT OnKeyboardHookEvent(UINT uMsg, int Code) = 0;
	};
}

#endif // _WIN_HOOK_LISTENER_HEADER_