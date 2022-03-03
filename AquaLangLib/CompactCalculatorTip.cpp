#include "Pch.h"
#include "CompactCalculatorTip.h"
#include "resource.h"

CompactCalculatorTip::CompactCalculatorTip(ICompactTipResult &rTipResult, TipCoordinator &rTipCoordinator)
	: CompactTip(rTipResult, IDC_EDIT_COMPACT_CALC_RESULT, &rTipCoordinator)
{
	SetRect(&m_RecentCaretRect, 0, 0, 0, 0);
}

CompactCalculatorTip::~CompactCalculatorTip()
{
	Close();
}

bool CompactCalculatorTip::Open(LPCTSTR UserName, int ApearanceDurationIn100msec)
{
	if(!DoOpen(UserName, ApearanceDurationIn100msec, IDD_DIALOG_COMPACT_CALCULATOR_TIP, _T("AquaLang Calculator Tip"), _T("Click to edit expression"), true))
		return false;
	HWND hDialog = GetDialogHandle();
	m_ResultEditToolTip.Open(hDialog, ::GetDlgItem(hDialog, IDC_EDIT_COMPACT_CALC_RESULT), _T("Mark result and copy"));
	return true;
}

void CompactCalculatorTip::SetCaretRect(const RECT &rCaretRect)
{
	m_RecentCaretRect = rCaretRect;
}

bool CompactCalculatorTip::Init(LPCTSTR Result)
{
	if(Result != NULL)
	{
		m_ResultString = Result;
	}
	return DoInit(m_ResultString.c_str(), m_RecentCaretRect);
}

void CompactCalculatorTip::StoreResult(LPCTSTR Result)
{
	if(Result != NULL)
	{
		m_ResultString = Result;
	}
}

void CompactCalculatorTip::Term()
{
	DoTerm();
}

void CompactCalculatorTip::Close()
{
	m_ResultEditToolTip.Close();
	DoClose();
}
