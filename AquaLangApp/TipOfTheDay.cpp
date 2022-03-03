#include "Pch.h"
#include "TipOfTheDay.h"
#include "AquaLangConfiguration.h"
#include "ConfigurationXml.h"
#include "TipOfTheDayScheduler.h"
#include "IConfigUpdateListener.h"
#include "resource.h"
#include "Log.h"

#define COLORKEY (RGB(242, 243, 244))
#define TIP_ALPHA 220
#define TIP_ALPHA_OPAQUE 255

#define ALPHA_TEST_POLLING_MSEC 100

enum
{
	TIP_ALPHA_TEST_TIMER_ID = 1,
};

TipOfTheDay::TipOfTheDay(TipOfTheDayScheduler &rScheduler, IConfigUpdateListener &rListener)
	: DialogObject(IDI_ICON_DROP),
	m_rScheduler(rScheduler),
	m_rListener(rListener),
	m_hFont(NULL),
	m_fLatelyWasOpaque(false)
{
}

TipOfTheDay::~TipOfTheDay()
{
	Close();
}

bool TipOfTheDay::Open(LPCTSTR UserName)
{
	if(IsDialogOpen())
		return true;

	m_UserName = UserName;

	if(!SetDialogBitmap(IDB_BITMAP_TIP_OF_THE_DAY, false))
		return false;

	HWND hDialog = OpenDialog(UserName, IDD_DIALOG_TIP_OF_THE_DAY);
	if(hDialog == NULL)
		return false;
	SetTitle(_T("AquaLang Tip of the day"));
	Show(false);

	m_ButtonBitmapList.SetDialogHandle(hDialog);
	m_ButtonBitmapList.Add(IDC_BUTTON_NEXT_TIP_OF_THE_DAY, _T("BUTTON_BITMAP_NEXTTIP"));
	m_ButtonBitmapList.Add(IDC_BUTTON_PREV_TIP_OF_THE_DAY, _T("BUTTON_BITMAP_PREVTIP"));
	m_ButtonBitmapList.Add(IDC_BUTTON_CLOSE_TIP_OF_THE_DAY, _T("BUTTON_BITMAP_CLOSETIP"));

	// open tool tips
	m_CloseButtonToolTip.Open(hDialog, ::GetDlgItem(hDialog, IDC_BUTTON_CLOSE_TIP_OF_THE_DAY), _T("Close Tip Window"));
	m_NextButtonToolTip.Open(hDialog, ::GetDlgItem(hDialog, IDC_BUTTON_NEXT_TIP_OF_THE_DAY), _T("Next Tip"));
	m_PrevButtonToolTip.Open(hDialog, ::GetDlgItem(hDialog, IDC_BUTTON_PREV_TIP_OF_THE_DAY), _T("Previous Tip"));

	// set font
	int FontHeight = 16;
	int FontWidth = 10;
	m_hFont = ::CreateFont(
						-FontHeight,
						-FontWidth,
						0,
						0,
						FW_BOLD,
						FALSE,
						FALSE,
						FALSE,
						DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS, 
						CLIP_DEFAULT_PRECIS,
						ANTIALIASED_QUALITY,//DEFAULT_QUALITY,
						DEFAULT_PITCH,
						_T("MS Sans Serif")
						);

	return true;
}

bool TipOfTheDay::CanPresent()
{
	return !IsVisible();
}

void TipOfTheDay::Present(LPCTSTR Text, bool fMoreNext, bool fMorePrev)
{
	if(!UpdateContent(Text, fMoreNext, fMorePrev))
		return;

	ModifyDialogPosition();

	m_fLatelyWasOpaque = false;
	StartTimer(TIP_ALPHA_TEST_TIMER_ID, ALPHA_TEST_POLLING_MSEC);
}

void TipOfTheDay::Close()
{
	StopTimer(TIP_ALPHA_TEST_TIMER_ID);
	CloseDialog();

	m_ButtonBitmapList.Close();

	m_CloseButtonToolTip.Close();
	m_NextButtonToolTip.Close();
	m_PrevButtonToolTip.Close();

	if(m_hFont != NULL)
	{
		::DeleteObject(m_hFont);
		m_hFont = NULL;
	}
}

void TipOfTheDay::DialogInit(HWND hwndDlg)
{
	::SetWindowLong(hwndDlg, GWL_EXSTYLE, ::GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_LAYERED);
	::SetLayeredWindowAttributes(hwndDlg, COLORKEY, TIP_ALPHA, LWA_COLORKEY | LWA_ALPHA);
}

