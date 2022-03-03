#ifndef _UTILITY_WINDOW_PARAMS_HEADER_
#define _UTILITY_WINDOW_PARAMS_HEADER_

enum
{
	WM_UTILITY_USER_MESSAGE = WM_USER + 10,

	// library
	WM_ASYNCEVENTREQUEST = WM_UTILITY_USER_MESSAGE + 1,
	WM_CRITICALEVENTREQUEST = WM_UTILITY_USER_MESSAGE + 2,
	WM_CLOSEEVENTHANDLER = WM_UTILITY_USER_MESSAGE + 3,

	WM_NOTIFY_NO_FOCUS_CHANGE = WM_UTILITY_USER_MESSAGE + 4,

	WM_KEYBOARDASYNCEVENT = WM_UTILITY_USER_MESSAGE + 5,
	WM_MOUSEASYNCEVENT = WM_UTILITY_USER_MESSAGE + 6,
	WM_CLOSEMESSAGEWINDOW = WM_UTILITY_USER_MESSAGE + 7,

	// application
	WM_MESSAGEWINDOW_TYPE = WM_UTILITY_USER_MESSAGE + 8, // for backward compatibility it should have been WM_UTILITY_USER_MESSAGE
	WM_NOTIFICATIONAREA = WM_UTILITY_USER_MESSAGE + 9
};

enum UTILITY_WPARAM
{
	WPARAM_TEXT_CHANGE_REQUEST_EVENT = 1,
	WPARAM_BALLOON_DISPLAY_EVENT = 2,
	WPARAM_KEYBOARD_SEQUENCE_EVENT = 3,
	WPARAM_KEYBOARD_SEQUENCE_EVENT_COMPLETED = 4,
	WPARAM_KEYBOARD_SEQUENCE_EVENT_ABORTED = 5,
	WPARAM_CALC_OP_EXPAND = 6,
	WPARAM_CALC_OP_REDUCE = 7,
	WPARAM_CALC_POPUP = 8,
	WPARAM_CLIPBOARD_SWAP = 9,

	// application
	WPARAM_TRIGGER_CALC = 10, // for backward compatibility it should have been 1
	WPARAM_REMOTE_COMMAND = 11,
};

enum UTILITY_TIMER_ID
{
	TIMER_ID_DELAYED_FOCUS_CHECK = 1,
	TIMER_ID_CALCULATOR_EVAL = 2,
	TIMER_ID_TEXT_CHANGE_VERIFY = 3,
	TIMER_ID_ASYNC_FOCUS_HANDLE = 4,
	TIMER_ID_GUESS_LANGUAGE = 5,

	// application
	TIMER_ID_TIP_OF_THE_DAY = 100,
	TIMER_ID_APP_WATCHDOG = 101
};

#endif // _UTILITY_WINDOW_PARAMS_HEADER_