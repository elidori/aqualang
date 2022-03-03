#ifndef _MESSAGE_HOOK_DISPATCHER_HEADER_
#define _MESSAGE_HOOK_DISPATCHER_HEADER_

#include "HookDispatcher.h"
#include "WinHookListenerMap.h"

namespace WinHook
{
	typedef int Result;
	class IWindowListener;

	class MessageHookDispatcher : public HookDispatcher<IWindowListener>
	{
	public:
		MessageHookDispatcher();
		virtual ~MessageHookDispatcher();

		// windows messages subscription: either on all message in a single window, or on a sepecific message in all windows
		Result RegisterOnWindow(HWND hwnd, IWindowListener &rListener);
		Result RegisterOnMessage(UINT uMsg, IWindowListener &rListener);

		Result Unregister(const IWindowListener &rListener);

		virtual Result OpenHook();
	private:
		virtual LRESULT Notify(IWindowListener &rListener, int nCode, WPARAM wParam, LPARAM lParam);

		static LRESULT CALLBACK CallWndProc(
			int nCode,
			WPARAM wParam,
			LPARAM lParam
			);
		LRESULT HandleMessage(
			int nCode,
			WPARAM wParam,
			LPARAM lParam
			);
	private:
		ListenerMap<HWND, IWindowListener> m_WindowLisenerMap;
		ListenerMap<UINT /*uMsg*/, IWindowListener> m_MessageLisenerMap;
	};

} // namespace WinHook

#endif // _MESSAGE_HOOK_DISPATCHER_HEADER_