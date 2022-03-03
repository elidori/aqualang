#include "Pch.h"
#include "CalculatorPopupHandler.h"
#include "KeyboardSequenceEvent.h"
#include "CalculatorHandler.h"

CalculatorPopupHandler::CalculatorPopupHandler(KeyboardSequenceEvent &rKeyboardSequenceEvent, CalculatorHandler &rCalculatorHandler)
	: m_rKeyboardSequenceEvent(rKeyboardSequenceEvent),
	m_rCalculatorHandler(rCalculatorHandler)
{
}

CalculatorPopupHandler::~CalculatorPopupHandler()
{
	Close();
}

bool CalculatorPopupHandler::Open(LPCTSTR KeyboardTrigger)
{
	Result res = m_rKeyboardSequenceEvent.Register(KeyboardTrigger, *this, false);
	if(res != Result_Success)
		return false;
	return true;
}

void CalculatorPopupHandler::Close()
{
	m_rKeyboardSequenceEvent.Unregister(*this);
}

bool CalculatorPopupHandler::CanOperate(KeyboardEventInfo & UNUSED(Info), bool &rfExclusive)
{
	rfExclusive = false;
	return true;
}

void CalculatorPopupHandler::Operate(const KeyboardEventInfo & UNUSED(Info), IKeyboardEventCallback &rCallback)
{
	m_rCalculatorHandler.NotifyUserTriggersCalculator();
	rCallback.EventOperationEnded(true);
}
