#include "Pch.h"
#include "CompactTip.h"
#include "ICompactTipResult.h"
#include "FocusInfo.h"
#include "resource.h"
#include "Log.h"

#define DEFAULT_TIP_DURATION_MSEC 3000
#define OUT_OF_FOCUS_DURATION_MSEC 500
#define TIP_POLLING_TIME 100
#define TIP_FADE_OUT_DURATION_MSEC 500
#define TIP_FADE_OUT_STEPS 20


#define WATER_DROP_X 2
#define POINTER_FIXED_WIDTH 20
#define SIMPLE_FIXED_WIDTH 10

#define MIN_TEXT_WIDTH (POINTER_FIXED_WIDTH + SIMPLE_FIXED_WIDTH)

enum
{
	WINDOW_POLLING_TIMER_ID = 1,
};

CompactTip::CompactTip(ICompactTipResult &rTipResult, int TextWindowId, TipCoordinator *pTipCoordinator)
	: BasicCompactTip(TextWindowId, pTipCoordinator),
	m_rCompactTipResult(rTipResult),
	m_fFading(false),
	m_nTimerCount(0),
	m_ApearanceDurationInMsec(DEFAULT_TIP_DURATION_MSEC),
	m_WaterDropIconBitmap()
{
}

CompactTip::~CompactTip()
{
	DoClose();
}

bool CompactTip::DoOpen(LPCTSTR UserName, int ApearanceDurationIn100msec, int DialogId, LPCTSTR DialogTitle, LPCTSTR DialogTip, bool fBold)
{
	m_ApearanceDurationInMsec = ApearanceDurationIn100msec * 100;

	if(!m_WaterDropIconBitmap.Load(IDB_BITMAP_WATER_DROP_ICON))
		return false;
	m_WaterDropIconBitmap.UseColorKey(RGB(0, 0, 255));


	return DoBasicOpen(UserName, DialogId, DialogTitle, DialogTip, fBold, fBold);
}

bool CompactTip::DoInit(LPCTSTR TextString, const RECT &rCaretRect)
{
	DoTerm();

	int WaterDropWidth;
	int WaterDropHeight;
	if(!m_WaterDropIconBitmap.GetBitmapSizeInfo(WaterDropWidth, WaterDropHeight))
		return false;

	if(!DoBasicInit(TextString, WaterDropWidth + 2 * WATER_DROP_X, POINTER_FIXED_WIDTH + SIMPLE_FIXED_WIDTH, rCaretRect))
		return false;
		
	m_nTimerCount = m_ApearanceDurationInMsec / TIP_POLLING_TIME;
	StartTimer(WINDOW_POLLING_TIMER_ID, TIP_POLLING_TIME);

	return true;
}

void CompactTip::DoTerm()
{
	StopTimer(WINDOW_POLLING_TIMER_ID);
	m_fFading = false;
	m_nTimerCount = 0;
	DoBasicTerm();
}

void CompactTip::DoClose()
{
	DoTerm();
	DoBasicClose();
}

void CompactTip::TipShouldClose(int Operation)
{
	m_rCompactTipResult.TipShouldClose(Operation);
}

bool CompactTip::CalculateRectAndType(const RECT &CaretRect, int TextWidth, int DialogResidualWidth, RECT &Rect)
{
	if(!BasicCompactTip::CalculateRectAndType(CaretRect, TextWidth, DialogResidualWidth, Rect))
		return false;

	int DialogHeight = Rect.bottom - Rect.top;
	int WaterDropWidth;
	int WaterDropHeight;
	if(!m_WaterDropIconBitmap.GetBitmapSizeInfo(WaterDropWidth, WaterDropHeight))
		return false;
	m_WaterDropRectInDialog.left = WATER_DROP_X;
	m_WaterDropRectInDialog.top = (DialogHeight - WaterDropHeight) / 2;
	m_WaterDropRectInDialog.right = m_WaterDropRectInDialog.left + WaterDropWidth;
	m_WaterDropRectInDialog.bottom = m_WaterDropRectInDialog.top + WaterDropHeight;

	return true;
}

LRESULT CompactTip::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault)
{
	switch(uMsg)
	{
	case WM_TIMER:
		if(TipShouldBeOpaque(hwnd))
		{
			m_fFading = false;
			m_nTimerCount = OUT_OF_FOCUS_DURATION_MSEC / TIP_POLLING_TIME;
			m_Alpha = TIP_IN_FOCUS_ALPHA;
			StartTimer(WINDOW_POLLING_TIMER_ID, TIP_POLLING_TIME);
			::SetLayeredWindowAttributes(hwnd, COLORKEY, (BYTE)m_Alpha, LWA_COLORKEY | LWA_ALPHA);
		}
		else
		{
			if(m_fFading)
			{
				m_nTimerCount++;

				if(m_nTimerCount >= TIP_FADE_OUT_STEPS)
				{
					m_rCompactTipResult.TipShouldClose(COMPACT_TIP_OP_CLOSE);
				}
				else
				{
					int Alpha = m_Alpha - (m_Alpha * m_nTimerCount) / TIP_FADE_OUT_STEPS;
					::SetLayeredWindowAttributes(hwnd, COLORKEY, (BYTE)Alpha, LWA_COLORKEY | LWA_ALPHA);
				}
			}
			else
			{
				m_nTimerCount--;
				if(m_nTimerCount == 0)
				{
					m_fFading = true;
					StartTimer(WINDOW_POLLING_TIMER_ID, TIP_FADE_OUT_DURATION_MSEC / TIP_FADE_OUT_STEPS);
				}
			}
		}
		break;
	case WM_LBUTTONDOWN:
		m_rCompactTipResult.TipShouldClose(COMPACT_TIP_OP_APPLY);
		break;
	}
	return BasicCompactTip::WindowProc(hwnd, uMsg, wParam, lParam, fSkipDefault);
}

bool CompactTip::DrawBackground(HDC hTargetDC)
{
	if(!BasicCompactTip::DrawBackground(hTargetDC))
		return false;

	m_WaterDropIconBitmap.Draw(hTargetDC, m_WaterDropRectInDialog);
	return true;
}
