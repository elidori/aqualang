#include "Pch.h"
#include "DebugBalloonTip.h"
#include "ScreenFitter.h"
#include "resource.h"
#include "Log.h"

DebugBalloonTip::DebugBalloonTip()
	: DialogObject()
{
}

DebugBalloonTip::~DebugBalloonTip()
{
	Term();
}

bool DebugBalloonTip::Init(
			LPCTSTR UserName,
			LPCTSTR Text,
			const RECT *pRect,
			int DialogId
			)
{
	Term();

	HWND hDialog = OpenDialog(UserName, DialogId);
	if(hDialog == NULL)
		return false;

	if(Text)
	{
		::SetWindowText(::GetDlgItem(hDialog, IDC_STATIC_TEXT), Text);
	}

	// set position
	WINDOWPLACEMENT WindowPlacement;
	WindowPlacement.length = sizeof(WindowPlacement);
	if(!::GetWindowPlacement(hDialog, &WindowPlacement))
		return false;
	RECT WinRect = WindowPlacement.rcNormalPosition;

	if(!pRect)
		return false;
	int Width = WinRect.right - WinRect.left;
	int Height = WinRect.bottom - WinRect.top;

	if(!ScreenFitter::PlaceInScreen(Width, Height, *pRect, WinRect))
		return false;

	::SetWindowPos(
		hDialog,
        HWND_TOPMOST,
		WinRect.left,
		WinRect.top,
		WinRect.right - WinRect.left,
		WinRect.bottom - WinRect.top,
        SWP_SHOWWINDOW | SWP_NOACTIVATE
    );
	return true;
}

void DebugBalloonTip::Term()
{
	CloseDialog();
}
