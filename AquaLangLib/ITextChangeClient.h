#ifndef _I_TEXT_CHANGE_CLIENT_HEADER_
#define _I_TEXT_CHANGE_CLIENT_HEADER_

class KeyboardEventInfo;

class ITextChangeClient
{
protected:
	virtual ~ITextChangeClient() {}
public:
	virtual void NotifyOnTextChangeTrigger(const KeyboardEventInfo &Info, bool &fUsed) = 0;
};

#endif // _I_TEXT_CHANGE_CLIENT_HEADER_