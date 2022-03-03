#include "Pch.h"
#include "LanguageHandler.h"
#include "BalloonTip.h"
#include "resource.h"
#include "AquaLangConfiguration.h"
#include "FocusChangeEvent.h"
#include "xmlStructure.h"
#include "UtilityWindowParams.h"
#include "UserInformationNotifier.h"
#include "AquaLangWaterMark.h"
#include "ILanguageModifier.h"
#include "Log.h"

#define TIP_INITIAL_ALPHA_MILD 100
#define TIP_INITIAL_ALPHA 200
#define TIP_INITIALLY_OPAQUE 255
#define TIP_DURATION_100MSEC 15
#define TIP_FADE_OUT_DURATION_MSEC 500
#define TIP_FADE_OUT_STEPS 20

struct TipInformation
{
	BalloonTip *pBalloonTip;
	TCHAR Text[256];
	RECT Rect;
	int TipInitialAlpha;
};

#pragma warning (disable:4238)
#pragma warning (disable:4355)

LanguageHandler::LanguageHandler(const LanguageStringTable &rLanguageStringTable, UtilityWindow &rUtilityWindow, FocusChangeEvent &rFocusChangeEvent, UserInformationNotifier &rUserInformationNotifier, ILanguageModifier &rLanguageModifier)
	: m_rUtilityWindow(rUtilityWindow),
	m_rFocusChangeEvent(rFocusChangeEvent),
	m_rUserInformationNotifier(rUserInformationNotifier),
	m_rLanguageModifier(rLanguageModifier),
	m_BalloonTip(
			TIP_FADE_OUT_DURATION_MSEC,
			TIP_FADE_OUT_STEPS,
			IDB_BITMAP_LEFT_UP_BALLOON,
			IDB_BITMAP_RIGHT_UP_BALLOON,
			IDB_BITMAP_LEFT_DOWN_BALLOON,
			IDB_BITMAP_RIGHT_DOWN_BALLOON,
			RGB(0, 0, 0),
			RGB(255, 255, 255),
			&TextRect(3, 12, 39, 35)
			),
	m_LiteBalloonTip(
			TIP_FADE_OUT_DURATION_MSEC,
			TIP_FADE_OUT_STEPS,
			IDB_BITMAP_LITE_LEFT_UP_BALLOON,
			IDB_BITMAP_LITE_RIGHT_UP_BALLOON,
			IDB_BITMAP_LITE_LEFT_DOWN_BALLOON,
			IDB_BITMAP_LITE_RIGHT_DOWN_BALLOON,
			RGB(0, 0, 255),
			RGB(1, 1, 1),
			&TextRect(3, 12, 39, 35)
			),
	m_DebugBalloonTip(),
	m_WindowItemDataBase(rLanguageStringTable),
	m_pCurrentWindowItem(NULL),
	m_pCurrentInfluencingWindowItem(NULL)
{
	m_BalloonTipSettings.fActive = false;
	m_BalloonTipSettings.ApearanceDurationIn100msec = TIP_DURATION_100MSEC;
	m_BalloonTipSettings.ShowMode = BALLOON_SHOW_ALWAYS;
}

LanguageHandler::~LanguageHandler()
{
	Close();
}

Result LanguageHandler::Open(LPCTSTR UserName, const AquaLangConfiguration &rConfig)
{
	assert(UserName);
	m_UserName = UserName;

	if(rConfig.BalloonTipSettings.fActive)
	{
		m_BalloonTip.Open();
		m_LiteBalloonTip.Open();
		m_WindowItemDataBase.Open(UserName);

		Result res = m_rFocusChangeEvent.Register(*this);
		if(res != Result_Success)
		{
			Log(_T("LanguageHandler::Open - failed to open focus change event (res=%d)\n"), res);
			return res;
		}
		m_BalloonTipSettings = rConfig.BalloonTipSettings;

		if(!m_rUtilityWindow.Register(*this))
		{
			Log(_T("LanguageHandler::Open - failed to open utility window\n"));
			return Result_ResourceAllocFailed;
		}
	}
	else
	{
		m_WindowItemDataBase.OpenRulesOnly(UserName);
	}

	Log(_T("LanguageHandler::Open - succeeded\n"));
	return Result_Success;
} 

