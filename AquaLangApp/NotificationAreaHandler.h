#ifndef _NOTIFICATION_AREA_HANDLER_HEADER_
#define _NOTIFICATION_AREA_HANDLER_HEADER_

#include "FloatingMenu.h"
#include "TipOfTheDayScheduler.h"
//#include "TipOfTheDay.h"
#include "UtilityWindow.h"
#include "INotificationAreaRequestsListener.h"

class IConfigUpdateListener;

class NotificationAreaHandler :
	public IWindowMessageListener,
	public IMenuEventHandler,
	public ITipOfTheDayPresentor,
	public INotificationAreaRequestsListener
{
public:
	NotificationAreaHandler(IMenuEventHandler &rMenuEventHandler, IConfigUpdateListener &rListener, UtilityWindow &rUtilityWindow);
	virtual ~NotificationAreaHandler();

	bool Open(bool fCalculatorEnabled);

	bool OpenTipOfTheDay(LPCTSTR UserName);
	void CloseTipOfTheDay();

	void Close();
private:
	// IWindowMessageListener pure virtuals {
	virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// }

	// IMenuEventHandler pure virtuals {
	virtual void OptionSelected(int OptionId);
	// }

	// ITipOfTheDayPresentor pure virtuals {
	virtual bool CanPresentTipOfTheDay();
	virtual void PresentTipOfTheDay(LPCTSTR Text, bool fMoreNext, bool fMorePrev);
	// }

	// INotificationAreaRequestsListener pure virtuals {
	virtual bool PresentTextRequest(const char *Title, const char *Text);
	virtual bool PresentTextRequest(const wchar_t *wTitle, const wchar_t *wText);
	// }

	bool EnableTip(bool fEnable);
	bool ModifyIcon(int IconId);
	bool PopupMessage(LPCTSTR Title, LPCTSTR Text);
private:
	IMenuEventHandler &m_rMenuEventHandler;
	UtilityWindow &m_rUtilityWindow;

	bool m_fOpened;
	FloatingMenu m_Menu;
	
	TipOfTheDayScheduler m_TipOfTheDayScheduler;

//	TipOfTheDay m_TipOfTheDay;
};

#endif // _NOTIFICATION_AREA_HANDLER_HEADER_