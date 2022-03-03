#ifndef _I_KEYBOARD_EVENT_HEADER
#define _I_KEYBOARD_EVENT_HEADER

class KeyboardEventInfo;
class IKeyboardEventCallback;

class IKeyboardEvent
{
protected:
	virtual ~IKeyboardEvent() {}
public:
	virtual bool CanOperate(/*in, out*/KeyboardEventInfo &Info, bool &rfExclusive) = 0; // info may be built in several calls, when going through the list of clients
	virtual void Operate(const KeyboardEventInfo &Info, IKeyboardEventCallback &rCallback) = 0;
};

#endif // _I_KEYBOARD_EVENT_HEADER