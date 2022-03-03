#include "Pch.h"
#include "UpgraderDialog.h"
#include "resource.h"
#include "Log.h"

enum
{
	WM_AQUALANG_USER_MESSAGE = WM_USER + 11,

	WPARAM_USER_FAILED_CONNECTING_TO_SERVER = 0,
	WPARAM_USER_NO_UPGRADE = 1,
	WPARAM_USER_UPGRADE_FOUND = 2,
	WPARAM_USER_DOWNLOAD_STATUS = 3,
	WPARAM_USER_DOWNLOAD_COMPLETED = 4,
	WPARAM_USER_DOWNLOAD_CHECKSUM_ERROR = 5,
	WPARAM_USER_WAKEUP = 6
};

struct DownloadStatus
{
	int BytesDownloaded;
	int TotalBytesToDownload;
};

UpgraderDialog::UpgraderDialog()
	: DialogObject(IDI_ICON_DROP, false),
	m_UpgradeInfoDialog(IDD_DIALOG_VIEW_UPGRADE_INFO, IDC_EDIT_UPGRADE_INFO),
	m_fExit(false),
	m_fRunUpgrade(false),
	m_fCheckForBetaVersion(false),
	m_fNewVersionParametersValid(false),
	m_TextResource()
{
	m_TextResource.SetTextColor(RGB(0, 0, 0), true);
	m_TextResource.SetTargetRect(TextRect(30, 250, 413, 100));
}

UpgraderDialog::~UpgraderDialog()
{
	Close();
}

