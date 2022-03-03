#ifndef _UPGRADER_DIALOG_HEADER_
#define _UPGRADER_DIALOG_HEADER_

#include "ThreadInstance.h"
#include "IProgressInfoReceiver.h"
#include "TextResource.h"
#include "TextDialog.h"
#include "DialogObject.h"
#include "VersionUpdater.h"

class UpgraderDialog :
	public ThreadInstance,
	public IProgressInfoReceiver,
	public DialogObject
{
public:
	UpgraderDialog();
	virtual ~UpgraderDialog();

	bool Run(LPCTSTR UserName, LPCSTR ServerName, bool fCheckForBetaVersion, bool &rfShouldRunUpgrade, bool fAutoCheckForUpgrade);
	void BreakRun();
private:
	// IProgressInfoReceiver pure virtuals {
	virtual void ProgressReport(int BytesDownloaded, int TotalBytesToDownload);
	// }

	// ThreadInstance pure virtuals {
	virtual DWORD RunInThread();
	virtual void BreakThread();
	// }

	bool OpenTheDialog(LPCTSTR UserName, bool fAlreadyCheckedForUpgrade);

	virtual void DialogInit(HWND hwndDlg);
	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault);

	void DisplayUserApproval(HWND hwnd);
	void Notify(WPARAM wParam, int lParam);
	bool CheckForUpdate(
					bool fCheckForBetaVersion,
					WPARAM &rwParamNotificationType,
					_tstring &FileName,
					_tstring &Version,
					_tstring &UpdateVersionDescription,
					_tstring &Checksum
					);
	void Close();
private:
	TextDialog m_UpgradeInfoDialog;
	VersionUpdater m_Updater;

	TextResource m_TextResource;
	bool m_fExit;

	bool m_fRunUpgrade;

	_tstring m_UserName;
	std::string m_ServerName;

	bool m_fNewVersionParametersValid;
	bool m_fCheckForBetaVersion;
	_tstring m_FileName;
	_tstring m_Version;
	_tstring m_UpdateVersionDescription;
	_tstring m_Checksum;
};

#endif // _UPGRADER_DIALOG_HEADER_