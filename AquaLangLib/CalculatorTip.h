#ifndef _CALCULATOR_TIP_HEADER_
#define _CALCULATOR_TIP_HEADER_

#include "DialogObject.h"
#include "ButtonBitmapHandlerList.h"
#include "TransparentEditBox.h"
#include "ToolTip.h"

class ICalcTipResult;

class CalculatorTip :
	public DialogObject,
	public IEditBoxListener
{
public:
	CalculatorTip(ICalcTipResult &rCalcTipResult, int MaxExpressionLength);
	virtual ~CalculatorTip();

	bool Open(LPCTSTR UserName);
	bool IsOpen() const;

	void SetCaretRect(const RECT &rCaretRect);
	void SetFocus();

	bool Init(LPCTSTR Expression, LPCTSTR Result, bool fIndependant);
	void Term();

	bool IsOwnerOf(HWND hWindow);
	bool Reevaluate(LPCTSTR Result);

	void Close();
private:
	bool ScaleDialogComponents(HWND hDialog, double xScale, double yScale);
	void ResizeDialogToMatchDialogUnits(HWND hDialog);
	bool CalcDialogRect(const RECT &CaretRect, RECT &Rect);

    virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault);

	void CheckInv(bool fCheck);
	bool IsInInv();
	bool AddText(LPCTSTR Text);
	bool Evaluate();
	bool Execute(LPCTSTR Operator);

	bool RefreshResultField(LPCTSTR Result);
	bool ReduceCalculator();
	bool UpdateExpressionString(LPCTSTR Expression);
	bool CopyResultToExpression();
	void RefreshEditBoxes();

	// IEditBoxListener pure virtuals {
	virtual void NotifyKeyDown(HWND hwnd, int VirtualKey);
	virtual void NotifyKillFocus(HWND hwnd);
	// }
private:
	ICalcTipResult &m_rCalcTipResult;

	const int m_MaxExpressionLength;
	LPTSTR m_ExpressionTextContainer;

	RECT m_CaretRect;

	RECT m_RecentWindowRect;
	bool m_fInIndependantMode;

	ButtonBitmapHandlerList m_ButtonBitmapList;
	
	TransparentEditBox m_ExpressionEditBox;
	TransparentEditBox m_ResultEditBox;

	ToolTip m_ButtonReduceTip;
	ToolTip m_ButtonAssignTip;
};

#endif // _CALCULATOR_TIP_HEADER_