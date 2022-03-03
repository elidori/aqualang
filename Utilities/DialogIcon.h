#ifndef _DIALOG_ICON_HEADER_
#define _DIALOG_ICON_HEADER_

class DialogIcon
{
public:
	DialogIcon(int IconId);
	virtual ~DialogIcon();

	bool SetToDialog(HWND hDialog);
private:
	const int m_IconId;
	HICON m_hIcon;
};

#endif // _DIALOG_ICON_HEADER_