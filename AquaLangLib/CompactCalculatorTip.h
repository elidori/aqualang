#ifndef _COMPACT_CALCULATOR_TIP_HEADER_
#define _COMPACT_CALCULATOR_TIP_HEADER_

#include "CompactTip.h"

class TipCoordinator;

class CompactCalculatorTip : public CompactTip
{
public:
	CompactCalculatorTip(ICompactTipResult &rTipResult, TipCoordinator &rTipCoordinator);
	virtual ~CompactCalculatorTip();

	bool Open(LPCTSTR UserName, int ApearanceDurationIn100msec);

	void SetCaretRect(const RECT &rCaretRect);

	bool Init(LPCTSTR Result);
	void StoreResult(LPCTSTR Result);

	void Term();
	void Close();
private:
	RECT m_RecentCaretRect;

	ToolTip m_ResultEditToolTip;

	_tstring m_ResultString;
};

#endif // _COMPACT_CALCULATOR_TIP_HEADER_