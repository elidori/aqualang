#include "Pch.h"
#include "GuessLanguageTip.h"
#include "resource.h"
#include "WinHookResult.h"
#include "AsyncHookDispatcher.h"
#include "ICompactTipResult.h"

GuessLanguageTip::GuessLanguageTip(
							ICompactTipResult &rTipResult, 
							TipCoordinator &rTipCoordinator,
							AsyncHookDispatcher &rAsyncHookDispatcher
							)
	: CompactTip(rTipResult, IDC_STATIC_GUESS_LANGUAGE_PROPORSAL, &rTipCoordinator),
	m_rAsyncHookDispatcher(rAsyncHookDispatcher)
{
}

GuessLanguageTip::~GuessLanguageTip()
{
	Close();
}

bool GuessLanguageTip::Open(LPCTSTR UserName, int ApearanceDurationIn100msec)
{
	return DoOpen(UserName, ApearanceDurationIn100msec, IDD_DIALOG_GUESS_LANGUAGE_TIP, _T("AquaLang Guess Language Tip"), _T("click on mouse or double press SHIFT to replace text and language"), true);
}

bool GuessLanguageTip::Init(LPCTSTR TextString, const RECT &rCaretRect)
{
	RegisterOnMouseClicks();
	return DoInit(TextString, rCaretRect);
}

void GuessLanguageTip::Term()
{
	UnregisterOnMouseClicks();
	DoTerm();
}

void GuessLanguageTip::Close()
{
	DoClose();
}

LRESULT GuessLanguageTip::OnMouseHookEvent(HWND UNUSED(hwnd), UINT UNUSED(uMsg), int x, int y)
{
	RECT DlgRect;
	if(::GetWindowRect(GetDialogHandle(), &DlgRect))
	{
		if(!IsPointInRect(x, y, DlgRect))
		{
			TipShouldClose(COMPACT_TIP_OP_CLOSE);
		}
	}

	return TRUE;
}

bool GuessLanguageTip::RegisterOnMouseClicks()
{
	for(int i = WM_MOUSEFIRST; i < WM_MOUSELAST; i++)
	{
		if(i != WM_MOUSEMOVE && i != WM_MOUSEWHEEL)
		{
			Result res = m_rAsyncHookDispatcher.RegisterOnMouseEvent(i, *this);
			if(res != Result_Success)
				return false;
		}
	}
	return true;
}

void GuessLanguageTip::UnregisterOnMouseClicks()
{
	m_rAsyncHookDispatcher.UnregisterOnMouseEvents(*this);
}