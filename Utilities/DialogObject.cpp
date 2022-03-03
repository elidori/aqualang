#include "Pch.h"
#include "DialogObject.h"
#include "Log.h"

extern HMODULE g_hModule;

DialogObject::DialogObject(int DialogIconId /*= -1*/, bool fHideFromTaskbar /*= true*/)
	: m_fHideFromTaskbar(fHideFromTaskbar),
	m_hParent(NULL),
	m_hDialog(NULL),
	m_OldWndProc(NULL),
	m_DialogIcon(DialogIconId),
	m_fDialogFitsBitmap(false),
	m_DialogBitmap()
{
}

DialogObject::~DialogObject()
{
	CloseDialog();
}

bool DialogObject::SetDialogBitmap(int BitmapId, bool fDialogFitsBitmap)
{
	if(!m_DialogBitmap.Load(BitmapId))
		return false;
	m_fDialogFitsBitmap = fDialogFitsBitmap;
	return true;
}

HWND DialogObject::OpenDialog(LPCTSTR UserName, int DialogId, HWND hParent /*= NULL*/)
{
	if(m_hDialog != NULL)
		return NULL;

	if(m_fHideFromTaskbar && hParent == NULL)
	{
		m_hParent = CreateWindowEx(0, _T("BUTTON"), _T("AquaLang Dialog Parent"), WS_POPUP, 0,0,0,0, NULL, NULL, NULL, NULL);
		if(!m_hParent)
			return NULL;
	}

	// create dialog window
	HRSRC hDlg = ::FindResource(g_hModule, MAKEINTRESOURCE(DialogId), (LPTSTR)RT_DIALOG);
	HGLOBAL hResource = ::LoadResource(g_hModule, hDlg);
	DLGTEMPLATE* pDlg = (DLGTEMPLATE*) ::LockResource(hResource);
	if(!pDlg)
		return NULL;
	m_hDialog = CreateDialogIndirectParam(
					g_hModule,
					pDlg,
					(m_hParent != NULL) ? m_hParent : hParent,
					DialogProc,
					(LPARAM)this
					);
	if(m_hDialog == NULL)
		return NULL;

	if(hParent == NULL)
	{
		HWND hActualParent = (m_hParent == NULL) ? m_hDialog : m_hParent;
		m_DialogIcon.SetToDialog(hActualParent);
		m_WaterMark.Open(UserName, hActualParent);
	}

	// resize dialog if need to fit bitmap
	if(m_DialogBitmap.IsLoaded() && m_fDialogFitsBitmap)
	{
		if(!FitDialogRectToBitmap())
			return false;
	}

	::SetWindowLongPtr (m_hDialog, GWLP_USERDATA, (LONG_PTR)this);
	m_OldWndProc = (WNDPROC)::SetWindowLongPtr (m_hDialog, GWLP_WNDPROC, (LONG_PTR)&ThunkWindowProc);

	return m_hDialog;
}

void DialogObject::CloseDialog()
{
	if(m_OldWndProc != NULL)
	{
		::SetWindowLongPtr(m_hDialog, GWLP_WNDPROC, (LONG_PTR)m_OldWndProc);
		m_OldWndProc = NULL;
	}
	if(m_hDialog != NULL)
	{
		::DestroyWindow(m_hDialog);
		m_hDialog = NULL;
	}
	if(m_hParent != NULL)
	{
		::DestroyWindow(m_hParent);
		m_hParent = NULL;
	}
	m_WaterMark.Close();
}

void DialogObject::Show(bool fShow)
{
	if(m_hDialog != NULL)
	{
		::ShowWindow(m_hDialog, fShow ? SW_SHOW : SW_HIDE);
	}
}

bool DialogObject::StartTimer(int nTimerId, DWORD dwTimeout)
{
	if(m_hDialog == NULL || nTimerId == 0)
		return false;

	StopTimer(nTimerId);

	if(::SetTimer(m_hDialog, nTimerId, dwTimeout, NULL) == 0)
		return false;
	return true;
}

void DialogObject::StopTimer(int nTimerId)
{
	if(m_hDialog == NULL)
		return;

	if(nTimerId != 0)
	{
		::KillTimer(m_hDialog, nTimerId);
	}					
}

bool DialogObject::IsVisible() const
{
	if(m_hDialog == NULL)
		return false;
	return (::IsWindowVisible(m_hDialog) == TRUE);
}

bool DialogObject::SetTitle(LPCTSTR Title)
{
	if(m_hDialog == NULL)
		return false;
	if(!::SetWindowText(m_hDialog, Title))
		return false;
	return true;
}

bool DialogObject::IsPointInWindow(int x, int y) const
{
	if(!IsVisible())
		return false;

	WINDOWPLACEMENT WindowPlacement;
	WindowPlacement.length = sizeof(WindowPlacement);
	if(!::GetWindowPlacement(m_hDialog, &WindowPlacement))
		return false;

	// Eli Dori: assuming that the window is top most, or else it could be hidden by another window
	return IsPointInRect(x, y, WindowPlacement.rcNormalPosition);
}

bool DialogObject::IsPointInRect(int x, int y, const RECT &rRect)
{
	if(x < rRect.left || x >= rRect.right)
		return false;
	if(y < rRect.top || y >= rRect.bottom)
		return false;

	return true;
}

void DialogObject::PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_hDialog != NULL)
	{
		::PostMessage(m_hDialog, uMsg, wParam, lParam);
	}
}

bool DialogObject::GetDialogUserName(HWND hDialog, _tstring &rUserName)
{
	AquaLangWaterMark _WaterMarkFinder;
	return _WaterMarkFinder.FindMarkInWindow(hDialog, rUserName);
}

