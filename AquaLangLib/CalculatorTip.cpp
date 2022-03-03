#include "Pch.h"
#include "CalculatorTip.h"
#include "resource.h"
#include "CompactCalculatorTip.h"
#include "ICalcTipResult.h"
#include "ButtonBitmapHandler.h"
#include "DialogTemplateParser.h"
#include "KeyboardSequence.h"
#include "ScreenFitter.h"
#include "Log.h"

#define CALC_TIP_TEXT_COLOR (RGB(200, 200, 200))

#define COLORKEY (RGB(0, 0, 255))

#define COLOR_TEXT_NORMAL RGB(147, 164, 228)
#define COLOR_TEXT_PRESSED RGB(255, 255, 255)
#define COLOR_TEXT_DISABLED RGB(64, 64, 64)
#define COLOR_TEXT_HIGHLIGHTED RGB(255, 255, 255)

#define COLOR_FN_TEXT_NORMAL RGB(0, 179, 179)
#define COLOR_FN_TEXT_PRESSED RGB(0, 179, 179)
#define COLOR_FN_TEXT_DISABLED RGB(0, 179, 179)
#define COLOR_FN_TEXT_HIGHLIGHTED RGB(0, 179, 179)

#define COLOR_STATIC_TEXT RGB(147, 164, 228)

#define VERT_MARGIN_TO_TEXT 6

#define MAX_RESULT_LENGTH 1024

extern HMODULE g_hModule;

#pragma warning (disable:4355)

CalculatorTip::CalculatorTip(ICalcTipResult &rCalcTipResult, int MaxExpressionLength)
	: DialogObject(IDI_ICON_DROP, false),
	m_rCalcTipResult(rCalcTipResult),
	m_MaxExpressionLength(MaxExpressionLength),
	m_fInIndependantMode(false),
	m_ExpressionTextContainer(NULL),
	m_ExpressionEditBox(this),
	m_ResultEditBox(this)
{
	SetRect(&m_CaretRect, 0, 0, 0, 0);
	SetRect(&m_RecentWindowRect, 0, 0, 0, 0);
}

CalculatorTip::~CalculatorTip()
{
	Close();
}