void LanguageHandler::Close()
{
	m_rUtilityWindow.Unregister(*this);
	m_BalloonTip.Close();
	m_LiteBalloonTip.Close();
	m_DebugBalloonTip.Term();
	m_rFocusChangeEvent.Unregister(*this);
	m_WindowItemDataBase.Close();
	m_pCurrentWindowItem = NULL;
	m_pCurrentInfluencingWindowItem = NULL;
}

void LanguageHandler::UIEvent(EventType Type, HWND hwnd, LPCTSTR SubObjectId, const RECT *pRect, UIActionType Action)
{
	// get current window setting for the language
	HKL hCurrentKL = ::GetKeyboardLayout(GetWindowThreadProcessId(hwnd, NULL));

	if(Type == Type_Focus)
	{
		// make sure this is not an aqualang window
		HWND hNextParent = hwnd;
		HWND hTopParent = NULL;
		while(hNextParent != NULL)
		{
			hTopParent = hNextParent;
			hNextParent = ::GetParent(hNextParent);
		}
		AquaLangWaterMark WaterMarkFinder;
		_tstring UserName;
		if(WaterMarkFinder.FindMarkInWindow(hTopParent, UserName))
		{
			Log(_T("LanguageHandler::UIEvent - window is an AquaLang component of %s. Ignoring\n"), UserName.c_str());
			return;
		}

		// find window entry
		bool fNewDataBaseEntry = false;
		HKL DefaultLanguage;
		const WindowItem *pWindowRule = NULL;
		WindowItem *pWindowItem = m_WindowItemDataBase.GetWindowItem(hwnd, SubObjectId, fNewDataBaseEntry, &DefaultLanguage, pWindowRule);
		if(!pWindowItem)
			return;
		m_pCurrentWindowItem = pWindowItem;

		// update default language according to the rules database
		if(fNewDataBaseEntry)
		{
			if(DefaultLanguage != NULL)
			{
				pWindowItem->SetLanguage(DefaultLanguage);
			}
		}
		// override Action type according to the rules database
		if(pWindowRule)
		{
			const xmlValue *pIsEditableXml = pWindowRule->GetPropertyValue(RuleTypeIsEditable);
			if(pIsEditableXml)
			{
				bool IsEditable = ((xmlBool *)pIsEditableXml)->GetBool(false);
				if(IsEditable)
				{
					Action = ActionType_EditWindow;
				}
				else
				{
					Action = ActionType_NonEditWindow;
				}
			}
		}

		m_rUserInformationNotifier.IncreaseNumberOfFocusEvents();
		m_pCurrentInfluencingWindowItem = NULL;

		bool CanPresentLiteTip = ((m_BalloonTipSettings.ShowMode == BALLOON_SHOW_ALWAYS) && CheckMinIntervalFromLastPresented(*pWindowItem));
		if(CanPresentLiteTip)
		{
			pWindowItem->MarkNow();
		}

		HKL hRestoredKL = NULL;
		pWindowItem->GetLanguage(hRestoredKL);
		if(hRestoredKL == NULL)
		{
			// store
			pWindowItem->SetLanguage(hCurrentKL);
			PresentTip(m_LiteBalloonTip, pRect, hCurrentKL, Action, TIP_INITIAL_ALPHA_MILD, CanPresentLiteTip, *pWindowItem, fNewDataBaseEntry);
		}
		else if(hRestoredKL != hCurrentKL)
		{
			m_rUserInformationNotifier.IncreaseNumberOfLanguageChangeEvents();
			m_pCurrentInfluencingWindowItem = pWindowItem;
			// restore
			m_rLanguageModifier.ChangeLanguage(hwnd, hRestoredKL);
			PresentTip(m_BalloonTip, pRect, hRestoredKL, Action, TIP_INITIAL_ALPHA, m_BalloonTipSettings.ShowMode != BALLOON_SHOW_NEVER, *pWindowItem, fNewDataBaseEntry);
		}
		else
		{
			PresentTip(m_LiteBalloonTip, pRect, hRestoredKL, Action, TIP_INITIALLY_OPAQUE, CanPresentLiteTip, *pWindowItem, fNewDataBaseEntry);
		}
	}
	else if(Type == Type_StoreLanguage)
	{
		if(m_pCurrentWindowItem != NULL)
		{
			HKL hStoredKL = NULL;
			m_pCurrentWindowItem->GetLanguage(hStoredKL);
			if(hStoredKL != hCurrentKL)
			{
				m_rUserInformationNotifier.IncreaseLanguageManualChangeEvents();
				if(m_pCurrentInfluencingWindowItem)
				{
					m_rUserInformationNotifier.IncreaseLanguageManualFixEvents();
					m_pCurrentInfluencingWindowItem = NULL;
				}
	
				m_pCurrentWindowItem->SetLanguage(hCurrentKL);
//				Log(_T("LanguageHandler::UIEvent - changed database to hkl=0x%x\n"), hCurrentKL);
			}
		}
		else
		{
			// this is possible only when the application started and the first window is handled without previous focus change
			Log(_T("LanguageHandler::UIEvent - m_pCurrentWindowItem is NULL\n"));
		}
	}
	else
	{
		assert(0);
	}
}