bool UpgraderDialog::Run(LPCTSTR UserName, LPCSTR ServerName, bool fCheckForBetaVersion, bool &rfShouldRunUpgrade, bool fAutoCheckForUpgrade)
{
	rfShouldRunUpgrade = false;
	m_fRunUpgrade = false;

	m_UserName = UserName;
	m_ServerName = ServerName;

	m_fCheckForBetaVersion = fCheckForBetaVersion;

	if(!SetDialogBitmap(IDB_BITMAP_UPGRADE_WINDOW, true))
		return false;

	if(fAutoCheckForUpgrade)
	{
		WPARAM wParamMessageType;
		m_fNewVersionParametersValid = CheckForUpdate(fCheckForBetaVersion, wParamMessageType, m_FileName, m_Version, m_UpdateVersionDescription, m_Checksum);
		if(!m_fNewVersionParametersValid)
			return true;
	}

	if(!m_fExit)
	{
		if(!OpenTheDialog(UserName, fAutoCheckForUpgrade))
			return false;
		if(!fAutoCheckForUpgrade) // in case a new version was found, the thread shall be activated only after user approved the installation
		{
			if(!OpenThread())
			{
				Close();
				return false;
			}
		}
	}

	MSG msg;
	while(!m_fExit)
	{
		if(::GetMessage(&msg, NULL, NULL, NULL))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	rfShouldRunUpgrade = m_fRunUpgrade;

	Close();
	return true;
}

void UpgraderDialog::BreakRun()
{
	m_Updater.Break();
	m_fExit = true;
	Notify(WPARAM_USER_WAKEUP, 0);
}

bool UpgraderDialog::OpenTheDialog(LPCTSTR UserName, bool fAlreadyCheckedForUpgrade)
{
	HWND hDialog = OpenDialog(UserName, IDD_AQUALANG_UPGRADE_DIALOG);
	if(hDialog == NULL)
		return false;

	SetTitle(_T("AquaLang Upgrade"));

	WINDOWPLACEMENT WindowPlacement;
	WindowPlacement.length = sizeof(WindowPlacement);
	if(!::GetWindowPlacement(hDialog, &WindowPlacement))
		return false;

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

	if(fAlreadyCheckedForUpgrade)
	{
		DisplayUserApproval(hDialog);
	}
	else
	{
		m_TextResource.SetText(_T("Searching for updated version ..."));
		::InvalidateRect(hDialog, NULL, TRUE);
	}
	Show(true);

	return true;
}

void UpgraderDialog::ProgressReport(int BytesDownloaded, int TotalBytesToDownload)
{
	DownloadStatus *pDownloadStatus = new DownloadStatus;
	if(pDownloadStatus)
	{
		pDownloadStatus->BytesDownloaded = BytesDownloaded;
		pDownloadStatus->TotalBytesToDownload = TotalBytesToDownload;
	
		Notify(WPARAM_USER_DOWNLOAD_STATUS, (int)pDownloadStatus);
	}
}

DWORD UpgraderDialog::RunInThread()
{
	if(!m_fNewVersionParametersValid)
	{
		WPARAM wParamMessageType;
		m_fNewVersionParametersValid = CheckForUpdate(m_fCheckForBetaVersion, wParamMessageType, m_FileName, m_Version, m_UpdateVersionDescription, m_Checksum);
		Notify(wParamMessageType, 0);
	}
	else
	{
		if(!m_Updater.Open(m_ServerName.c_str()))
		{
			Notify(WPARAM_USER_FAILED_CONNECTING_TO_SERVER, 0);
			return (DWORD)-1;
		}
		_tstring CalcCheckSum;
		bool fSuccess = m_Updater.DownloadUpdate(m_FileName.c_str(), this, CalcCheckSum);
		m_Updater.Close();

		if(!IsBreaking())
		{
			if(!fSuccess)
			{
				Notify(WPARAM_USER_FAILED_CONNECTING_TO_SERVER, 0);
				return (DWORD)-1;
			}
			
			if(CalcCheckSum != m_Checksum)
			{
				Log(_T("UpgraderDialog::RunInThread() - checksum error expected='%s', received='%s'\n"), m_Checksum.c_str(), CalcCheckSum.c_str());
				Notify(WPARAM_USER_DOWNLOAD_CHECKSUM_ERROR, 0);
			}
			else
			{
				Notify(WPARAM_USER_DOWNLOAD_COMPLETED, 0);
			}
		}
	}
	return 0;
}

void UpgraderDialog::BreakThread()
{
	m_Updater.Break();
}

void UpgraderDialog::DialogInit(HWND hwndDlg)
{
	const COLORREF ColorKey = RGB(0, 0, 255);
	const unsigned int Alpha = 255;

	::SetWindowLong(hwndDlg, GWL_EXSTYLE, ::GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_LAYERED);
	::SetLayeredWindowAttributes(hwndDlg, ColorKey, Alpha, LWA_COLORKEY | LWA_ALPHA);
}

LRESULT UpgraderDialog::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault)
{
	fSkipDefault = false;
	LRESULT lResult = 0;

	switch(uMsg)
	{
	case WM_ERASEBKGND:
		m_TextResource.DrawText((HDC)wParam, false);
		fSkipDefault = true;
		lResult = FALSE;
		break;
	case WM_COMMAND:
		if((HWND)lParam == ::GetDlgItem(hwnd, IDCANCEL))
		{
			CloseThread();
			VersionUpdater::DeleteInstallationFolder();
			m_fExit = true;
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDOK))
		{
			::ShowWindow(::GetDlgItem(hwnd, IDOK), SW_HIDE);
			::SetWindowText(::GetDlgItem(hwnd, IDCANCEL), _T("Cancel"));
			::ShowWindow(::GetDlgItem(hwnd, IDC_BUTTON_VIEW_UPGRADE_INFO), SW_HIDE);
			m_UpgradeInfoDialog.Close();
			if(!OpenThread()) // for start downloading
			{
#ifdef _DEBUG
				MessageBox(NULL, _T("Intenal failure: can't open thread"), _T("AquaLang update version"), MB_ICONEXCLAMATION | MB_OK);
#endif
				m_fExit = true;
			}
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_VIEW_UPGRADE_INFO))
		{
			_tstring VersionInfoText;
			VersionInfoText += _T("Version ") + m_Version + _T(":\r\n\r\n") + m_UpdateVersionDescription;
			m_UpgradeInfoDialog.Open(m_UserName.c_str(), VersionInfoText.c_str(), hwnd);
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_MINIMIZE))
		{
			::ShowWindow(hwnd, SW_MINIMIZE);
		}
		break;
	case WM_AQUALANG_USER_MESSAGE:
		{
			switch (wParam)
			{
			case WPARAM_USER_FAILED_CONNECTING_TO_SERVER:
				m_TextResource.SetText(_T("Error connecting to Server. Please try again later"));
				::InvalidateRect(hwnd, NULL, TRUE);
				::SetWindowText(::GetDlgItem(hwnd, IDCANCEL), _T("OK"));
				::ShowWindow(::GetDlgItem(hwnd, IDC_BUTTON_MINIMIZE), SW_HIDE);
				::ShowWindow(hwnd, SW_NORMAL);
				break;
			case WPARAM_USER_NO_UPGRADE:
				m_TextResource.SetText(_T("No newer version was found. Please try again later"));
				::InvalidateRect(hwnd, NULL, TRUE);
				::SetWindowText(::GetDlgItem(hwnd, IDCANCEL), _T("OK"));
				break;
			case WPARAM_USER_UPGRADE_FOUND:
				CloseThread();
				DisplayUserApproval(hwnd);
				break;
			case WPARAM_USER_DOWNLOAD_STATUS:
				{
					DownloadStatus *pDownloadStatus = (DownloadStatus *)lParam;
					if(pDownloadStatus)
					{
						TCHAR Text[256];
						_stprintf_s(
								Text, sizeof(Text) / sizeof(Text[0]),
								_T("Downloaded %d Kbytes out of %d"),
								pDownloadStatus->BytesDownloaded / 1000,
								pDownloadStatus->TotalBytesToDownload / 1000
								);
						m_TextResource.SetText(Text);

						if(pDownloadStatus->TotalBytesToDownload)
						{
							TCHAR UpgradeCaption[256];
							_stprintf_s(
									UpgradeCaption, sizeof(UpgradeCaption) / sizeof(UpgradeCaption[0]),
									_T("%d%% Downloaded"), (int)((__int64)pDownloadStatus->BytesDownloaded * 100 / pDownloadStatus->TotalBytesToDownload)
									);
							::SetWindowText(hwnd, UpgradeCaption);
						}
						::InvalidateRect(hwnd, NULL, TRUE);
						delete pDownloadStatus;

						::ShowWindow(::GetDlgItem(hwnd, IDC_BUTTON_MINIMIZE), SW_SHOW);
					}
				}
				break;
			case WPARAM_USER_DOWNLOAD_CHECKSUM_ERROR:
				m_TextResource.SetText(_T("Download failed with checksum error.\nPlease try again later"));
				::InvalidateRect(hwnd, NULL, TRUE);
				::SetWindowText(::GetDlgItem(hwnd, IDCANCEL), _T("OK"));
				::ShowWindow(::GetDlgItem(hwnd, IDC_BUTTON_MINIMIZE), SW_HIDE);
				::ShowWindow(hwnd, SW_NORMAL);
				break;
			case WPARAM_USER_DOWNLOAD_COMPLETED:
				m_fRunUpgrade = true;
				m_fExit = true;
				break;
			case WPARAM_USER_WAKEUP:
				// was sent to release from GetMessage
				break;
			}
		}
		break;
	}

	return lResult;
}

