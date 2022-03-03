#ifndef _CALCULATOR_POPUP_HANDLER_HEADER_
#define _CALCULATOR_POPUP_HANDLER_HEADER_

#include "IKeyboardEvent.h"

class KeyboardSequenceEvent;
class CalculatorHandler;

class CalculatorPopupHandler :
	public IKeyboardEvent
{
public:
	CalculatorPopupHandler(KeyboardSequenceEvent &rKeyboardSequenceEvent, CalculatorHandler &rCalculatorHandler);
	virtual ~CalculatorPopupHandler();

	bool Open(LPCTSTR KeyboardTrigger);
	void Close();
private:
	// IKeyboardEvent pure virtuals {
	virtual bool CanOperate(KeyboardEventInfo &Info, bool &rfExclusive);
	virtual void Operate(const KeyboardEventInfo &Info, IKeyboardEventCallback &rCallback);
	// }
private:
	KeyboardSequenceEvent &m_rKeyboardSequenceEvent;
	CalculatorHandler &m_rCalculatorHandler;
};

#endif //_CALCULATOR_POPUP_HANDLER_HEADER_