#ifndef _KEYBOARD_HOOK_DISPATCHER_HEADER_
#define _KEYBOARD_HOOK_DISPATCHER_HEADER_

namespace WinHook
{
	typedef int Result;
	class IKeyboardListener;

	class KeyboardHookDispatcher : public HookDispatcher<IKeyboardListener>
	{
	public:
		KeyboardHookDispatcher();
		virtual ~KeyboardHookDispatcher();

		Result Register(IKeyboardListener &rListener);
		Result Unregister(const IKeyboardListener &rListener);

		virtual Result OpenHook();
	private:
		virtual LRESULT Notify(IKeyboardListener &rListener, int nCode, WPARAM wParam, LPARAM lParam);

		static LRESULT CALLBACK LowLevelKeyboardProc(
			int code,
			WPARAM wParam,
			LPARAM lParam
			);

		LRESULT HandleEvent(
			int nCode,
			WPARAM wParam,
			LPARAM lParam
			);
	private:
		ListenerList<IKeyboardListener> m_KeyboardLisenerList;
	};

} // namespace WinHook

extern WinHook::KeyboardHookDispatcher g_KeyboardHookDispatcher;

#endif // _KEYBOARD_HOOK_DISPATCHER_HEADER_
