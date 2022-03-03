#include "Pch.h"
#include "BalloonTip.h"
#include "TimeHelpers.h"
#include "resource.h"
#include "Log.h"

#define MAX_WINDOW_HEIGHT_FOR_TEXT_POINTING 50

enum
{
	FADE_OUT_TIMER_ID = 1,
};

BalloonTip::BalloonTip(
				int TipFadeOutDuration,
				int TipFadeOutSteps,
				int UpLeftId,
				int UpRightId,
				int DownLeftId,
				int DownRightId,
				COLORREF ColorKey /*= RGB(0, 0, 255)*/,
				COLORREF TextColor /*= RGB(0, 0, 0)*/,
				const RECT *pTextRect /*= NULL*/
				)
	: DialogObject(IDI_ICON_DROP),
	m_pListener(NULL),
	m_nTimerCount(0),
	m_TipInitialAlpha(255),
	m_TipFadeOutDuration(TipFadeOutDuration),
	m_TipFadeOutSteps(TipFadeOutSteps),
	m_UpLeftId(UpLeftId),
	m_UpRightId(UpRightId),
	m_DownLeftId(DownLeftId),
	m_DownRightId(DownRightId),
	m_ColorKey(ColorKey),
	m_TextResource(),
	m_fExit(false)
{
	if(pTextRect)
	{
		m_TextResource.SetTextColor(TextColor, true);
		m_TextResource.SetTargetRect(*pTextRect);
	}
}

BalloonTip::~BalloonTip()
{
	Close();
}

bool BalloonTip::Open()
{
	return SetDialogBitmap(m_UpRightId, true);
}

bool BalloonTip::Init(
			LPCTSTR UserName,
			LPCTSTR Title,
			LPCTSTR Text,
			const RECT *pSubjectRect,
			int DialogId,
			int TipInitialAlpha,
			int TipDurationMsec,
			bool UNUSED(fFollowMousePosition),
			ITipListener *pListener
			)
{
	Term();

	Open();

	m_TipInitialAlpha = TipInitialAlpha;
	m_TextResource.SetText(Text);
	m_pListener = pListener;

	HWND hDialog = OpenDialog(UserName, DialogId);
	if(hDialog == NULL)
		return false;
	SetTitle(Title);

	// this must be called after OpenDialog, as the placement changes there
	WINDOWPLACEMENT WindowPlacement;
	WindowPlacement.length = sizeof(WindowPlacement);
	if(!::GetWindowPlacement(hDialog, &WindowPlacement))
		return false;

	int BitmapDirection = -1;

	// set position
	RECT WinRect;
	if(pSubjectRect != NULL)
	{
		int Width = WindowPlacement.rcNormalPosition.right - WindowPlacement.rcNormalPosition.left;
		int Height = WindowPlacement.rcNormalPosition.bottom - WindowPlacement.rcNormalPosition.top;
		CalculateRectAndType(*pSubjectRect, WinRect, BitmapDirection, Width, Height);
	}
	else
	{
		// the tip dialog is initially centered. Below code is using this assumption
		WinRect = WindowPlacement.rcNormalPosition;
		BitmapDirection = m_UpRightId;
	}
	SetDialogBitmap(BitmapDirection, true);

	::SetWindowPos(
		hDialog,
        HWND_TOPMOST,
		WinRect.left,
		WinRect.top,
		WinRect.right - WinRect.left,
		WinRect.bottom - WinRect.top,
        SWP_SHOWWINDOW | SWP_NOACTIVATE
    );

	if(!StartTimer(FADE_OUT_TIMER_ID, TipDurationMsec))
		return false;

	// block until dialog exit if no external listener
	if(!m_pListener)
	{
		DWORD TimeLimit = TipDurationMsec + m_TipFadeOutDuration + 2000; // 2 seconds spare
		Timer _Timer;

		MSG msg;
		while(!m_fExit && _Timer.GetDiff() < TimeLimit)
		{
			if(::GetMessage(&msg, hDialog, NULL, NULL))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		Term();
	}

	return true;
}

void BalloonTip::Term()
{
	StopTimer(FADE_OUT_TIMER_ID);
	CloseDialog();
	m_nTimerCount = 0;
	m_TipInitialAlpha = 255;
	m_TextResource.SetText(_T(""));
	m_fExit = false;
}

void BalloonTip::Close()
{
	Term();
}

void BalloonTip::DialogInit(HWND hwndDlg)
{
	::SetWindowLong(hwndDlg, GWL_EXSTYLE, ::GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_LAYERED);
	::SetLayeredWindowAttributes(hwndDlg, m_ColorKey, (BYTE)m_TipInitialAlpha, LWA_COLORKEY | LWA_ALPHA);
}

LRESULT BalloonTip::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM UNUSED(lParam), bool &fSkipDefault)
{
	fSkipDefault = false;
	LRESULT lResult = 0;

	switch(uMsg)
	{
	case WM_ERASEBKGND:
		m_TextResource.DrawText((HDC)wParam, true);
		fSkipDefault = true;
		lResult = FALSE;
		break;
	case WM_TIMER:
		if(m_nTimerCount == 0)
		{
			StartTimer(FADE_OUT_TIMER_ID, m_TipFadeOutDuration / m_TipFadeOutSteps);
		}
		m_nTimerCount++;
		if(m_nTimerCount >= m_TipFadeOutSteps)
		{
			if(m_pListener)
			{
				m_pListener->TipShouldClose(*this);
			}
			else
			{
				m_fExit = true;
			}
		}
		else
		{
			assert(m_TipFadeOutSteps != 0);
			int Alpha = m_TipInitialAlpha - (m_TipInitialAlpha * m_nTimerCount) / m_TipFadeOutSteps;
			::SetLayeredWindowAttributes(hwnd, m_ColorKey, (BYTE)Alpha, LWA_ALPHA | LWA_COLORKEY);
		}
		break;
	case WM_LBUTTONDOWN:
		if(m_pListener)
		{
			m_pListener->TipWasClicked(*this, true);
		}
		break;
	case WM_RBUTTONDOWN:
		if(m_pListener)
		{
			m_pListener->TipWasClicked(*this, false);
		}
		break;
	}
	return lResult;
}

void BalloonTip::CalculateRectAndType(const RECT &rSubjectRect, RECT &rWinRect, int &rBitmapDirection, int Width, int Height)
{
	RECT rctDesktop;
	::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rctDesktop, NULL);

	int x;
	int y;
	if(rSubjectRect.left > Width / 2)
	{
		x = rSubjectRect.left - Width / 2;

		if(rSubjectRect.bottom + Height >= rctDesktop.bottom)
		{
			rBitmapDirection = m_DownRightId;
			y = rSubjectRect.top - Height;
		}
		else
		{
			rBitmapDirection = m_UpRightId;
			y = rSubjectRect.bottom;
		}
	}
	else
	{
		x = rSubjectRect.right + Width / 2;
		if(rSubjectRect.bottom + Height >= rctDesktop.bottom)
		{
			rBitmapDirection = m_DownLeftId;
			y = rSubjectRect.top - Height;
		}
		else
		{
			rBitmapDirection = m_UpLeftId;
			y = rSubjectRect.bottom;
		}
	}
	if(x < 0)
	{
		x = 0;
	}
	if(x + Width > rctDesktop.right)
	{
		x = rctDesktop.right - Width;
	}
	if(y < 0)
	{
		y = 0;
	}
	if(y + Height > rctDesktop.bottom)
	{
		y = rctDesktop.bottom - Height;
	}
	::SetRect(&rWinRect, x, y, x + Width, y + Height);
}
