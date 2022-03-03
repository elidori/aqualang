#ifndef _BUTTON_BITMAP_HANDLER_HEADER_
#define _BUTTON_BITMAP_HANDLER_HEADER_

#include "BitmapImage.h"

class ButtonBitmapHandler
{
public:
	ButtonBitmapHandler(LPCTSTR ResourceName, HWND hWindow, bool fUseManualState = false);
	virtual ~ButtonBitmapHandler();

	void SetText(
				LPCTSTR Text,
				COLORREF NormalColor,
				COLORREF PressedColor,
				COLORREF DisabledColor,
				COLORREF HighlightedColor
				);

	void Check(bool fCheck); // in case of manual state
	bool GetChecked() const;
	void HighLight(bool fHighLight);

	bool Draw(const DRAWITEMSTRUCT &DrawItem);
private:
	const HWND m_hWindow;
	const bool m_fUseManualState;
	bool m_fChecked;

	bool m_fHighLight;

	BitmapImage m_NormalBitmap;
	BitmapImage m_PressedBitmap;
	BitmapImage m_DisabledBitmap;
	BitmapImage m_HighlightedBitmap;
};

#endif // _BUTTON_BITMAP_HANDLER_HEADER_