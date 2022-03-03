#include "Pch.h"
#include "BasicCompactTip.h"
#include "FocusInfo.h"
#include "resource.h"
#include "Log.h"

#define TIP_ARROW_OFFSET 18
#define VERT_MARGIN_TO_TEXT 0

#define POINTER_FIXED_WIDTH 20
#define SIMPLE_FIXED_WIDTH 10
#define MIN_TEXT_WIDTH (POINTER_FIXED_WIDTH + SIMPLE_FIXED_WIDTH)

BasicCompactTip::BasicCompactTip(int TextWindowId, TipCoordinator *pTipCoordinator)
	: DialogObject(IDI_ICON_DROP),
	m_TextWindowId(TextWindowId),
	m_TipCoordinatorClient(pTipCoordinator),
	m_Alpha(TIP_ALPHA),
	m_BackgroundBitmap(),
	m_hFont(NULL),
	m_fBoldFont(false)
{
}

BasicCompactTip::~BasicCompactTip()
{
	DoBasicClose();
}

bool BasicCompactTip::DoBasicOpen(LPCTSTR UserName, int DialogId, LPCTSTR DialogTitle, LPCTSTR DialogTip, bool fSetFont, bool fBoldFont)
{
	// set font
	if(fSetFont || fBoldFont)
	{
		m_fBoldFont = fBoldFont;

		int FontHeight = 8;
		int FontWidth = 6;
		m_hFont = ::CreateFont(
			-FontHeight, -FontWidth, 0, 0, fBoldFont ? FW_BOLD : 0, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
							CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("MS Sans Serif")
							);
	}

	if(!m_BackgroundBitmap.Load(IDB_BITMAP_COMPACT_TIP_DN_LEFT))
		return false;

	HWND hDialog = OpenDialog(UserName, DialogId);
	if(hDialog == NULL)
		return false;
	SetTitle(DialogTitle);

	// open tool tips
	m_DialogToolTip.Open(hDialog, hDialog, DialogTip);

	Show(false);
	return true;
}

bool BasicCompactTip::DoBasicInit(LPCTSTR TextString, int TextOffsetWidth, int DialogResidualWidth, const RECT &rCaretRect)
{
	HWND hDialog = GetDialogHandle();
	if(!hDialog)
		return false;

	DoBasicTerm();

	if(!m_TipCoordinatorClient.Lock())
		return false;

	if(TextString == NULL || _tcslen(TextString) == 0)
		return false;

	int TextWidth;
	if(!ModifyTextBox(TextString, TextOffsetWidth, TextWidth))
		return false;

	RECT DialogRect;
	if(!CalculateRectAndType(rCaretRect, TextWidth, DialogResidualWidth, DialogRect))
		return false;

	m_Alpha = IsMouseInRect(DialogRect) ? TIP_IN_FOCUS_ALPHA : TIP_ALPHA;
	::SetLayeredWindowAttributes(hDialog, COLORKEY, (BYTE)m_Alpha, LWA_COLORKEY | LWA_ALPHA);

	::SetWindowPos(
		hDialog,
		HWND_TOPMOST,
		DialogRect.left,
		DialogRect.top,
		DialogRect.right - DialogRect.left,
		DialogRect.bottom - DialogRect.top,
		SWP_SHOWWINDOW | SWP_NOACTIVATE
	);

	return true;
}

void BasicCompactTip::DoBasicTerm()
{
	Show(false);
	m_Alpha = TIP_ALPHA;
	m_TipCoordinatorClient.Unlock();
}

void BasicCompactTip::DoBasicClose()
{
	DoBasicTerm();
	CloseDialog();

	m_DialogToolTip.Close();

	if(m_hFont != NULL)
	{
		::DeleteObject(m_hFont);
		m_hFont = NULL;
	}
	m_fBoldFont = false;
}

bool BasicCompactTip::CalculateRectAndType(const RECT &CaretRect, int TextWidth, int DialogResidualWidth, RECT &Rect)
{
	int BitmapWidth = 0;
	int BitmapHeight = 0;
	if(!m_BackgroundBitmap.GetBitmapSizeInfo(BitmapWidth, BitmapHeight))
		return false;
	m_DialogWidth = TextWidth + DialogResidualWidth;
	int DialogHeight = BitmapHeight;

	RECT rctDesktop;
	if(!::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rctDesktop, NULL))
		return false;

	int x;
	bool fLeft;
	if(CaretRect.left + m_DialogWidth - TIP_ARROW_OFFSET < rctDesktop.right)
	{
		x = CaretRect.left - TIP_ARROW_OFFSET;
		fLeft = true;
	}
	else
	{
		x = CaretRect.left - (m_DialogWidth - TIP_ARROW_OFFSET);
		fLeft = false;
	}

	int y;
	bool fUp;
	if(CaretRect.bottom + VERT_MARGIN_TO_TEXT + DialogHeight < rctDesktop.bottom)
	{
		y = CaretRect.bottom + VERT_MARGIN_TO_TEXT;
		fUp = true;
	}
	else
	{
		y = CaretRect.top - VERT_MARGIN_TO_TEXT - DialogHeight;
		fUp = false;
	}

	// make sure the dialog is entirely in the screen
	if(x < 0)
	{
		x = 0;
	}
	if(x + m_DialogWidth > rctDesktop.right)
	{
		x = rctDesktop.right - m_DialogWidth;
	}
	if(y < 0)
	{
		y = 0;
	}
	if(y + DialogHeight > rctDesktop.bottom)
	{
		y = rctDesktop.bottom - DialogHeight;
	}
	::SetRect(&Rect, x, y, x + m_DialogWidth, y + DialogHeight);

	// Initialize background bitmap acccording to resources
	int BitmapId;
	if(fLeft)
	{
		m_LeftFixedMargin = POINTER_FIXED_WIDTH;
		m_RightFixedMargin = SIMPLE_FIXED_WIDTH;

		if(fUp)
		{
			BitmapId = IDB_BITMAP_COMPACT_TIP_UP_LEFT;
		}
		else
		{
			BitmapId = IDB_BITMAP_COMPACT_TIP_DN_LEFT;
		}
	}
	else
	{
		m_LeftFixedMargin = SIMPLE_FIXED_WIDTH;
		m_RightFixedMargin = POINTER_FIXED_WIDTH;

		if(fUp)
		{
			BitmapId = IDB_BITMAP_COMPACT_TIP_UP_RIGHT;
		}
		else
		{
			BitmapId = IDB_BITMAP_COMPACT_TIP_DN_RIGHT;
		}
	}
	if(!m_BackgroundBitmap.Load(BitmapId))
		return false;

	return true;
}

