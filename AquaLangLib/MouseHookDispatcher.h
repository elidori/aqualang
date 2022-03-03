#ifndef _MOUSE_HOOK_DISPATCHER_HEADER_
#define _MOUSE_HOOK_DISPATCHER_HEADER_

namespace WinHook
{
	typedef int Result;
	class IMouseListener;

	class MouseHookDispatcher : public HookDispatcher<IMouseListener>
	{
	public:
		MouseHookDispatcher();
		virtual ~MouseHookDispatcher();

		Result RegisterOnWindowMouseEvents(HWND hwnd, IMouseListener &rListener);
		Result RegisterOnMouseEvent(UINT uMsg, IMouseListener &rListener);

		Result Unregister(const IMouseListener &rListener);

		virtual Result OpenHook();
	private:
		virtual LRESULT Notify(IMouseListener &rListener, int nCode, WPARAM wParam, LPARAM lParam);

		static LRESULT CALLBACK MouseProc(
			int nCode,
			WPARAM wParam,
			LPARAM lParam
			);
		LRESULT HandleEvent(
			int nCode,
			WPARAM wParam,
			LPARAM lParam
			);
	private:
		ListenerMap<HWND, IMouseListener> m_WindowMouseLisenerMap;
		ListenerMap<UINT /*uMsg*/, IMouseListener> m_MouseMessageLisenerMap;
	};

} // namespace WinHook


extern WinHook::MouseHookDispatcher g_MouseHookDispatcher;

#endif // _MOUSE_HOOK_DISPATCHER_HEADER_
