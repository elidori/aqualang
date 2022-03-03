#ifndef _DIALOG_TAB_HEADER_
#define _DIALOG_TAB_HEADER_

class DialogObject;

class DialogTab
{
public:
	DialogTab();
	virtual ~DialogTab();

	void Open(HWND hTab);
	bool AddPage(DialogObject *pPage, LPCTSTR Name);
	void Close();

	DialogObject *GetActivePage()	{ return m_pActivePage; }
	bool ActivatePage(DialogObject *pPage);

	void OnNotifySelectChange(NMHDR &rInfo);
private:
	void DoActivatePage(DialogObject *pNextActivePage);
private:
	HWND m_hTab;

	typedef std::map<int, DialogObject *> PageMap;
	PageMap m_PageMap;

	DialogObject *m_pActivePage;
};

#endif // _DIALOG_TAB_HEADER_