void LanguageHandler::PresentTip(
							BalloonTip &rBalloonTip,
							const RECT *pRect,
							HKL hKL,
							UIActionType Action,
							int InitialAlpha,
							bool fPresent,
							WindowItem &rItem,
							bool fNewEntry
							)
{
	m_LiteBalloonTip.Term();
	m_BalloonTip.Term();

	_tstring Text;
	_tstring Dummy;
	if(!m_WindowItemDataBase.GetLanguageString(hKL, Dummy, Text))
	{
		TCHAR Language[256];
		_stprintf_s(Language, sizeof(Language) / sizeof(Language[0]), _T("%04X"), HIWORD((int)hKL));
		Text = Language;
	}

	if(m_BalloonTipSettings.ShowMode == BALLOON_SHOW_DEBUG)
	{
		if(&rBalloonTip == &m_BalloonTip)
		{
			Text += _T(" - Modified");
		}
		if(fNewEntry)
		{
			Text += _T(" - New");
		}
		Text += _T("\n");
		rItem.GetDebugFullString(Text);
		m_DebugBalloonTip.Init(m_UserName.c_str(), Text.c_str(), pRect, IDD_DEBUGBALLOONTIPWINDOW_DIALOG);
	}
	else if(fPresent)
	{
		if(Action != ActionType_NonEditWindow && pRect != NULL)
		{
			TipInformation *pInfo = new TipInformation;
			if(pInfo)
			{
				pInfo->pBalloonTip = &rBalloonTip;
				_tcsncpy_s(pInfo->Text, sizeof(pInfo->Text) / sizeof(pInfo->Text[0]), Text.c_str(), sizeof(pInfo->Text) / sizeof(pInfo->Text[0]));
				pInfo->Text[sizeof(pInfo->Text) / sizeof(pInfo->Text[0]) - 1] = _T('\0');
				pInfo->Rect = *pRect;
				pInfo->TipInitialAlpha = InitialAlpha;

				m_rUtilityWindow.UserMessage(WPARAM_BALLOON_DISPLAY_EVENT, (LPARAM)pInfo);
			}
		}
	}
}

void LanguageHandler::TipShouldClose(BalloonTip &rTip)
{
	// may need to synchronize with PresentTip
	if(&rTip == &m_BalloonTip)
	{
		m_BalloonTip.Term();
	}
	else if(&rTip == &m_LiteBalloonTip)
	{
		m_LiteBalloonTip.Term();
	}
}

void LanguageHandler::TipWasClicked(BalloonTip &rTip, bool fLeftClick)
{
	TipShouldClose(rTip);
	
	mouse_event(fLeftClick ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, NULL);
}

void LanguageHandler::Message(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_UTILITY_USER_MESSAGE)
	{
		switch(wParam)
		{
		case WPARAM_BALLOON_DISPLAY_EVENT:
			if(lParam != 0)
			{
				TipInformation *pInfo = (TipInformation *)lParam;
				if(pInfo->pBalloonTip)
				{
					pInfo->pBalloonTip->Init(
										m_UserName.c_str(),
										_T("AquaLang water drop"),
										pInfo->Text,
										&pInfo->Rect,
										IDD_BALLOONTIPWINDOW_DIALOG,
										pInfo->TipInitialAlpha,
										m_BalloonTipSettings.ApearanceDurationIn100msec * 100,
										false,
										this
										);
				}
			}
			break;
		}
	}
}

bool LanguageHandler::CheckMinIntervalFromLastPresented(WindowItem &rWindowItem) const
{
	__int64 SecondsFromLastMark = 0;
	if(!rWindowItem.GetLastMark(SecondsFromLastMark))
		return true;
	return (SecondsFromLastMark >= m_BalloonTipSettings.MinTimeBetweenTwoSameTipsInMin * 60);
}