#ifndef _I_COMPACT_TIP_RESULT_HEADER_
#define _I_COMPACT_TIP_RESULT_HEADER_

enum COMPACT_TIP_OPERATION
{
	COMPACT_TIP_OP_CLOSE,
	COMPACT_TIP_OP_APPLY
};

class ICompactTipResult
{
protected:
	virtual ~ICompactTipResult() {}
public:
	virtual void TipShouldClose(int Operation) = 0;
};

#endif // _I_COMPACT_TIP_RESULT_HEADER_