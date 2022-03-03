#ifndef _SHELL_HOOK_DISPATCHER_HEADER_
#define _SHELL_HOOK_DISPATCHER_HEADER_

namespace WinHook
{
	typedef int Result;
	class IShellListener;

	class ShellHookDispatcher : public HookDispatcher<IShellListener>
	{
	public:
		ShellHookDispatcher();
		virtual ~ShellHookDispatcher();

		Result RegisterOnShellEvent(int Code, IShellListener &rListener);
		Result Unregister(const IShellListener &rListener);

		virtual Result OpenHook();

		virtual LRESULT Notify(IShellListener &rListener, int nCode, WPARAM wParam, LPARAM lParam);

		static LRESULT CALLBACK ShellProc(
			int code,
			WPARAM wParam,
			LPARAM lParam
			);
	private:
		LRESULT HandleEvent(
			int nCode,
			WPARAM wParam,
			LPARAM lParam
			);
	private:
		ListenerMap<int /*Code*/, IShellListener> m_ShellLisenerMap;
	};

} // namespace WinHook

#endif // _SHELL_HOOK_DISPATCHER_HEADER_
