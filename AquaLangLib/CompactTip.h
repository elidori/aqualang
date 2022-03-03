#ifndef _COMPACT_TIP_HEADER_
#define _COMPACT_TIP_HEADER_

#include "BasicCompactTip.h"

class ICompactTipResult;

class CompactTip : public BasicCompactTip
{
public:
	CompactTip(ICompactTipResult &rTipResult, int TextWindowId, TipCoordinator *pTipCoordinator);
	virtual ~CompactTip();
protected:
	bool DoOpen(LPCTSTR UserName, int ApearanceDurationIn100msec, int DialogId, LPCTSTR DialogTitle, LPCTSTR DialogTip, bool fBold);
	bool DoInit(LPCTSTR TextString, const RECT &rCaretRect);
	void DoTerm();
	void DoClose();

	void TipShouldClose(int Operation);
private:
	virtual bool CalculateRectAndType(const RECT &CaretRect, int TextWidth, int DialogResidualWidth, RECT &Rect);
    virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault);
	
	virtual bool DrawBackground(HDC hTargetDC);
private:
	ICompactTipResult &m_rCompactTipResult;

	bool m_fFading;
	int m_nTimerCount;
	int m_ApearanceDurationInMsec;

	BitmapImage m_WaterDropIconBitmap;
	RECT m_WaterDropRectInDialog;
};

#endif // _COMPACT_TIP_HEADER_