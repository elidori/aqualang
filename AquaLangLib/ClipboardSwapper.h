#ifndef _CLIPBOARD_SWAPPER_HEADER_
#define _CLIPBOARD_SWAPPER_HEADER_

#include "IKeyboardEvent.h"
#include "UtilityWindow.h"

class KeyboardSequenceEvent;
class UserInformationNotifier;
class IKeyboardEventCallback;

class ClipboardSwapper :
	public IKeyboardEvent,
	public IWindowMessageListener
{
public:
	ClipboardSwapper(UtilityWindow &rUtilityWindow, KeyboardSequenceEvent &rKeyboardSequenceEvent);
	virtual ~ClipboardSwapper();

	bool Open(LPCTSTR KeyboardTrigger);
	void Close();
private:
	// IKeyboardEvent pure virtuals {
	virtual bool CanOperate(KeyboardEventInfo &Info, bool &fExclusive);
	virtual void Operate(const KeyboardEventInfo &Info, IKeyboardEventCallback &rCallback);
	// }

	// IWindowMessageListener pure virtuals {
	virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// }

private:
	UtilityWindow &m_rUtilityWindow;
	KeyboardSequenceEvent &m_rKeyboardSequenceEvent;
	IKeyboardEventCallback *m_pCallback;
};

#endif // _CLIPBOARD_SWAPPER_HEADER_