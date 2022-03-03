#ifndef _CONFIG_ADVISOR_DIALOG_HEADER_
#define _CONFIG_ADVISOR_DIALOG_HEADER_

#include "DialogObject.h"

class ConfigAdvisorDialog : public DialogObject
{
public:
	ConfigAdvisorDialog();
	virtual ~ConfigAdvisorDialog();

	bool Run(LPCTSTR UserName, bool &fShouldRunConfig);
	void Close();
private:
	bool OpenTheDialog(LPCTSTR UserName);

	virtual void DialogInit(HWND hwndDlg);
	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault);
private:
	bool m_fExit;
	bool m_fShouldRunConfig;
};

#endif // _CONFIG_ADVISOR_DIALOG_HEADER_