LRESULT TipOfTheDay::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault)
{
	fSkipDefault = false;
	LRESULT lResult = 0;

	switch(uMsg)
	{
	case WM_TIMER:
		if(wParam == TIP_ALPHA_TEST_TIMER_ID)
		{
			bool fShouldBeOpaque = IsMouseOnTipWindow();
			if(fShouldBeOpaque != m_fLatelyWasOpaque)
			{
				m_fLatelyWasOpaque = fShouldBeOpaque;
				::SetLayeredWindowAttributes(hwnd, COLORKEY, m_fLatelyWasOpaque ? TIP_ALPHA_OPAQUE : TIP_ALPHA, LWA_COLORKEY | LWA_ALPHA);
			}
		}
		break;
	case WM_COMMAND:
		if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CLOSE_TIP_OF_THE_DAY))
		{
			StopTimer(TIP_ALPHA_TEST_TIMER_ID);
			Show(false);

			bool fStopShowingTipa = (::SendMessage(::GetDlgItem(hwnd, IDC_CHECK_STOP_TIPS), BM_GETCHECK, 0, 0) == BST_CHECKED);
			if(fStopShowingTipa)
			{
				DisableTipsInConfiguration();
			}
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_NEXT_TIP_OF_THE_DAY))
		{
			bool fMoreNext;
			bool fMorePrev;
			LPCTSTR Text = m_rScheduler.GetNextText(fMoreNext, fMorePrev);
			if(Text != NULL && _tcslen(Text) > 0)
			{
				UpdateContent(Text, fMoreNext, fMorePrev);
			}
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_PREV_TIP_OF_THE_DAY))
		{
			bool fMoreNext;
			bool fMorePrev;
			LPCTSTR Text = m_rScheduler.GetPrevText(fMoreNext, fMorePrev);
			if(Text != NULL && _tcslen(Text) > 0)
			{
				UpdateContent(Text, fMoreNext, fMorePrev);
			}
		}
		break;
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
	    ::SetBkMode((HDC)wParam, TRANSPARENT);
		fSkipDefault = true;
		lResult = (LRESULT)::GetStockObject(NULL_BRUSH);
		if((HWND)lParam == ::GetDlgItem(hwnd, IDC_STATIC_TIP_OF_THE_DAY_TITLE))
		{
			::SetTextColor((HDC)wParam, RGB(255, 255, 255));
			if(m_hFont != NULL)
			{
				::SelectObject((HDC)wParam, m_hFont);
			}
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_STATIC_TIP_OF_THE_DAY_MESSAGE))
		{
//			::SetTextColor((HDC)wParam, RGB(255, 255, 255));
		}
		break;
	case WM_DRAWITEM:
		if(m_ButtonBitmapList.Draw((int)wParam, (DRAWITEMSTRUCT *)lParam))
		{
			fSkipDefault = true;
			lResult = TRUE;
		}
		break;
	case WM_SETCURSOR:
		m_ButtonBitmapList.Highlight((HWND)wParam);
		break;
	}
	return lResult;
}

bool TipOfTheDay::UpdateContent(LPCTSTR Text, bool fMoreNext, bool fMorePrev)
{
	HWND hDialog = GetDialogHandle();
	if(hDialog == NULL)
		return false;
	HWND hTextWindow = ::GetDlgItem(hDialog, IDC_STATIC_TIP_OF_THE_DAY_MESSAGE);
	if(hTextWindow == NULL)
		return false;
	::SetWindowText(hTextWindow, Text);

	::EnableWindow(::GetDlgItem(hDialog, IDC_BUTTON_NEXT_TIP_OF_THE_DAY), fMoreNext);
	::EnableWindow(::GetDlgItem(hDialog, IDC_BUTTON_PREV_TIP_OF_THE_DAY), fMorePrev);

	::InvalidateRect(hDialog, NULL, TRUE);
	::InvalidateRect(::GetDlgItem(hDialog, IDC_STATIC_TIP_OF_THE_DAY_MESSAGE), NULL, TRUE);
	return true;
}

bool TipOfTheDay::ModifyDialogPosition()
{
	RECT rctDesktop;
	if(!::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rctDesktop, NULL))
		return false;

	APPBARDATA Info;
	Info.cbSize = sizeof(Info);
	if(!::SHAppBarMessage(ABM_GETTASKBARPOS, &Info))
		return false;

	HWND hDialog = GetDialogHandle();
	if(hDialog == NULL)
		return false;

	RECT DialogRect;
	if(!::GetWindowRect(GetDialogHandle(), &DialogRect))
		return false;

	int Width = DialogRect.right - DialogRect.left;
	int Height = DialogRect.bottom - DialogRect.top;
	int X = 0;
	int Y = 0;

	if(Info.rc.top >= rctDesktop.bottom)
	{
		// notification area is in the bottom of the screen on the right side
		X = rctDesktop.right - Width;
		Y = Info.rc.top - Height;
	}
	else if(Info.rc.bottom <= rctDesktop.top)
	{
		// notification area is in the top of the screen on the right side
		X = rctDesktop.right - Width;
		Y = Info.rc.bottom;
	}
	else if(Info.rc.left >= rctDesktop.right)
	{
		// notification area is in the right size of the screen in the bottom corner
		X = Info.rc.left - Width;
		Y = Info.rc.bottom - Height;
	}
	else if(Info.rc.right <= rctDesktop.left)
	{
		// notification area is in the left size of the screen in the bottom corner
		X = Info.rc.right;
		Y = Info.rc.bottom - Height;
	}
	else
	{
		return false;
	}

	::SetWindowPos(
		hDialog,
		HWND_TOPMOST,
		X,
		Y,
		Width,
		Height,
		SWP_SHOWWINDOW | SWP_NOACTIVATE
	);

	return true;
}

bool TipOfTheDay::IsMouseOnTipWindow() const
{
	RECT Rect;
	if(!::GetWindowRect(GetDialogHandle(), &Rect))
		return false;

	CURSORINFO Info;
	Info.cbSize = sizeof(Info);
	if(!::GetCursorInfo(&Info))
		return false;

	if(Info.ptScreenPos.x < Rect.left || Info.ptScreenPos.x >= Rect.right)
		return false;
	if(Info.ptScreenPos.y < Rect.top || Info.ptScreenPos.y >= Rect.bottom)
		return false;

	return true;
}

void TipOfTheDay::DisableTipsInConfiguration()
{
	ConfigurationXml ConfigXml(m_UserName.c_str());

	AquaLangConfiguration Config;
	__int64 CurrentFileVersion;
	bool fDebug;
	bool fDebugModeValid;
	ConfigXml.Load(Config, CurrentFileVersion, fDebug, fDebugModeValid);

	Config.NotificationArea.fShowTipOfTheDay = false;

	ConfigXml.Store(Config, CurrentFileVersion + 1, fDebug, fDebugModeValid, false);

	m_rListener.ConfigurationChanged();
}
