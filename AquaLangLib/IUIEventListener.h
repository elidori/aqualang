#ifndef _UI_EVENT_LISTENER_HEADER_
#define _UI_EVENT_LISTENER_HEADER_

namespace WinHook
{
	enum EventType
	{
		Type_Focus,
		Type_StoreLanguage,
		Type_KeyboardSequence
	};

	enum UIActionType
	{
		ActionType_Unknown,
		ActionType_EditWindow,
		ActionType_NonEditWindow
	};

	class IUIEventListener
	{
	public:
		virtual ~IUIEventListener() {}

		virtual void UIEvent(EventType Type, HWND hwnd, LPCTSTR SubObjectId, const RECT *pRect, UIActionType Action) = 0;
	};
}

#endif // _UI_EVENT_LISTENER_HEADER_