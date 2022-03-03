#ifndef _TIP_OF_THE_DAY_HEADER_
#define _TIP_OF_THE_DAY_HEADER_

#include "DialogObject.h"
#include "BitmapImage.h"
#include "ToolTip.h"
#include "ButtonBitmapHandlerList.h"

class TipOfTheDayScheduler;
class IConfigUpdateListener;

class TipOfTheDay : public DialogObject
{
public:
	TipOfTheDay(TipOfTheDayScheduler &rScheduler, IConfigUpdateListener &rListener);
	virtual ~TipOfTheDay();

	bool Open(LPCTSTR UserName);

	bool CanPresent();
	void Present(LPCTSTR Text, bool fMoreNext, bool fMorePrev);

	void Close();
private:
	virtual void DialogInit(HWND hwndDlg);
    virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault);

	bool UpdateContent(LPCTSTR Text, bool fMoreNext, bool fMorePrev);
	bool ModifyDialogPosition();
	bool IsMouseOnTipWindow() const;
	void DisableTipsInConfiguration();
private:
	TipOfTheDayScheduler &m_rScheduler;

	IConfigUpdateListener &m_rListener;

	_tstring m_UserName;

	ButtonBitmapHandlerList m_ButtonBitmapList;

	ToolTip m_CloseButtonToolTip;
	ToolTip m_NextButtonToolTip;
	ToolTip m_PrevButtonToolTip;

	bool m_fLatelyWasOpaque;

	HFONT m_hFont;
};

#endif // _TIP_OF_THE_DAY_HEADER_