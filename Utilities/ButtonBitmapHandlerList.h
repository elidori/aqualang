#ifndef _BUTTON_BITMAP_HANDLER_LIST_HEADER_
#define _BUTTON_BITMAP_HANDLER_LIST_HEADER_

class ButtonBitmapHandler;

class ButtonBitmapHandlerList
{
public:
	ButtonBitmapHandlerList();
	virtual ~ButtonBitmapHandlerList();

	void SetDialogHandle(HWND hDialog)	{ m_hDialog = hDialog; }

	bool Add(
			int ControlId,
			LPCTSTR ResourceName,
			LPCTSTR Text = _T(""),
			COLORREF NormalText = RGB(0, 0, 0),
			COLORREF PressedText = RGB(0, 0, 0),
			COLORREF DisabledText = RGB(0, 0, 0),
			COLORREF HighlightedText = RGB(0, 0, 0),
			bool fUseManualState = false
			);

	bool Check(int ControlId, bool fCheck);
	bool GetChecked(int ControlId) const;
	bool Highlight(HWND hwnd);

	bool Draw(int ControlId, DRAWITEMSTRUCT *pDrawItemStruct);

	void Close();
private:
	HWND m_hDialog;

	typedef std::map<int, ButtonBitmapHandler *> BitmapObjectMap;
	BitmapObjectMap m_BitmapObjectIdMap;
	BitmapObjectMap m_BitmapObjectHWNDMap;

	HWND m_hHighLightedControl;
};

#endif // _BUTTON_BITMAP_HANDLER_LIST_HEADER_