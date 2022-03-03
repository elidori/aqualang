#ifndef _WIN_HOOK_RESULT_HEADER_
#define _WIN_HOOK_RESULT_HEADER_

namespace WinHook
{
	typedef int Result;

	enum
	{
		Result_Success,
		Result_False,
		Result_InvalidParameter,
		Result_MemAllocFailed,
		Result_ResourceAlreadyInUse,
		Result_ResourceAllocFailed,
		Result_ResourceNotFound,
		Result_InternalError
	};
};

#endif // _WIN_HOOK_RESULT_HEADER_