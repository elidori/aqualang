#include "Pch.h"
#include "ConfigAdvisorDialog.h"
#include "resource.h"

ConfigAdvisorDialog::ConfigAdvisorDialog()
	: DialogObject(IDI_ICON_DROP),
	m_fExit(false),
	m_fShouldRunConfig(false)
{
}

ConfigAdvisorDialog::~ConfigAdvisorDialog()
{
	Close();
}

bool ConfigAdvisorDialog::Run(LPCTSTR UserName, bool &fShouldRunConfig)
{
	if(!SetDialogBitmap(IDB_BITMAP_CONFIG_ADVISOR_WINDOW, true))
		return false;

	if(!OpenTheDialog(UserName))
		return false;
	
	m_fExit = false;
	m_fShouldRunConfig = false;

	MSG msg;
	while(!m_fExit)
	{
		if(::GetMessage(&msg, NULL, NULL, NULL))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	fShouldRunConfig = m_fShouldRunConfig;
	Close();
	return true;
}

bool ConfigAdvisorDialog::OpenTheDialog(LPCTSTR UserName)
{
	HWND hDialog = OpenDialog(UserName, IDD_AQUALANG_CONFIG_ADVISOR_DIALOG);
	if(hDialog == NULL)
		return false;

	// set dialog size
	WINDOWPLACEMENT WindowPlacement;
	WindowPlacement.length = sizeof(WindowPlacement);
	if(!::GetWindowPlacement(hDialog, &WindowPlacement))
	{
		Close();
		return false;
	}

	// the tip dialog is initially centered. Below code is using this assumption
	RECT WinRect = WindowPlacement.rcNormalPosition;

	::SetWindowPos(
		hDialog,
        HWND_TOPMOST,
		WinRect.left,
		WinRect.top,
		WinRect.right - WinRect.left,
		WinRect.bottom - WinRect.top,
        SWP_SHOWWINDOW | SWP_NOACTIVATE
    );

	Show(true);

	return true;
}

void ConfigAdvisorDialog::Close()
{
	CloseDialog();

	m_fExit = false;
	m_fShouldRunConfig = false;
}

void ConfigAdvisorDialog::DialogInit(HWND hwndDlg)
{
	const COLORREF ColorKey = RGB(157, 93, 65);
	const unsigned int Alpha = 200;

	::SetWindowLong(hwndDlg, GWL_EXSTYLE, ::GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_LAYERED);
	::SetLayeredWindowAttributes(hwndDlg, ColorKey, Alpha, LWA_COLORKEY | LWA_ALPHA);
}

LRESULT ConfigAdvisorDialog::WindowProc(HWND hwnd, UINT uMsg, WPARAM UNUSED(wParam), LPARAM lParam, bool &fSkipDefault)
{
	fSkipDefault = false;
	LRESULT lResult = 0;

	switch(uMsg)
	{
	case WM_COMMAND:
		if((HWND)lParam == ::GetDlgItem(hwnd, IDCANCEL))
		{
			m_fExit = true;
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDOK))
		{
			m_fExit = true;
			m_fShouldRunConfig = true;
		}
		break;
	}
	return lResult;
}
