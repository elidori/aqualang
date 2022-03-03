#ifndef _HOOK_DISPATCHER_HEADER_
#define _HOOK_DISPATCHER_HEADER_

#include "HookHandler.h"
#include "WinHookListenerList.h"

namespace WinHook
{
	typedef int Result;

	template<class T>
	class HookDispatcher : public HookHandler
	{
	public:
		HookDispatcher()			{}
		virtual ~HookDispatcher()	{}

		virtual Result OpenHook() = 0;
	protected:
		LRESULT Dispatch(
					ListenerList<T> &rList, 
					int nCode,
					WPARAM wParam,
					LPARAM lParam
					);

		virtual LRESULT Notify(T &rListener, int nCode, WPARAM wParam, LPARAM lParam) = 0;
	};

	template<class T>
	inline LRESULT HookDispatcher<T>::Dispatch(
										ListenerList<T> &rList, 
										int nCode,
										WPARAM wParam,
										LPARAM lParam
										)
	{
		LRESULT lResult = TRUE;

		T *pNextListener = NULL;
		Result res = rList.First(pNextListener);

		StartedHandlingHook();

		while(res == Result_Success && pNextListener != NULL)
		{
			lResult &= Notify(*pNextListener, nCode, wParam, lParam);
			res = rList.Next(pNextListener);
		}

		EndedHandlingHook();

		return lResult;
	}

} // namespace WinHook

#endif // _HOOK_DISPATCHER_HEADER_