struct FindStruct
{
	LPCTSTR Title;
	LPCTSTR UserName;
	HWND hFoundWindow;
};

HWND DialogObject::FindAquaLangWindow(LPCTSTR Title, LPCTSTR UserName)
{
	FindStruct WindowFind;
	WindowFind.Title = Title;
	WindowFind.UserName = UserName;
	WindowFind.hFoundWindow = NULL;

	::EnumWindows(EnumWindowsProc, (LPARAM)&WindowFind);

	return WindowFind.hFoundWindow;
}

BOOL CALLBACK DialogObject::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	FindStruct *pWindowFind = (FindStruct *)lParam;
	if(pWindowFind == NULL)
		return FALSE;

	TCHAR Text[256];
	_tcscpy_s(Text, sizeof(Text) / sizeof(Text[0]), _T(""));
	::GetWindowText(hwnd, Text, sizeof(Text) / sizeof(Text[0]));
	if(_tcscmp(pWindowFind->Title, Text) == 0)
	{
		_tstring UserName;
		if(DialogObject::GetDialogUserName(hwnd, UserName))
		{
			if(UserName == pWindowFind->UserName)
			{
				pWindowFind->hFoundWindow = hwnd;
				return FALSE;
			}
		}
	}

	return TRUE;
}

INT_PTR CALLBACK DialogObject::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM UNUSED(wParam), LPARAM lParam)
{
	if(uMsg == WM_INITDIALOG)
	{
		DialogObject *pThis = (DialogObject *)lParam;
		if(pThis)
		{
			pThis->DialogInit(hwndDlg);
		}
	}
	return FALSE;
}

LRESULT CALLBACK DialogObject::ThunkWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DialogObject *pThis = (DialogObject *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if(pThis)
	{
		return pThis->BaseWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

LRESULT DialogObject::BaseWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	assert(hwnd == m_hDialog);
	WNDPROC OldWndProc = m_OldWndProc;

	bool fMySkipDefault = false;
	LRESULT lMyResult = MyWindowProc(hwnd, uMsg, wParam, lParam, fMySkipDefault);

	bool fSkipDefault = false;
	LRESULT lResult = WindowProc(hwnd, uMsg, wParam, lParam, fSkipDefault);

	if(fSkipDefault)
	{
		return lResult;
	}
	else if(fMySkipDefault)
	{ 
		return lMyResult;
	}

	lResult = ::CallWindowProc(OldWndProc, hwnd, uMsg, wParam, lParam);
	
	bool fOverrideDefault = false;
	LRESULT lLateResult = LateWindowProc(hwnd, uMsg, wParam, lParam, fOverrideDefault);
	if(fOverrideDefault)
	{
		return lLateResult;
	}
	return lResult;
}

LRESULT DialogObject::MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM UNUSED(lParam), bool &fMySkipDefault)
{
	fMySkipDefault = false;
	LRESULT lMyResult = 0;

	switch(uMsg)
	{
	case WM_ERASEBKGND:
		if(m_DialogBitmap.IsLoaded())
		{
			bool fSuccess = false;
			if(m_fDialogFitsBitmap)
			{
				fSuccess = m_DialogBitmap.Draw((HDC)wParam);
			}
			else
			{
				RECT DialogClientRect;
				if(::GetClientRect(hwnd, &DialogClientRect))
				{
					fSuccess = m_DialogBitmap.StretchDraw((HDC)wParam, DialogClientRect);
				}
			}
			if(fSuccess)
			{
				fMySkipDefault = true;
				lMyResult = FALSE;
			}
		}
		break;
	}

	return lMyResult;
}

bool DialogObject::FitDialogRectToBitmap()
{
	int BitmapWidth = 0;
	int BitmapHeight = 0;
	if(!m_DialogBitmap.GetBitmapSizeInfo(BitmapWidth, BitmapHeight))
		return false;

	WINDOWPLACEMENT WindowPlacement;
	WindowPlacement.length = sizeof(WindowPlacement);
	if(!::GetWindowPlacement(m_hDialog, &WindowPlacement))
		return false;
	RECT DialogClientRect;
	if(!::GetClientRect(m_hDialog, &DialogClientRect))
		return false;

	int OuterClientWidthMargin = (WindowPlacement.rcNormalPosition.right - WindowPlacement.rcNormalPosition.left) - (DialogClientRect.right - DialogClientRect.left);
	int OuterClientHeightMargin = (WindowPlacement.rcNormalPosition.bottom - WindowPlacement.rcNormalPosition.top) - (DialogClientRect.bottom - DialogClientRect.top);

	// resize and keep the center
	int DialogWidth = WindowPlacement.rcNormalPosition.right - WindowPlacement.rcNormalPosition.left;
	int DialogHeight = WindowPlacement.rcNormalPosition.bottom - WindowPlacement.rcNormalPosition.top;
	int NewWidth = BitmapWidth + OuterClientWidthMargin;
	int NewHeight = BitmapHeight + OuterClientHeightMargin;

	WindowPlacement.rcNormalPosition.left += (DialogWidth - NewWidth) / 2;
	WindowPlacement.rcNormalPosition.right = WindowPlacement.rcNormalPosition.left + NewWidth;
	WindowPlacement.rcNormalPosition.top += (DialogHeight - NewHeight) / 2;
	WindowPlacement.rcNormalPosition.bottom = WindowPlacement.rcNormalPosition.top + NewHeight;
	WindowPlacement.showCmd = SW_HIDE;
	if(!::SetWindowPlacement(m_hDialog, &WindowPlacement))
		return false;
	return true;
}