#ifndef _HOOK_HANDLER_HEADER_
#define _HOOK_HANDLER_HEADER_

namespace WinHook
{
	typedef int Result;

	class HookHandler
	{
	protected:
		HookHandler();
		virtual ~HookHandler();

		Result DoOpenHook(int idHook, HOOKPROC lpfn);
	public:
		void CloseHook();
	protected:
		void StartedHandlingHook()	{ m_fInHook = true; }
		void EndedHandlingHook()	{ m_fInHook = false; }
		LRESULT CompletedHandlingHook(
			int nCode,
			WPARAM wParam,
			LPARAM lParam
			);
	private:
		HHOOK m_Hook;

		bool m_fInHook;
		bool m_fCloseRequest;
	};

} // namespace WinHook

#endif // _HOOK_HANDLER_HEADER_