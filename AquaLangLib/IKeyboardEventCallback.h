#ifndef _I_KEYBOARD_EVENT_CALLBACK_HEADER
#define _I_KEYBOARD_EVENT_CALLBACK_HEADER

class KeyboardEventInfo;

class IKeyboardEventCallback
{
protected:
	virtual ~IKeyboardEventCallback() {}
public:
	virtual void EventOperationEnded(bool fComplete) = 0;
};

#endif // _I_KEYBOARD_EVENT_CALLBACK_HEADER