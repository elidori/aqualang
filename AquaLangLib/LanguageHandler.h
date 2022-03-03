#ifndef _LANGUAGE_HANDLER_HEADER_
#define _LANGUAGE_HANDLER_HEADER_

#include "IUIEventListener.h"
#include "WindowItemDataBase.h"
#include "BalloonTip.h"
#include "DebugBalloonTip.h"
#include "AquaLangConfiguration.h"
#include "UtilityWindow.h"
#include "WinHookResult.h"

using namespace WinHook;

class FocusChangeEvent;
class UserInformationNotifier;
class LanguageStringTable;
class ILanguageModifier;

class LanguageHandler :
	public IUIEventListener,
	public ITipListener,
	public IWindowMessageListener
{
public:
	LanguageHandler(const LanguageStringTable &rLanguageStringTable, UtilityWindow &rUtilityWindow, FocusChangeEvent &rFocusChangeEvent, UserInformationNotifier &rUserInformationNotifier, ILanguageModifier &rLanguageModifier);
	virtual ~LanguageHandler();

	const WindowItemDataBase &GetDataBase() const	{ return m_WindowItemDataBase; }

	Result Open(LPCTSTR UserName, const AquaLangConfiguration &rConfig);
	void Close();
private:
	// IUIEventListener pure virtuals {
	virtual void UIEvent(EventType Type, HWND hwnd, LPCTSTR SubObjectId, const RECT *pRect, UIActionType Action);
	// }

	void PresentTip(
				BalloonTip &rBalloonTip,
				const RECT *pRect,
				HKL hKL,
				UIActionType Action,
				int InitialAlpha,
				bool fPresent,
				WindowItem &rItem,
				bool fNewEntry
				);

	// ITipListener pure virtuals {
	virtual void TipShouldClose(BalloonTip &rTip);
	virtual void TipWasClicked(BalloonTip &rTip, bool fLeftClick);
	// }
	
	// IWindowMessageListener pure virtuals {
	virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// }

	bool CheckMinIntervalFromLastPresented(WindowItem &rWindowItem) const;
private:
	UtilityWindow &m_rUtilityWindow;
	FocusChangeEvent &m_rFocusChangeEvent;
	WindowItemDataBase m_WindowItemDataBase;

	ILanguageModifier &m_rLanguageModifier;

	BalloonTip m_BalloonTip;
	BalloonTip m_LiteBalloonTip;
	DebugBalloonTip m_DebugBalloonTip;
	AquaLangBalloonTipSettings m_BalloonTipSettings;

	WindowItem *m_pCurrentWindowItem;

	// this is for statistics counting
	WindowItem *m_pCurrentInfluencingWindowItem;
	UserInformationNotifier &m_rUserInformationNotifier;

	_tstring m_UserName;
};

#endif // _LANGUAGE_HANDLER_HEADER_