bool BasicCompactTip::ModifyTextBox(LPCTSTR Text, int TextX, int &rTextWidth)
{
	HWND hTextWindow = ::GetDlgItem(GetDialogHandle(), m_TextWindowId);
	HDC hDC = ::GetDC(hTextWindow);
	if(hDC == NULL)
		return false;

	TEXTMETRIC tm; 
	::GetTextMetrics(hDC, &tm); 

	// get text size
	SIZE Size;
	bool fSuccess = (::GetTextExtentPoint32(hDC, Text, (int)_tcslen(Text), &Size) != FALSE);
	::ReleaseDC(hTextWindow, hDC);
	if(!fSuccess)
		return false;
	rTextWidth = Size.cx - tm.tmOverhang;
	rTextWidth = max(rTextWidth, MIN_TEXT_WIDTH);
	if(m_fBoldFont)
	{
		rTextWidth = (int)((double)rTextWidth * 1.25);
	}

	// set text
	::SetWindowText(hTextWindow, Text);

	// set window placement
	WINDOWPLACEMENT WindowPlacement;
	WindowPlacement.length = sizeof(WindowPlacement);
	::GetWindowPlacement(hTextWindow, &WindowPlacement);
	WindowPlacement.rcNormalPosition.left = TextX;
	WindowPlacement.rcNormalPosition.right = WindowPlacement.rcNormalPosition.left + rTextWidth;
	::SetWindowPlacement(hTextWindow, &WindowPlacement);

	return true;
}

bool BasicCompactTip::IsMouseInRect(const RECT &rRect) const
{
	CURSORINFO Info;
	Info.cbSize = sizeof(Info);
	if(!::GetCursorInfo(&Info))
		return false;

	return IsPointInRect(Info.ptScreenPos.x, Info.ptScreenPos.y, rRect);
}

bool BasicCompactTip::TipShouldBeOpaque(HWND hwnd) const
{
	if(hwnd == NULL)
		return false;
	RECT DlgRect;
	if(!::GetWindowRect(hwnd, &DlgRect))
		return false;

	// return true either if mouse is on tip, or the keyboard focus is on the tip
	if(IsMouseInRect(DlgRect))
		return true;

	FocusInfo _FocusInfo;
	bool fFocusChanged;
	if(_FocusInfo.UpdateCurrentInfo(false, fFocusChanged))
	{
		if(_FocusInfo.QueryCurrentFocusWindow() == ::GetDlgItem(GetDialogHandle(), m_TextWindowId))
			return true;
	}
	return false;
}

void BasicCompactTip::DialogInit(HWND hwndDlg)
{
	::SetWindowLong(hwndDlg, GWL_EXSTYLE, ::GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_LAYERED);
}

LRESULT BasicCompactTip::WindowProc(HWND UNUSED(hwnd), UINT uMsg, WPARAM wParam, LPARAM UNUSED(lParam), bool &fSkipDefault)
{
	fSkipDefault = false;
	LRESULT lResult = 0;

	switch(uMsg)
	{
	case WM_ERASEBKGND:
		DrawBackground((HDC)wParam);
		fSkipDefault = true;
		lResult = FALSE;
		break;
		break;
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORDLG:
		::SetBkMode((HDC)wParam, TRANSPARENT);
		if(m_hFont != NULL)
		{
			::SelectObject((HDC)wParam, m_hFont);
		}
		fSkipDefault = true;
		lResult = (LRESULT)::GetStockObject(NULL_BRUSH);
		break;
	}
	return lResult;
}

bool BasicCompactTip::DrawBackground(HDC hTargetDC)
{
	POINT TopLeftEdge;
	TopLeftEdge.x = m_LeftFixedMargin;
	TopLeftEdge.y = 0;

	POINT BottomRightEdge;
	BottomRightEdge.x = m_RightFixedMargin;
	BottomRightEdge.y = 0;

	int BitmapWidth;
	int BitmapHeight;
	if(!m_BackgroundBitmap.GetBitmapSizeInfo(BitmapWidth, BitmapHeight))
		return false;

	RECT TargetRect;
	SetRect(&TargetRect, 0, 0, m_DialogWidth, BitmapHeight);

	if(!m_BackgroundBitmap.StretchDrawKeepEdges(hTargetDC, TargetRect, TopLeftEdge, BottomRightEdge))
		return false;

	return true;
}
