#ifndef _BASIC_COMPACT_TIP_HEADER_
#define _BASIC_COMPACT_TIP_HEADER_

#include "DialogObject.h"
#include "ToolTip.h"
#include "BitmapImage.h"
#include "TipCoordinator.h"

#define TIP_ALPHA 200
#define TIP_IN_FOCUS_ALPHA 255
#define COLORKEY (RGB(0, 0, 255))

class BasicCompactTip : public DialogObject
{
public:
	BasicCompactTip(int TextWindowId, TipCoordinator *pTipCoordinator);
	virtual ~BasicCompactTip();

protected:
	bool DoBasicOpen(LPCTSTR UserName, int DialogId, LPCTSTR DialogTitle, LPCTSTR DialogTip, bool fSetFont, bool fBoldFont);
	bool DoBasicInit(LPCTSTR TextString, int TextOffsetWidth, int DialogResidualWidth, const RECT &rCaretRect);
	void DoBasicTerm();
	void DoBasicClose();

	virtual bool CalculateRectAndType(const RECT &CaretRect, int TextWidth, int DialogResidualWidth, RECT &Rect);
	virtual bool ModifyTextBox(LPCTSTR Text, int TextX, int &rTextWidth);

	bool IsMouseInRect(const RECT &rRect) const;
	bool TipShouldBeOpaque(HWND hwnd) const;

	virtual void DialogInit(HWND hwndDlg);
    virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault);
	
	virtual bool DrawBackground(HDC hTargetDC);
protected:
	int m_Alpha;
private:
	int m_TextWindowId;
	TipCoordinatorClient m_TipCoordinatorClient;

	HFONT m_hFont; // because the font is not bold in standard ways
	bool m_fBoldFont;

	BitmapImage m_BackgroundBitmap;
	int m_DialogWidth;
	int m_LeftFixedMargin;
	int m_RightFixedMargin;

	ToolTip m_DialogToolTip;

	_tstring m_TextString;
};

#endif // _BASIC_COMPACT_TIP_HEADER_