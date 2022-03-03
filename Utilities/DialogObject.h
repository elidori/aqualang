#ifndef _DIALOG_OBJECT_HEADER_
#define _DIALOG_OBJECT_HEADER_

#include "DialogIcon.h"
#include "AquaLangWaterMark.h"
#include "BitmapImage.h"

class DialogObject
{
protected:
	DialogObject(int DialogIconId = -1, bool fHideFromTaskbar = true);
	virtual ~DialogObject();

	bool SetDialogBitmap(int BitmapId, bool fDialogFitsBitmap);

	HWND OpenDialog(LPCTSTR UserName, int DialogId, HWND hParent = NULL);
	bool IsDialogOpen() const	{ return m_hDialog != NULL; }
	void CloseDialog();

	virtual void DialogInit(HWND UNUSED(hwndDlg)) {}
	virtual LRESULT WindowProc(HWND UNUSED(hwnd), UINT UNUSED(uMsg), WPARAM UNUSED(wParam), LPARAM UNUSED(lParam), bool &fSkipDefault) { fSkipDefault = false; return 0; }
	virtual LRESULT LateWindowProc(HWND UNUSED(hwnd), UINT UNUSED(uMsg), WPARAM UNUSED(wParam), LPARAM UNUSED(lParam), bool &fOverrideDefault) { fOverrideDefault = false; return 0; }

	bool StartTimer(int nTimerId, DWORD dwTimeout);
	void StopTimer(int nTimerId);

	void PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	static bool GetDialogUserName(HWND hDialog, _tstring &rUserName);
public:
	HWND GetDialogHandle() const { return m_hDialog; }
	
	void Show(bool fShow);
	bool IsVisible() const;

	bool SetTitle(LPCTSTR Title);

	bool IsPointInWindow(int x, int y) const;

	static bool IsPointInRect(int x, int y, const RECT &rRect);
	static HWND FindAquaLangWindow(LPCTSTR Title, LPCTSTR UserName);
private:
	static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK ThunkWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT BaseWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fMySkipDefault);

	bool FitDialogRectToBitmap();

	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
private:
	const bool m_fHideFromTaskbar;
	HWND m_hParent;

	HWND m_hDialog;
	WNDPROC m_OldWndProc;

	DialogIcon m_DialogIcon;

	AquaLangWaterMark m_WaterMark;

	BitmapImage m_DialogBitmap;
	bool m_fDialogFitsBitmap; // either the bitmap resizes or the dialog resizes
};

#endif // _DIALOG_OBJECT_HEADER_