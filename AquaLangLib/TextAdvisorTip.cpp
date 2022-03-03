#include "Pch.h"
#include "TextAdvisorTip.h"
#include "ButtonBitmapHandler.h"
#include "ICompactTipResult.h"
#include "FocusInfo.h"
#include "ClipboardAccess.h"
#include "StringConvert.h"
#include "resource.h"
#include "Log.h"

#define TIP_POLLING_TIME 100
#define TEXT_X_OFFSET 6

enum
{
	WINDOW_POLLING_TIMER_ID = 1,
};

TextAdvisorTip::TextAdvisorTip(ICompactTipResult &rTipResult, TipCoordinator &rTipCoordinator)
	: BasicCompactTip(IDC_STATIC_TEXT_ADVISOR, &rTipCoordinator),
	m_rTipResult(rTipResult),
	m_fActive(false),
	m_pCloseButton(NULL)
{
}

TextAdvisorTip::~TextAdvisorTip()
{
	Close();
}

bool TextAdvisorTip::HasWindow(HWND hWindow) const
{
	HWND hDialog = GetDialogHandle();
	if(!hDialog)
		return false;
	return (hWindow == hDialog || ::GetParent(hWindow) == hDialog);
}

bool TextAdvisorTip::Open(LPCTSTR UserName)
{
	if(!DoBasicOpen(UserName, IDD_DIALOG_TEXT_ADVISOR_TIP, _T("AquaLang Text Convert Advisor Tip"), _T("click on mouse or double-click on SHIFT"), false, false))
		return false;

	HWND hButtonWnd = ::GetDlgItem(GetDialogHandle(), IDC_BUTTON_CLOSE_TEXT_ADVISOR);
	if(hButtonWnd == NULL)
		return false;
	m_pCloseButton = new ButtonBitmapHandler(_T("BUTTON_BITMAP_CLOSE_TEXT_ADVISOR"), hButtonWnd);
	if(m_pCloseButton == NULL)
		return false;

	return true;
}

bool TextAdvisorTip::Init(const RECT &rCaretRect, LPCTSTR TypedText, LPCTSTR RemainingText)
{
	Term();

	m_TypedText = TypedText;
	m_RemainingText = RemainingText;

	_tstring TotalText;
	if(m_TypedText.length() > 0)
	{
		TotalText += _T("[");
		TotalText += m_TypedText;
		TotalText += _T("]");
	}
	TotalText += m_RemainingText;
	
	if(!DoBasicInit(TotalText.c_str(), TEXT_X_OFFSET, TEXT_X_OFFSET * 2, rCaretRect))
		return false;
		
	StartTimer(WINDOW_POLLING_TIMER_ID, TIP_POLLING_TIME);

	m_fActive = true;

	return true;
}

void TextAdvisorTip::Term()
{
	m_fActive = false;
	StopTimer(WINDOW_POLLING_TIMER_ID);
	DoBasicTerm();
}

void TextAdvisorTip::Close()
{
	Term();
	DoBasicClose();

	if(m_pCloseButton)
	{
		delete m_pCloseButton;
		m_pCloseButton = NULL;
	}
}

bool TextAdvisorTip::ModifyTextBox(LPCTSTR Text, int TextX, int &rTextWidth)
{
	if(!BasicCompactTip::ModifyTextBox(Text, TextX, rTextWidth))
		return false;

	// set close box
	HWND hButtonWindow = ::GetDlgItem(GetDialogHandle(), IDC_BUTTON_CLOSE_TEXT_ADVISOR);
	if(hButtonWindow == NULL)
		return false;

	WINDOWPLACEMENT WindowPlacement;
	WindowPlacement.length = sizeof(WindowPlacement);
	::GetWindowPlacement(hButtonWindow, &WindowPlacement);
	int ButtonWidth = WindowPlacement.rcNormalPosition.right - WindowPlacement.rcNormalPosition.left;
	WindowPlacement.rcNormalPosition.left = TextX + rTextWidth;
	WindowPlacement.rcNormalPosition.right = WindowPlacement.rcNormalPosition.left + ButtonWidth;
	::SetWindowPlacement(hButtonWindow, &WindowPlacement);

	rTextWidth += ButtonWidth;

	return true;
}

LRESULT TextAdvisorTip::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault)
{
	fSkipDefault = false;
	LRESULT lResult = TRUE;

	switch(uMsg)
	{
	case WM_TIMER:
		m_Alpha = TipShouldBeOpaque(hwnd) ? TIP_IN_FOCUS_ALPHA : TIP_ALPHA;
		::SetLayeredWindowAttributes(hwnd, COLORKEY, (BYTE)m_Alpha, LWA_COLORKEY | LWA_ALPHA);
		break;
	case WM_LBUTTONDOWN:
		m_rTipResult.TipShouldClose(COMPACT_TIP_OP_APPLY);
		break;
	case WM_COMMAND:
		if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CLOSE_TEXT_ADVISOR))
		{
			m_rTipResult.TipShouldClose(COMPACT_TIP_OP_CLOSE);
		}
		break;
	case WM_DRAWITEM:
		if(wParam == IDC_BUTTON_CLOSE_TEXT_ADVISOR)
		{
			DRAWITEMSTRUCT *pDrawItemStruct = (DRAWITEMSTRUCT *)lParam;
			if(pDrawItemStruct != NULL && m_pCloseButton != NULL && m_pCloseButton->Draw(*pDrawItemStruct))
			{
				fSkipDefault = true;
				lResult = TRUE;
			}
		}
		break;
	case WM_SETCURSOR:
		if(m_pCloseButton != NULL)
		{
			m_pCloseButton->HighLight((HWND)wParam == ::GetDlgItem(hwnd, IDC_BUTTON_CLOSE_TEXT_ADVISOR));
		}
		break;
	}
	if(fSkipDefault)
		return lResult;

	return BasicCompactTip::WindowProc(hwnd, uMsg, wParam, lParam, fSkipDefault);
}
