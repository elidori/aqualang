#ifndef _I_CALC_TIP_RESULT_HEADER_
#define _I_CALC_TIP_RESULT_HEADER_

#include "ICompactTipResult.h"

enum CALC_OPERATION
{
	CALC_OP_CLOSE = COMPACT_TIP_OP_CLOSE,
	CALC_OP_EXPAND = COMPACT_TIP_OP_APPLY,
	CALC_OP_REDUCE
};

class ICalcTipResult : public ICompactTipResult
{
protected:
	virtual ~ICalcTipResult() {}
public:
	virtual void TipShouldReevaluate(LPCTSTR Expression) = 0;
	virtual void AngleUnitModeChanged(bool fUseDegrees) = 0;
};

#endif // _I_CALC_TIP_RESULT_HEADER_