void UpgraderDialog::DisplayUserApproval(HWND hwnd)
{
	_tstring Text = _T("Version ");
	Text += m_Version;
	Text += _T(" was found.\nPress the 'Install now' button if you wish to install it");
	m_TextResource.SetText(Text.c_str());

	::InvalidateRect(hwnd, NULL, TRUE);
	::SetWindowText(::GetDlgItem(hwnd, IDCANCEL), _T("Remind me later"));
	::SetWindowText(::GetDlgItem(hwnd, IDOK), _T("Install now"));
	::ShowWindow(::GetDlgItem(hwnd, IDOK), SW_SHOW);
	::ShowWindow(::GetDlgItem(hwnd, IDC_BUTTON_VIEW_UPGRADE_INFO), SW_SHOW);
}

void UpgraderDialog::Notify(WPARAM wParam, int lParam)
{
	PostMessage(WM_AQUALANG_USER_MESSAGE, wParam, lParam);
}

bool UpgraderDialog::CheckForUpdate(
							bool fCheckForBetaVersion,
							WPARAM &rwParamNotificationType,
							_tstring &FileName,
							_tstring &Version,
							_tstring &UpdateVersionDescription,
							_tstring &Checksum
							)
{
	if(!m_Updater.Open(m_ServerName.c_str()))
	{
		rwParamNotificationType = WPARAM_USER_FAILED_CONNECTING_TO_SERVER;
		return false;
	}
	bool fConnectionError;
	bool fUpgradeFound;
	fUpgradeFound = m_Updater.CheckForUpdate(fCheckForBetaVersion, FileName, Version, UpdateVersionDescription, Checksum, fConnectionError);
	m_Updater.Close();
	
	if(fUpgradeFound)
	{
		rwParamNotificationType = WPARAM_USER_UPGRADE_FOUND;
	}
	else
	{
		if(fConnectionError)
		{
			rwParamNotificationType = WPARAM_USER_FAILED_CONNECTING_TO_SERVER;
		}
		else
		{
			rwParamNotificationType = WPARAM_USER_NO_UPGRADE;
		}
	}
	return fUpgradeFound;
}

void UpgraderDialog::Close()
{
	m_UpgradeInfoDialog.Close();
	CloseDialog();
	m_fExit = false;
	CloseThread();

	m_Updater.Close();

	m_fRunUpgrade = false;
	m_fNewVersionParametersValid = false;
	m_fCheckForBetaVersion = false;
	m_TextResource.SetText(_T(""));
}
