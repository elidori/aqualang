#ifndef _TEXT_DIALOG_HEADER_
#define _TEXT_DIALOG_HEADER_

#include "DialogObject.h"

class TextDialog : public DialogObject
{
public:
	TextDialog(int DialogId, int TextId);
	virtual ~TextDialog();

	bool Open(LPCTSTR UserName, LPCTSTR Text, HWND hParent);
	void Close();

	HWND GetItem(int Id);
private:
	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault);
	virtual LRESULT LateWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fOverrideDefault);
private:
	const int m_DialogId;
	const int m_TextId;

	const COLORREF m_BkColor;
	HBRUSH m_BackgroundBrush;
};

#endif // _TEXT_DIALOG_HEADER_