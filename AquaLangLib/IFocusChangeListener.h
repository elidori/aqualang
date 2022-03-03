#ifndef _I_FOCUS_CHANGE_LISTENER_HEADER_
#define _I_FOCUS_CHANGE_LISTENER_HEADER_

class IFocusChangeListener
{
protected:
	virtual ~IFocusChangeListener() {}
public:
	virtual void FocusChanged(HWND hFocusWindow) = 0;
};

#endif // _I_FOCUS_CHANGE_LISTENER_HEADER_