bool CalculatorTip::Open(LPCTSTR UserName)
{
	if(!SetDialogBitmap(IDB_BITMAP_CALC_TIP, false))
		return false;

	if(m_ExpressionTextContainer == NULL)
	{
		m_ExpressionTextContainer = new TCHAR[m_MaxExpressionLength];
		if(m_ExpressionTextContainer == NULL)
			return false;
	}

	HWND hDialog = OpenDialog(UserName, IDD_DIALOG_CALCULATOR_TIP);
	if(hDialog == NULL)
		return false;
	RECT r;
	::GetClientRect(hDialog, &r);
	SetTitle(_T("AquaLang Calc"));
//	ResizeDialogToMatchDialogUnits(hDialog);
//	ScaleDialogComponents(hDialog, 1, 1);

	::GetWindowRect(hDialog, &m_RecentWindowRect);

	// start with degrees, not radians
	m_rCalcTipResult.AngleUnitModeChanged(true);
	::ShowWindow(::GetDlgItem(hDialog, IDC_BUTTON_CALC_RAD), SW_HIDE);
	::ShowWindow(::GetDlgItem(hDialog, IDC_BUTTON_CALC_DEG), SW_SHOW);

	m_ButtonBitmapList.SetDialogHandle(hDialog);

	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DIG0, _T("BUTTON_BITMAP_CALC"), _T("0"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DIG1, _T("BUTTON_BITMAP_CALC"), _T("1"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DIG2, _T("BUTTON_BITMAP_CALC"), _T("2"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DIG3, _T("BUTTON_BITMAP_CALC"), _T("3"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DIG4, _T("BUTTON_BITMAP_CALC"), _T("4"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DIG5, _T("BUTTON_BITMAP_CALC"), _T("5"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DIG6, _T("BUTTON_BITMAP_CALC"), _T("6"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DIG7, _T("BUTTON_BITMAP_CALC"), _T("7"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DIG8, _T("BUTTON_BITMAP_CALC"), _T("8"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DIG9, _T("BUTTON_BITMAP_CALC"), _T("9"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DOT, _T("BUTTON_BITMAP_CALC"), _T("."), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_BRACKET_OPEN, _T("BUTTON_BITMAP_CALC"), _T("("), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_BRACKET_CLOSE, _T("BUTTON_BITMAP_CALC"), _T(")"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_PLUS, _T("BUTTON_BITMAP_CALC"), _T("+"), COLOR_FN_TEXT_NORMAL, COLOR_FN_TEXT_PRESSED, COLOR_FN_TEXT_DISABLED, COLOR_FN_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_MINUS, _T("BUTTON_BITMAP_CALC"), _T("-"), COLOR_FN_TEXT_NORMAL, COLOR_FN_TEXT_PRESSED, COLOR_FN_TEXT_DISABLED, COLOR_FN_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_MULTIPLY, _T("BUTTON_BITMAP_CALC"), _T("x"), COLOR_FN_TEXT_NORMAL, COLOR_FN_TEXT_PRESSED, COLOR_FN_TEXT_DISABLED, COLOR_FN_TEXT_HIGHLIGHTED);
//	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DIVIDE, _T("BUTTON_BITMAP_CALC_DIVIDE"));
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DIVIDE, _T("BUTTON_BITMAP_CALC"), _T("/"), COLOR_FN_TEXT_NORMAL, COLOR_FN_TEXT_PRESSED, COLOR_FN_TEXT_DISABLED, COLOR_FN_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_POWER, _T("BUTTON_BITMAP_CALC"), _T("^"), COLOR_FN_TEXT_NORMAL, COLOR_FN_TEXT_PRESSED, COLOR_FN_TEXT_DISABLED, COLOR_FN_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_1_DIV_X, _T("BUTTON_BITMAP_CALC"), _T("1/x"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_PI, _T("BUTTON_BITMAP_CALC"), _T("PI"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_EXPONENT, _T("BUTTON_BITMAP_CALC"), _T("[e]"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_CLEAR, _T("BUTTON_BITMAP_CALC"), _T("C"), COLOR_FN_TEXT_NORMAL, COLOR_FN_TEXT_PRESSED, COLOR_FN_TEXT_DISABLED, COLOR_FN_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_DEG, _T("BUTTON_BITMAP_CALC_DEGRAD"), _T("DEG"), COLOR_FN_TEXT_NORMAL, COLOR_FN_TEXT_PRESSED, COLOR_FN_TEXT_DISABLED, COLOR_FN_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_RAD, _T("BUTTON_BITMAP_CALC_DEGRAD"), _T("RAD"), COLOR_FN_TEXT_NORMAL, COLOR_FN_TEXT_PRESSED, COLOR_FN_TEXT_DISABLED, COLOR_FN_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_SIN, _T("BUTTON_BITMAP_CALC"), _T("sin"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_COS, _T("BUTTON_BITMAP_CALC"), _T("cos"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_TAN, _T("BUTTON_BITMAP_CALC"), _T("tan"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_LAN, _T("BUTTON_BITMAP_CALC"), _T("ln"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_INV, _T("BUTTON_BITMAP_CALC"), _T("inv"), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED, true);
	m_ButtonBitmapList.Add(IDC_BUTTON_EQUAL_SIGN, _T("BUTTON_BITMAP_CALC_STATIC"), _T("="), COLOR_STATIC_TEXT, COLOR_STATIC_TEXT, COLOR_STATIC_TEXT, COLOR_STATIC_TEXT);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_ASSIGN, _T("BUTTON_BITMAP_CALC"), _T("="), COLOR_TEXT_NORMAL, COLOR_TEXT_PRESSED, COLOR_TEXT_DISABLED, COLOR_TEXT_HIGHLIGHTED);
	m_ButtonBitmapList.Add(IDC_BUTTON_CALC_REDUCE, _T("BUTTON_BITMAP_CALC_REDUCE"));

	m_ExpressionEditBox.Open(::GetDlgItem(hDialog, IDC_EDIT_CALC_EXPRESSION));
	m_ResultEditBox.Open(::GetDlgItem(hDialog, IDC_EDIT_CALC_RESULT));

	m_ButtonReduceTip.Open(hDialog, ::GetDlgItem(hDialog, IDC_BUTTON_CALC_REDUCE), _T("Reduce back to result tip"));
	m_ButtonAssignTip.Open(hDialog, ::GetDlgItem(hDialog, IDC_BUTTON_CALC_ASSIGN), _T("Copy result value to expression"));

	return true;
}

bool CalculatorTip::IsOpen() const
{
	return IsDialogOpen();
}

void CalculatorTip::SetCaretRect(const RECT &rCaretRect)
{
	m_CaretRect = rCaretRect;
	m_CaretRect.top -= VERT_MARGIN_TO_TEXT;
	m_CaretRect.bottom += VERT_MARGIN_TO_TEXT;
}

void CalculatorTip::SetFocus()
{
	if(IsVisible())
	{
		// the focus should be at the expression edit box. This is because it is intentionaly appearing as the first
		// control in the resources list.
		::SetFocus(GetDialogHandle());
	}
}

bool CalculatorTip::Init(LPCTSTR Expression, LPCTSTR Result, bool fIndependant)
{
	HWND hDialog = GetDialogHandle();
	if(!hDialog)
		return false;

	Term();

	m_fInIndependantMode = fIndependant;

	RECT DialogRect;
	if(fIndependant)
	{
		DialogRect = m_RecentWindowRect;
	}
	else
	{
		if(!CalcDialogRect(m_CaretRect, DialogRect))
			return false;
	}

	::ShowWindow(::GetDlgItem(hDialog, IDC_BUTTON_CALC_REDUCE), fIndependant ? SW_HIDE : SW_SHOW);

	UpdateExpressionString(Expression);
	::SetWindowText(::GetDlgItem(hDialog, IDC_EDIT_CALC_RESULT), Result);
	CheckInv(false);

	::SetWindowPos(
		hDialog,
		HWND_TOPMOST,
		DialogRect.left,
		DialogRect.top,
		DialogRect.right - DialogRect.left,
		DialogRect.bottom - DialogRect.top,
		SWP_SHOWWINDOW | (fIndependant ? 0 : SWP_NOACTIVATE)
	);

	return true;
}

void CalculatorTip::Term()
{
	if(m_fInIndependantMode)
	{
		::GetWindowRect(GetDialogHandle(), &m_RecentWindowRect);
		m_fInIndependantMode = false;
	}

	Show(false);
}

bool CalculatorTip::CalcDialogRect(const RECT &CaretRect, RECT &Rect)
{
	RECT DialogRect;
	if(!::GetWindowRect(GetDialogHandle(), &DialogRect))
		return false;
	int DialogWidth = DialogRect.right - DialogRect.left;
	int DialogHeight = DialogRect.bottom - DialogRect.top;

	if(!ScreenFitter::PlaceInScreen(DialogWidth, DialogHeight, CaretRect, Rect, DialogWidth / 2))
		return false;
	return true;
}

bool CalculatorTip::IsOwnerOf(HWND hWindow)
{
	HWND hDialog = GetDialogHandle();
	if(hDialog == NULL)
		return false;
	if(::GetDlgItem(hDialog, IDC_EDIT_CALC_EXPRESSION) == hWindow)
		return true;
	if(::GetDlgItem(hDialog, IDC_EDIT_CALC_RESULT) == hWindow)
		return true;
	return false;
}

bool CalculatorTip::Reevaluate(LPCTSTR Result)
{
	return RefreshResultField(Result);
}

void CalculatorTip::Close()
{
	Term();
	CloseDialog();
	m_ButtonBitmapList.Close();
	m_ExpressionEditBox.Close();
	m_ResultEditBox.Close();
	m_ButtonReduceTip.Close();
	m_ButtonAssignTip.Close();
	if(m_ExpressionTextContainer != NULL)
	{
		delete[] m_ExpressionTextContainer;
		m_ExpressionTextContainer = NULL;
	}
}

bool CalculatorTip::ScaleDialogComponents(HWND hDialog, double xScale, double yScale)
{
	HRSRC hDlg = ::FindResource(g_hModule, MAKEINTRESOURCE(IDD_DIALOG_CALCULATOR_TIP), (LPTSTR)RT_DIALOG);
	HGLOBAL hResource = ::LoadResource(g_hModule, hDlg);
	DLGTEMPLATE* pDlg = (DLGTEMPLATE*) ::LockResource(hResource);
	if(!pDlg)
		return false;

	DialogTemplateParser _DialogTemplateParser;
	if(!_DialogTemplateParser.Open(pDlg))
		return false;

	POINT DialogClientOffset = { 0, 0 };
	if(!::ClientToScreen(hDialog, &DialogClientOffset))
		return false;

	// rescale dialog
	RECT r;
	if(!::GetClientRect(hDialog, &r))
		return false;
	r.right = r.left + (int)(((double)(r.right - r.left) + 0.5) * xScale);
	r.bottom = r.top + (int)(((double)(r.bottom - r.top) + 0.5) * yScale);
	if(!::MoveWindow(hDialog, r.left, r.top, r.right - r.left, r.bottom - r.top, FALSE))
		return false;

	// rescale dialog items
	for(index_t i = 0; i < (index_t)_DialogTemplateParser.GetItemCount(); i++)
	{
		const DialogItemParser *pItem = NULL;
		if(_DialogTemplateParser.GetItem(i, pItem))
		{
			HWND hControl = ::GetDlgItem(hDialog, pItem->GetItemId());

			POINT ClientOffset = { 0, 0 };
			if(!::ClientToScreen(hControl, &ClientOffset))
				return false;

			if(!::GetWindowRect(hControl, &r))
				return false;
			int Width = r.right - r.left;
			int Height = r.bottom - r.top;
			r.left = (int)(((double)(ClientOffset.x - DialogClientOffset.x) + 0.5) * xScale);
			r.right = r.left + (int)(((double)(Width) + 0.5) * xScale);
			r.top = (int)(((double)(ClientOffset.y - DialogClientOffset.y) + 0.5) * yScale);
			r.bottom = r.top + (int)(((double)(Height) + 0.5)* yScale);
			if(!::MoveWindow(hControl, r.left, r.top, r.right - r.left, r.bottom - r.top, FALSE))
				return false;
		}
	}
	return true;
}

void CalculatorTip::ResizeDialogToMatchDialogUnits(HWND hDialog)
{
	HRSRC hDlg = ::FindResource(g_hModule, MAKEINTRESOURCE(IDD_DIALOG_CALCULATOR_TIP), (LPTSTR)RT_DIALOG);
	HGLOBAL hResource = ::LoadResource(g_hModule, hDlg);
	DLGTEMPLATE* pDlg = (DLGTEMPLATE*) ::LockResource(hResource);
	if(pDlg)
	{
		DialogTemplateParser _DialogTemplateParser;
		if(_DialogTemplateParser.Open(pDlg))
		{
			// rescale dialog
			const RECT &dr = _DialogTemplateParser.GetDialogRect();
			::MoveWindow(hDialog, dr.left, dr.top, dr.right - dr.left, dr.bottom - dr.top, FALSE);

			// rescale dialog items
			for(index_t i = 0; i < (index_t)_DialogTemplateParser.GetItemCount(); i++)
			{
				const DialogItemParser *pItem = NULL;
				if(_DialogTemplateParser.GetItem(i, pItem))
				{
					const RECT &r = pItem->GetItemRect();
					::MoveWindow(::GetDlgItem(hDialog, pItem->GetItemId()), r.left, r.top, r.right - r.left, r.bottom - r.top, FALSE);
				}
			}
		}
	}
}

LRESULT CalculatorTip::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault)
{
	fSkipDefault = false;
	LRESULT lResult = 0;

	switch(uMsg)
	{
	case WM_COMMAND:
		if((HWND)lParam == ::GetDlgItem(hwnd, IDCANCEL))
		{
			Term();
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_REDUCE))
		{
			ReduceCalculator();
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_DIG0))
		{
			AddText(_T("0"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_DIG1))
		{
			AddText(_T("1"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_DIG2))
		{
			AddText(_T("2"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_DIG3))
		{
			AddText(_T("3"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_DIG4))
		{
			AddText(_T("4"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_DIG5))
		{
			AddText(_T("5"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_DIG6))
		{
			AddText(_T("6"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_DIG7))
		{
			AddText(_T("7"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_DIG8))
		{
			AddText(_T("8"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_DIG9))
		{
			AddText(_T("9"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_DOT))
		{
			AddText(_T("."));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_BRACKET_OPEN))
		{
			AddText(_T("("));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_BRACKET_CLOSE))
		{
			AddText(_T(")"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_PLUS))
		{
			AddText(_T("+"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_MINUS))
		{
			AddText(_T("-"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_MULTIPLY))
		{
			AddText(_T("*"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_DIVIDE))
		{
			AddText(_T("/"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_POWER))
		{
			AddText(_T("^"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_PI))
		{
			AddText(_T("PI"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_EXPONENT))
		{
			AddText(_T("[e]"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_SIN))
		{
			Execute(IsInInv() ? _T("asin") : _T("sin"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_COS))
		{
			Execute(IsInInv() ? _T("acos") : _T("cos"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_TAN))
		{
			Execute(IsInInv() ? _T("atan") : _T("tan"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_LAN))
		{
			Execute(IsInInv() ? _T("[e]^") : _T("log"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_1_DIV_X))
		{
			Execute(_T("1/"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_DEG))
		{
			::ShowWindow(::GetDlgItem(hwnd, IDC_BUTTON_CALC_DEG), SW_HIDE);
			::ShowWindow(::GetDlgItem(hwnd, IDC_BUTTON_CALC_RAD), SW_SHOW);
			m_rCalcTipResult.AngleUnitModeChanged(false); // move to radians
			UpdateExpressionString(NULL); // put focus
			Evaluate();
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_RAD))
		{
			::ShowWindow(::GetDlgItem(hwnd, IDC_BUTTON_CALC_RAD), SW_HIDE);
			::ShowWindow(::GetDlgItem(hwnd, IDC_BUTTON_CALC_DEG), SW_SHOW);
			m_rCalcTipResult.AngleUnitModeChanged(true); // move to degrees
			UpdateExpressionString(NULL); // put focus
			Evaluate();
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_CLEAR))
		{
			UpdateExpressionString(_T(""));
			CheckInv(false);
			::SetWindowText(::GetDlgItem(hwnd, IDC_EDIT_CALC_RESULT), _T("0"));
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_ASSIGN))
		{
			CopyResultToExpression();
			CheckInv(false);
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CALC_INV))
		{
			CheckInv(!IsInInv());
			UpdateExpressionString(NULL); // put focus
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_EDIT_CALC_EXPRESSION))
		{
			if(HIWORD(wParam) == EN_UPDATE)
			{
				Evaluate();
			}
		}
		break;
	case WM_DRAWITEM:
		if(m_ButtonBitmapList.Draw((int)wParam, (DRAWITEMSTRUCT *)lParam))
		{
			fSkipDefault = true;
			lResult = TRUE;
		}
		break;
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLOREDIT:
	    ::SetBkMode((HDC)wParam, TRANSPARENT);
		::SetTextColor((HDC)wParam, CALC_TIP_TEXT_COLOR);
		fSkipDefault = true;
		lResult = (LRESULT)::GetStockObject(NULL_BRUSH);
		break;
	case WM_SETCURSOR:
		m_ButtonBitmapList.Highlight((HWND)wParam);
		break;
	}
	return lResult;
}

void CalculatorTip::CheckInv(bool fCheck)
{
	if(m_ButtonBitmapList.Check(IDC_BUTTON_CALC_INV, fCheck))
	{
		::InvalidateRect(::GetDlgItem(GetDialogHandle(), IDC_BUTTON_CALC_INV), NULL, FALSE);
	}
}

bool CalculatorTip::IsInInv()
{
	return m_ButtonBitmapList.GetChecked(IDC_BUTTON_CALC_INV);
}

bool CalculatorTip::AddText(LPCTSTR Text)
{
	CheckInv(false);

	HWND hExpressionWindow = ::GetDlgItem(GetDialogHandle(), IDC_EDIT_CALC_EXPRESSION);
	if(hExpressionWindow == NULL)
		return false;

	int TextLength = ::GetWindowTextLength(hExpressionWindow);
	if(TextLength + _tcslen(Text) >= (DWORD)m_MaxExpressionLength)
		return false;

	::GetWindowText(hExpressionWindow, m_ExpressionTextContainer, m_MaxExpressionLength);
	_tcscat_s(m_ExpressionTextContainer, m_MaxExpressionLength, Text);

	if(!UpdateExpressionString(m_ExpressionTextContainer))
		return false;

	// update result text
	if(!Evaluate())
		return false;
	return true;
}

bool CalculatorTip::Evaluate()
{
	HWND hExpressionWindow = ::GetDlgItem(GetDialogHandle(), IDC_EDIT_CALC_EXPRESSION);
	if(hExpressionWindow == NULL)
		return false;

	int TextLength = ::GetWindowTextLength(hExpressionWindow);
	if(TextLength >= m_MaxExpressionLength)
		return false;

	::GetWindowText(hExpressionWindow, m_ExpressionTextContainer, m_MaxExpressionLength);
	m_rCalcTipResult.TipShouldReevaluate(m_ExpressionTextContainer);
	return true;
}

bool CalculatorTip::Execute(LPCTSTR Operator)
{
	CheckInv(false);

	HWND hExpressionWindow = ::GetDlgItem(GetDialogHandle(), IDC_EDIT_CALC_EXPRESSION);
	if(hExpressionWindow == NULL)
		return false;

	int TextLength = ::GetWindowTextLength(hExpressionWindow);
	if(TextLength + _tcslen(Operator) + 2 >= (DWORD)m_MaxExpressionLength)
		return false;

	if(::GetWindowText(hExpressionWindow, m_ExpressionTextContainer, m_MaxExpressionLength) == 0)
		return false;

	_tstring NewText = Operator;
	NewText += _T("(");
	NewText += m_ExpressionTextContainer;
	NewText += _T(")");

	if(!UpdateExpressionString(NewText.c_str()))
		return false;

	if(!Evaluate())
		return false;
	return true;
}

bool CalculatorTip::RefreshResultField(LPCTSTR Result)
{
	HWND hDialog = GetDialogHandle();
	if(hDialog == NULL)
		return false;

	if(!::SetWindowText(::GetDlgItem(hDialog, IDC_EDIT_CALC_RESULT), Result))
		return false;

	RefreshEditBoxes();
	return true;
}

void CalculatorTip::RefreshEditBoxes()
{
	HWND hDialog = GetDialogHandle();
	if(hDialog != NULL)
	{
		::InvalidateRect(hDialog, NULL, TRUE);
		::InvalidateRect(::GetDlgItem(hDialog, IDC_EDIT_CALC_RESULT), NULL, TRUE);
		::InvalidateRect(::GetDlgItem(hDialog, IDC_EDIT_CALC_EXPRESSION), NULL, TRUE);
	}
}

bool CalculatorTip::ReduceCalculator()
{
	m_rCalcTipResult.TipShouldClose(CALC_OP_REDUCE);
	return true;
}

bool CalculatorTip::UpdateExpressionString(LPCTSTR Expression)
{
	HWND hExpressionWindow = ::GetDlgItem(GetDialogHandle(), IDC_EDIT_CALC_EXPRESSION);
	if(hExpressionWindow == NULL)
		return false;
	if(Expression != NULL)
	{
		if(!::SetWindowText(hExpressionWindow, Expression))
			return false;
	}

	// put focus on expression and go to end
	::SetFocus(hExpressionWindow);
	KeyboardSequence::SendKeyboardMessage(hExpressionWindow, VK_END, true);
	KeyboardSequence::SendKeyboardMessage(hExpressionWindow, VK_END, false);

	return true;
}

bool CalculatorTip::CopyResultToExpression()
{
	HWND hResultWindow = ::GetDlgItem(GetDialogHandle(), IDC_EDIT_CALC_RESULT);
	if(hResultWindow == NULL)
		return false;
	int TextLength = ::GetWindowTextLength(hResultWindow);
	if(TextLength >= m_MaxExpressionLength)
		return false;
	TCHAR ResultText[MAX_RESULT_LENGTH];
	if(::GetWindowText(hResultWindow, ResultText, sizeof(ResultText)/sizeof(ResultText[0])) == 0)
		return false;

	if(!UpdateExpressionString(ResultText))
		return false;
	return true;
}

void CalculatorTip::NotifyKeyDown(HWND hwnd, int VirtualKey)
{
	if(VirtualKey == VK_RETURN && hwnd == ::GetDlgItem(GetDialogHandle(), IDC_EDIT_CALC_EXPRESSION))
	{
		CopyResultToExpression();
	}
	else
	{
		RefreshEditBoxes();
	}
}

void CalculatorTip::NotifyKillFocus(HWND UNUSED(hwnd))
{
	RefreshEditBoxes();
}
