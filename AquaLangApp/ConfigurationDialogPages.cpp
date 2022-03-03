#include "Pch.h"
#include "ConfigurationDialogPages.h"
#include "ConfigurationMessages.h"
#include "Resource.h"

extern HMODULE g_hModule;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ConfigPage
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ConfigPage::ConfigPage()
	: DialogObject(-1, false)
{
}

ConfigPage::~ConfigPage()
{
	// CloseDialog() is done by the derived class
}

LRESULT ConfigPage::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault)
{
	fSkipDefault = false;
	LRESULT lResult = 0;

	HandleMessage(hwnd, uMsg, wParam, lParam);

	return lResult;
}

void ConfigPage::Release()
{
	delete this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WaterDropConfigPage
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ConfigPage *WaterDropConfigPage::Create(HWND hParent, AquaLangBalloonTipSettings &rSettings)
{
	WaterDropConfigPage *pNew = new WaterDropConfigPage(rSettings);
	if(!pNew)
		return NULL;
	if(!pNew->OpenDialog(_T(""), IDD_DIALOG_WATER_TIPS, hParent))
	{
		delete pNew;
		return NULL;
	}
	return pNew;
}

WaterDropConfigPage::WaterDropConfigPage(AquaLangBalloonTipSettings &rSettings)
	: m_rSettings(rSettings)
{
}

WaterDropConfigPage::~WaterDropConfigPage()
{
	CloseDialog();
}

void WaterDropConfigPage::Load()
{
	HWND hPageDialog = GetDialogHandle();
	if(!hPageDialog)
		return;

	HWND hwnd;
	bool fBalloonTimeSettingsEnabled = false;

	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ENABLE_AUTO_LANGUAGE_SET);
	if(m_rSettings.fActive)
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	::SendMessage(::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DONT_DISPLAY), BM_SETCHECK, BST_UNCHECKED, 0);
	::SendMessage(::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DISPLAY_WHEN_CHANGED), BM_SETCHECK, BST_UNCHECKED, 0);
	::SendMessage(::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DISPLAY_ALWAYS), BM_SETCHECK, BST_UNCHECKED, 0);
	::SendMessage(::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DISPLAY_DEBUG), BM_SETCHECK, BST_UNCHECKED, 0);

	switch(m_rSettings.ShowMode)
	{
	case BALLOON_SHOW_NEVER:
		hwnd = ::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DONT_DISPLAY);
		::SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
		fBalloonTimeSettingsEnabled = false;
		break;
	case BALLOON_SHOW_WHEN_MODIFIED:
		hwnd = ::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DISPLAY_WHEN_CHANGED);
		::SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
		fBalloonTimeSettingsEnabled = true;
		break;
	case BALLOON_SHOW_ALWAYS:
		hwnd = ::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DISPLAY_ALWAYS);
		::SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
		fBalloonTimeSettingsEnabled = true;
		break;
	case BALLOON_SHOW_DEBUG:
		hwnd = ::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DISPLAY_DEBUG);
		::SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
		fBalloonTimeSettingsEnabled = false;
		break;
	}
	::EnableWindow(::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DONT_DISPLAY), m_rSettings.fActive);
	::EnableWindow(::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DISPLAY_WHEN_CHANGED), m_rSettings.fActive);
	::EnableWindow(::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DISPLAY_ALWAYS), m_rSettings.fActive);
	::EnableWindow(::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DISPLAY_DEBUG), m_rSettings.fActive);

	TCHAR WindowText[256];

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_BALLOON_DURATION);
	_stprintf_s(WindowText, sizeof(WindowText) / sizeof(WindowText[0]), _T("%d"), m_rSettings.ApearanceDurationIn100msec);
	::SetWindowText(hwnd, WindowText);
	::EnableWindow(hwnd, fBalloonTimeSettingsEnabled && m_rSettings.fActive);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_MIN_TIME_BETWEEN_TIPS);
	_stprintf_s(WindowText, sizeof(WindowText) / sizeof(WindowText[0]), _T("%d"), m_rSettings.MinTimeBetweenTwoSameTipsInMin);
	::SetWindowText(hwnd, WindowText);
	::EnableWindow(hwnd, fBalloonTimeSettingsEnabled && m_rSettings.fActive);
}

void WaterDropConfigPage::Store()
{
	HWND hPageDialog = GetDialogHandle();
	if(!hPageDialog)
		return;

	HWND hwnd;

	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ENABLE_AUTO_LANGUAGE_SET);
	m_rSettings.fActive = (::SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);

	if(::SendMessage(::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DONT_DISPLAY), BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		m_rSettings.ShowMode = BALLOON_SHOW_NEVER;
	}
	else if(::SendMessage(::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DISPLAY_WHEN_CHANGED), BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		m_rSettings.ShowMode = BALLOON_SHOW_WHEN_MODIFIED;
	}
	else if(::SendMessage(::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DISPLAY_ALWAYS), BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		m_rSettings.ShowMode = BALLOON_SHOW_ALWAYS;
	}
	else if(::SendMessage(::GetDlgItem(hPageDialog, IDC_RADIO_BALLOON_DISPLAY_DEBUG), BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		m_rSettings.ShowMode = BALLOON_SHOW_DEBUG;
	}
	else
	{
		assert(0);
		m_rSettings.ShowMode = BALLOON_SHOW_NEVER;
	}

	TCHAR WindowText[256];

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_BALLOON_DURATION);
	::GetWindowText(hwnd, WindowText, sizeof(WindowText) / sizeof(WindowText[0]));
	_stscanf_s(WindowText, _T("%d"), &m_rSettings.ApearanceDurationIn100msec);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_MIN_TIME_BETWEEN_TIPS);
	::GetWindowText(hwnd, WindowText, sizeof(WindowText) / sizeof(WindowText[0]));
	_stscanf_s(WindowText, _T("%d"), &m_rSettings.MinTimeBetweenTwoSameTipsInMin);
}

void WaterDropConfigPage::HandleMessage(HWND hwnd, UINT uMsg, WPARAM UNUSED(wParam), LPARAM lParam)
{
	if(uMsg == WM_COMMAND)
	{
		if((HWND)lParam == ::GetDlgItem(hwnd, IDC_CHECK_ENABLE_AUTO_LANGUAGE_SET))
		{
			bool fActive = (::SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
			
			bool fBalloonTimeSettingsEnabled = false;
			if(::SendMessage(::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DISPLAY_WHEN_CHANGED), BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				fBalloonTimeSettingsEnabled = true;
			}
			else if(::SendMessage(::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DISPLAY_ALWAYS), BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				fBalloonTimeSettingsEnabled = true;
			}

			::EnableWindow(::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DONT_DISPLAY), fActive);
			::EnableWindow(::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DISPLAY_WHEN_CHANGED), fActive);
			::EnableWindow(::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DISPLAY_ALWAYS), fActive);
			::EnableWindow(::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DISPLAY_DEBUG), fActive);
			::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_BALLOON_DURATION), fBalloonTimeSettingsEnabled && fActive);
			::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_MIN_TIME_BETWEEN_TIPS), fBalloonTimeSettingsEnabled && fActive);
		}
		if((HWND)lParam == ::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DONT_DISPLAY))
		{
			if(::SendMessage(::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DONT_DISPLAY), BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_BALLOON_DURATION), false);
				::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_MIN_TIME_BETWEEN_TIPS), false);
			}
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DISPLAY_WHEN_CHANGED))
		{
			if(::SendMessage(::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DISPLAY_WHEN_CHANGED), BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_BALLOON_DURATION), true);
				::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_MIN_TIME_BETWEEN_TIPS), true);
			}
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DISPLAY_ALWAYS))
		{
			if(::SendMessage(::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DISPLAY_ALWAYS), BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_BALLOON_DURATION), true);
				::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_MIN_TIME_BETWEEN_TIPS), true);
			}
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DISPLAY_DEBUG))
		{
			if(::SendMessage(::GetDlgItem(hwnd, IDC_RADIO_BALLOON_DISPLAY_DEBUG), BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_BALLOON_DURATION), false);
				::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_MIN_TIME_BETWEEN_TIPS), false);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HotKeyConfigPage
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ConfigPage *HotKeyConfigPage::Create(HWND hParent, AquaLangConfiguration &rSettings)
{
	HotKeyConfigPage *pNew = new HotKeyConfigPage(rSettings);
	if(!pNew)
		return NULL;
	if(!pNew->OpenDialog(_T(""), IDD_DIALOG_HOT_KEY_SETTINGS, hParent))
	{
		delete pNew;
		return NULL;
	}
	return pNew;
}

HotKeyConfigPage::HotKeyConfigPage(AquaLangConfiguration &rSettings)
	: m_rTextConversionSettings(rSettings.TextConversionSettings),
	m_rWebBrowseSettings(rSettings.WebBrowseSettings),
	m_rClipboardSwapSettings(rSettings.SwapSettings)
{
}

HotKeyConfigPage::~HotKeyConfigPage()
{
	CloseDialog();
}

void HotKeyConfigPage::Load()
{
	HWND hPageDialog = GetDialogHandle();
	if(!hPageDialog)
		return;

	HWND hwnd;

	// text conversion
	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ENABLE_TEXT_CONVERSION);
	if(m_rTextConversionSettings.fActive)
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_TEXT_CONVERSION_HOT_KEY);
	::SetWindowTextA(hwnd, m_rTextConversionSettings.HotKey);
	::EnableWindow(hwnd, m_rTextConversionSettings.fActive);

	// web browse
	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ENABLE_WEB_BROWSE);
	if(m_rWebBrowseSettings.fActive)
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	hwnd = ::GetDlgItem(hPageDialog, IDC_COMBO_SEARCH_ENGINE);
	LRESULT nCount = ::SendMessage(hwnd, CB_GETCOUNT, 0, 0);
	if(nCount > 0)
	{
		::SendMessage(hwnd, CB_SETCURSEL, 0, 0);
	}
	int i;
	for(i = 0; i < nCount; i++)
	{
		LRESULT Code = ::SendMessage(hwnd, CB_GETITEMDATA, i, 0);
		if(Code == m_rWebBrowseSettings.nWebSearchOptionCode)
		{
			::SendMessage(hwnd, CB_SETCURSEL, i, 0);
			break;
		}
	}
	::EnableWindow(hwnd, m_rWebBrowseSettings.fActive);

	hwnd = ::GetDlgItem(hPageDialog, IDC_COMBO_TRANSLATE_ENGINE);
	nCount = ::SendMessage(hwnd, CB_GETCOUNT, 0, 0);
	if(nCount > 0)
	{
		::SendMessage(hwnd, CB_SETCURSEL, 0, 0);
	}
	for(i = 0; i < nCount; i++)
	{
		LRESULT Code = ::SendMessage(hwnd, CB_GETITEMDATA, i, 0);
		if(Code == m_rWebBrowseSettings.nWebTranslateOptionCode)
		{
			::SendMessage(hwnd, CB_SETCURSEL, i, 0);
			break;
		}
	}
	::EnableWindow(hwnd, m_rWebBrowseSettings.fActive);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_WEB_BROWSE_HOT_KEY);
	::SetWindowTextA(hwnd, m_rWebBrowseSettings.HotKey);
	::EnableWindow(hwnd, m_rWebBrowseSettings.fActive);

	// clipboard swap
	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ENABLE_CLIPBOARD_SWAP);
	if(m_rClipboardSwapSettings.fActive)
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_CLIPBOARD_SWAP_HOT_KEY);
	::SetWindowTextA(hwnd, m_rClipboardSwapSettings.HotKey);
	::EnableWindow(hwnd, m_rClipboardSwapSettings.fActive);
}

void HotKeyConfigPage::Store()
{
	HWND hPageDialog = GetDialogHandle();

	HWND hwnd;

	// text conversion settings
	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ENABLE_TEXT_CONVERSION);
	m_rTextConversionSettings.fActive = (::SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);
	
	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_TEXT_CONVERSION_HOT_KEY);
	::GetWindowTextA(hwnd, m_rTextConversionSettings.HotKey, sizeof(m_rTextConversionSettings.HotKey) / sizeof(m_rTextConversionSettings.HotKey[0]));

	// web search settings
	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ENABLE_WEB_BROWSE);
	m_rWebBrowseSettings.fActive = (::SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);

	hwnd = ::GetDlgItem(hPageDialog, IDC_COMBO_SEARCH_ENGINE);
	LRESULT nIndex = ::SendMessage(hwnd, CB_GETCURSEL, 0, 0);
	if(nIndex == CB_ERR)
	{
		nIndex = 0;
	}
	m_rWebBrowseSettings.nWebSearchOptionCode = (int)::SendMessage(hwnd, CB_GETITEMDATA, nIndex, 0);

	hwnd = ::GetDlgItem(hPageDialog, IDC_COMBO_TRANSLATE_ENGINE);
	nIndex = ::SendMessage(hwnd, CB_GETCURSEL, 0, 0);
	if(nIndex == CB_ERR)
	{
		nIndex = 0;
	}
	m_rWebBrowseSettings.nWebTranslateOptionCode = (int)::SendMessage(hwnd, CB_GETITEMDATA, nIndex, 0);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_WEB_BROWSE_HOT_KEY);
	::GetWindowTextA(hwnd, m_rWebBrowseSettings.HotKey, sizeof(m_rWebBrowseSettings.HotKey) / sizeof(m_rWebBrowseSettings.HotKey[0]));

	// clipboard swap settings
	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ENABLE_CLIPBOARD_SWAP);
	m_rClipboardSwapSettings.fActive = (::SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);
	
	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_CLIPBOARD_SWAP_HOT_KEY);
	::GetWindowTextA(hwnd, m_rClipboardSwapSettings.HotKey, sizeof(m_rClipboardSwapSettings.HotKey) / sizeof(m_rClipboardSwapSettings.HotKey[0]));
}

void HotKeyConfigPage::DialogInit(HWND hwndDlg)
{
	HWND hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_SEARCH_ENGINE);
	if(hCombo)
	{
		// fill in search engine options
		AddComboItem(hCombo, _T("Google"), WEB_SEARCH_GOOGLE);
		AddComboItem(hCombo, _T("Yahoo"), WEB_SEARCH_YAHOO);
		AddComboItem(hCombo, _T("Bing"), WEB_SEARCH_BING);
		AddComboItem(hCombo, _T("Ask"), WEB_SEARCH_ASK);
	}

	hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TRANSLATE_ENGINE);
	if(hCombo)
	{
		// fill in search engine options
		AddComboItem(hCombo, _T("Google"), WEB_TRANSLATE_GOOGLE);
		AddComboItem(hCombo, _T("Babylon"), WEB_TRANSLATE_BABYLON);
		AddComboItem(hCombo, _T("Bing"), WEB_TRANSLATE_BING);
	}
}

void HotKeyConfigPage::HandleMessage(HWND hwnd, UINT uMsg, WPARAM UNUSED(wParam), LPARAM lParam)
{
	if(uMsg == WM_COMMAND)
	{
		if((HWND)lParam == ::GetDlgItem(hwnd, IDC_CHECK_ENABLE_TEXT_CONVERSION))
		{
			bool fChecked = (::SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
			::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_TEXT_CONVERSION_HOT_KEY), fChecked);
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_CHECK_ENABLE_WEB_BROWSE))
		{
			bool fChecked = (::SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
			::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_WEB_BROWSE_HOT_KEY), fChecked);
			::EnableWindow(::GetDlgItem(hwnd, IDC_COMBO_SEARCH_ENGINE), fChecked);
			::EnableWindow(::GetDlgItem(hwnd, IDC_COMBO_TRANSLATE_ENGINE), fChecked);
		}
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_CHECK_ENABLE_CLIPBOARD_SWAP))
		{
			bool fChecked = (::SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
			::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_CLIPBOARD_SWAP_HOT_KEY), fChecked);
		}
	}
}

bool HotKeyConfigPage::AddComboItem(HWND hCombo, LPCTSTR Text, int Code)
{
	LRESULT nIndex = ::SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)Text);
	if(nIndex == CB_ERR)
		return false;
	if(::SendMessage(hCombo, CB_SETITEMDATA, nIndex, Code) == CB_ERR)
		return false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CalculatorConfigPage
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ConfigPage *CalculatorConfigPage::Create(HWND hParent, AquaLangCalculatorSettings &rSettings)
{
	CalculatorConfigPage *pNew = new CalculatorConfigPage(rSettings);
	if(!pNew)
		return NULL;
	if(!pNew->OpenDialog(_T(""), IDD_DIALOG_CALC_SETTINGS, hParent))
	{
		delete pNew;
		return NULL;
	}
	return pNew;
}

CalculatorConfigPage::CalculatorConfigPage(AquaLangCalculatorSettings &rSettings)
	: m_rSettings(rSettings)
{
}

CalculatorConfigPage::~CalculatorConfigPage()
{
	CloseDialog();
}

void CalculatorConfigPage::Load()
{
	HWND hPageDialog = GetDialogHandle();
	if(!hPageDialog)
		return;

	HWND hwnd;

	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ENABLE_CALCULATOR);
	if(m_rSettings.fActive)
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_CALCULATOR_HOT_KEY);
	::SetWindowTextA(hwnd, m_rSettings.HotKey);
	::EnableWindow(hwnd, m_rSettings.fActive);

	TCHAR WindowText[256];

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_CALC_TIP_DURATION);
	_stprintf_s(WindowText, sizeof(WindowText) / sizeof(WindowText[0]), _T("%d"), m_rSettings.ApearanceDurationIn100msec);
	::SetWindowText(hwnd, WindowText);
	::EnableWindow(hwnd, m_rSettings.fActive);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_CALC_KB_PAUSE_BEFORE_TIP);
	_stprintf_s(WindowText, sizeof(WindowText) / sizeof(WindowText[0]), _T("%d"), m_rSettings.KbPauseBeforePopupIn100msec);
	::SetWindowText(hwnd, WindowText);
	::EnableWindow(hwnd, m_rSettings.fActive);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_CALCULATOR_POPUP_HOT_KEY);
	::SetWindowTextA(hwnd, m_rSettings.PopupHotKey);
	::EnableWindow(hwnd, m_rSettings.fActive);
}

void CalculatorConfigPage::Store()
{
	HWND hPageDialog = GetDialogHandle();
	if(!hPageDialog)
		return;

	HWND hwnd;

	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ENABLE_CALCULATOR);
	m_rSettings.fActive = (::SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_CALCULATOR_HOT_KEY);
	::GetWindowTextA(hwnd, m_rSettings.HotKey, sizeof(m_rSettings.HotKey) / sizeof(m_rSettings.HotKey[0]));

	TCHAR WindowText[256];
	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_CALC_TIP_DURATION);
	::GetWindowText(hwnd, WindowText, sizeof(WindowText) / sizeof(WindowText[0]));
	_stscanf_s(WindowText, _T("%d"), &m_rSettings.ApearanceDurationIn100msec);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_CALC_KB_PAUSE_BEFORE_TIP);
	::GetWindowText(hwnd, WindowText, sizeof(WindowText) / sizeof(WindowText[0]));
	_stscanf_s(WindowText, _T("%d"), &m_rSettings.KbPauseBeforePopupIn100msec);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_CALCULATOR_POPUP_HOT_KEY);
	::GetWindowTextA(hwnd, m_rSettings.PopupHotKey, sizeof(m_rSettings.PopupHotKey) / sizeof(m_rSettings.PopupHotKey[0]));
}

void CalculatorConfigPage::HandleMessage(HWND hwnd, UINT uMsg, WPARAM UNUSED(wParam), LPARAM lParam)
{
	if(uMsg == WM_COMMAND)
	{
		if((HWND)lParam == ::GetDlgItem(hwnd, IDC_CHECK_ENABLE_CALCULATOR))
		{
			bool fChecked = (::SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
			::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_CALC_TIP_DURATION), fChecked);
			::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_CALCULATOR_HOT_KEY), fChecked);
			::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_CALC_KB_PAUSE_BEFORE_TIP), fChecked);
			::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_CALCULATOR_POPUP_HOT_KEY), fChecked);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GuessLanguageConfigPage
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ConfigPage *GuessLanguageConfigPage::Create(HWND hParent, AquaLangGuessLanguageSettings &rSettings)
{
	GuessLanguageConfigPage *pNew = new GuessLanguageConfigPage(rSettings);
	if(!pNew)
		return NULL;
	if(!pNew->OpenDialog(_T(""), IDD_DIALOG_GUESS_LANGUAGE_SETTINGS, hParent))
	{
		delete pNew;
		return NULL;
	}
	return pNew;
}

GuessLanguageConfigPage::GuessLanguageConfigPage(AquaLangGuessLanguageSettings &rSettings)
	: m_rSettings(rSettings)
{
}

GuessLanguageConfigPage::~GuessLanguageConfigPage()
{
	CloseDialog();
}

void GuessLanguageConfigPage::Load()
{
	HWND hPageDialog = GetDialogHandle();
	if(!hPageDialog)
		return;

	HWND hwnd;

	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ENABLE_GUESSLANGUAGE);
	if(m_rSettings.fActive)
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	TCHAR WindowText[256];

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_GUESSLANGUAGE_TIP_DURATION);
	_stprintf_s(WindowText, sizeof(WindowText) / sizeof(WindowText[0]), _T("%d"), m_rSettings.ApearanceDurationIn100msec);
	::SetWindowText(hwnd, WindowText);
	::EnableWindow(hwnd, m_rSettings.fActive);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_GUESSLANGUAGE_KB_PAUSE_BEFORE_TIP);
	_stprintf_s(WindowText, sizeof(WindowText) / sizeof(WindowText[0]), _T("%d"), m_rSettings.KbPauseBeforePopupIn100msec);
	::SetWindowText(hwnd, WindowText);
	::EnableWindow(hwnd, m_rSettings.fActive);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_GUESSLANGUAGE_MINIMAL_NUMBER_OF_CHARACTERS);
	_stprintf_s(WindowText, sizeof(WindowText) / sizeof(WindowText[0]), _T("%d"), m_rSettings.MinimalNumberOfCharacters);
	::SetWindowText(hwnd, WindowText);
	::EnableWindow(hwnd, m_rSettings.fActive);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_GUESSLANGUAGE_MAXIMAL_NUMBER_OF_CHARACTERS);
	_stprintf_s(WindowText, sizeof(WindowText) / sizeof(WindowText[0]), _T("%d"), m_rSettings.MaximalNumberOfCharacters);
	::SetWindowText(hwnd, WindowText);
	::EnableWindow(hwnd, m_rSettings.fActive);
}

void GuessLanguageConfigPage::Store()
{
	HWND hPageDialog = GetDialogHandle();
	if(!hPageDialog)
		return;

	HWND hwnd;

	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ENABLE_GUESSLANGUAGE);
	m_rSettings.fActive = (::SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);

	TCHAR WindowText[256];
	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_GUESSLANGUAGE_TIP_DURATION);
	::GetWindowText(hwnd, WindowText, sizeof(WindowText) / sizeof(WindowText[0]));
	_stscanf_s(WindowText, _T("%d"), &m_rSettings.ApearanceDurationIn100msec);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_GUESSLANGUAGE_KB_PAUSE_BEFORE_TIP);
	::GetWindowText(hwnd, WindowText, sizeof(WindowText) / sizeof(WindowText[0]));
	_stscanf_s(WindowText, _T("%d"), &m_rSettings.KbPauseBeforePopupIn100msec);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_GUESSLANGUAGE_MINIMAL_NUMBER_OF_CHARACTERS);
	::GetWindowText(hwnd, WindowText, sizeof(WindowText) / sizeof(WindowText[0]));
	_stscanf_s(WindowText, _T("%d"), &m_rSettings.MinimalNumberOfCharacters);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_GUESSLANGUAGE_MAXIMAL_NUMBER_OF_CHARACTERS);
	::GetWindowText(hwnd, WindowText, sizeof(WindowText) / sizeof(WindowText[0]));
	_stscanf_s(WindowText, _T("%d"), &m_rSettings.MaximalNumberOfCharacters);
}

void GuessLanguageConfigPage::HandleMessage(HWND hwnd, UINT uMsg, WPARAM UNUSED(wParam), LPARAM lParam)
{
	if(uMsg == WM_COMMAND)
	{
		if((HWND)lParam == ::GetDlgItem(hwnd, IDC_CHECK_ENABLE_GUESSLANGUAGE))
		{
			bool fChecked = (::SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
			::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_GUESSLANGUAGE_TIP_DURATION), fChecked);
			::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_GUESSLANGUAGE_KB_PAUSE_BEFORE_TIP), fChecked);
			::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_GUESSLANGUAGE_MINIMAL_NUMBER_OF_CHARACTERS), fChecked);
			::EnableWindow(::GetDlgItem(hwnd, IDC_EDIT_GUESSLANGUAGE_MAXIMAL_NUMBER_OF_CHARACTERS), fChecked);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DebugConfigPage
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ConfigPage *DebugConfigPage::Create(HWND hParent, AquaLangConfiguration &rSettings)
{
	DebugConfigPage *pNew = new DebugConfigPage(rSettings);
	if(!pNew)
		return NULL;
	if(!pNew->OpenDialog(_T(""), IDD_DIALOG_AQUALANG_DEBUG_MODE, hParent))
	{
		delete pNew;
		return NULL;
	}
	return pNew;
}

DebugConfigPage::DebugConfigPage(AquaLangConfiguration &rSettings)
	: m_rUserInfoSettings(rSettings.UserInfoSettings),
	m_rVersionUpdateSettings(rSettings.VersionUpdateSettings)
{
}

DebugConfigPage::~DebugConfigPage()
{
	CloseDialog();
}

void DebugConfigPage::Load()
{
	HWND hPageDialog = GetDialogHandle();
	if(!hPageDialog)
		return;

	HWND hwnd;
	TCHAR WindowText[256];

	// user info settings
	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_SERVER_URL);
	::SetWindowTextA(hwnd, m_rUserInfoSettings.ServerUrl);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_NOTIFICATION_INTERVAL);
	_stprintf_s(WindowText, sizeof(WindowText) / sizeof(WindowText[0]), _T("%d"), m_rUserInfoSettings.NotificationIntervalMinutes);
	::SetWindowText(hwnd, WindowText);

	// version update settings
	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_DOWNLOAD_SERVER_URL);
	::SetWindowTextA(hwnd, m_rVersionUpdateSettings.ServerUrl);

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_VERSION_UPDATE_CHECK_INTERVAL);
	_stprintf_s(WindowText, sizeof(WindowText) / sizeof(WindowText[0]), _T("%d"), m_rVersionUpdateSettings.UpdateCheckIntervalMinutes);
	::SetWindowText(hwnd, WindowText);
}

void DebugConfigPage::Store()
{
	HWND hPageDialog = GetDialogHandle();
	if(!hPageDialog)
		return;

	HWND hwnd;
	TCHAR WindowText[256];

	// user info settings
	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_SERVER_URL);
	::GetWindowTextA(hwnd, m_rUserInfoSettings.ServerUrl, sizeof(m_rUserInfoSettings.ServerUrl) / sizeof(m_rUserInfoSettings.ServerUrl[0]));

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_NOTIFICATION_INTERVAL);
	::GetWindowText(hwnd, WindowText, sizeof(WindowText) / sizeof(WindowText[0]));
	_stscanf_s(WindowText, _T("%d"), &m_rUserInfoSettings.NotificationIntervalMinutes);

	// version update settings
	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_DOWNLOAD_SERVER_URL);
	::GetWindowTextA(hwnd, m_rVersionUpdateSettings.ServerUrl, sizeof(m_rVersionUpdateSettings.ServerUrl) / sizeof(m_rVersionUpdateSettings.ServerUrl[0]));

	hwnd = ::GetDlgItem(hPageDialog, IDC_EDIT_VERSION_UPDATE_CHECK_INTERVAL);
	::GetWindowText(hwnd, WindowText, sizeof(WindowText) / sizeof(WindowText[0]));
	_stscanf_s(WindowText, _T("%d"), &m_rVersionUpdateSettings.UpdateCheckIntervalMinutes);
}

void DebugConfigPage::HandleMessage(HWND UNUSED(hwnd), UINT UNUSED(uMsg), WPARAM UNUSED(wParam), LPARAM UNUSED(lParam))
{
	// do nothing
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VersionConfigPage
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ConfigPage *VersionConfigPage::Create(HWND hParent, AquaLangUpdate &rVersionUpdateSettings, HWND hListener)
{
	VersionConfigPage *pNew = new VersionConfigPage(rVersionUpdateSettings, hListener);
	if(!pNew)
		return NULL;
	if(!pNew->OpenDialog(_T(""), IDD_DIALOG_VERSION_UPDATE, hParent))
	{
		delete pNew;
		return NULL;
	}
	return pNew;
}

VersionConfigPage::VersionConfigPage(AquaLangUpdate &rVersionUpdateSettings, HWND hListener)
	: m_rVersionUpdateSettings(rVersionUpdateSettings),
	m_hListener(hListener)
{
}

VersionConfigPage::~VersionConfigPage()
{
	CloseDialog();
}

void VersionConfigPage::Load()
{
	HWND hPageDialog = GetDialogHandle();
	if(!hPageDialog)
		return;

	HWND hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ACCEPT_BETA_VERSION);
	if(m_rVersionUpdateSettings.fCheckForBetaVersion)
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}
}

void VersionConfigPage::Store()
{
	HWND hPageDialog = GetDialogHandle();
	if(!hPageDialog)
		return;

	HWND hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_ACCEPT_BETA_VERSION);
	m_rVersionUpdateSettings.fCheckForBetaVersion = (::SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);
}

void VersionConfigPage::HandleMessage(HWND hwnd, UINT uMsg, WPARAM UNUSED(wParam), LPARAM lParam)
{
	if(uMsg == WM_COMMAND)
	{
		// On About
		if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_ABOUT))
		{
			::PostMessage(m_hListener, WM_AQUALANG_MSG, WPARAM_ABOUT, 0);
		}
		// version update
		else if((HWND)lParam == ::GetDlgItem(hwnd, IDC_BUTTON_CHECK_UPDATES))
		{
			::PostMessage(m_hListener, WM_AQUALANG_MSG, WPARAM_VERSION_UPDATE, 0);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SystemPage
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ConfigPage *SystemPage::Create(HWND hParent, AquaLangNotificationArea &rNotificationArea, bool &rfAutoStart)
{
	SystemPage *pNew = new SystemPage(rNotificationArea, rfAutoStart);
	if(!pNew)
		return NULL;
	if(!pNew->OpenDialog(_T(""), IDD_DIALOG_SYSTEM_SETTINGS, hParent))
	{
		delete pNew;
		return NULL;
	}
	return pNew;
}

SystemPage::SystemPage(AquaLangNotificationArea &rNotificationArea, bool &rfAutoStart)
	: m_rNotificationArea(rNotificationArea),
	m_rfAutoStart(rfAutoStart)
{
}

SystemPage::~SystemPage()
{
	CloseDialog();
}

void SystemPage::Load()
{
	HWND hPageDialog = GetDialogHandle();
	if(!hPageDialog)
		return;

	HWND hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_SHOW_ICON_IN_NOTIFCATION_AREA);
	if(m_rNotificationArea.fShowIcon)
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_SHOW_TIP_OF_THE_DAY);
	if(m_rNotificationArea.fShowTipOfTheDay)
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	::EnableWindow(hwnd, m_rNotificationArea.fShowIcon);
	if(!m_rNotificationArea.fShowIcon)
	{
		::SendMessage(hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	// auto run
	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_AUTORUN);
	::SendMessage(hwnd, BM_SETCHECK, m_rfAutoStart ? BST_CHECKED : BST_UNCHECKED, 0);
}

void SystemPage::Store()
{
	HWND hPageDialog = GetDialogHandle();
	if(!hPageDialog)
		return;

	HWND hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_SHOW_ICON_IN_NOTIFCATION_AREA);
	m_rNotificationArea.fShowIcon = (::SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);

	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_SHOW_TIP_OF_THE_DAY);
	m_rNotificationArea.fShowTipOfTheDay = (::SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);

	// auto run
	hwnd = ::GetDlgItem(hPageDialog, IDC_CHECK_AUTORUN);
	m_rfAutoStart = (::SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);
}

void SystemPage::HandleMessage(HWND hwnd, UINT uMsg, WPARAM UNUSED(wParam), LPARAM lParam)
{
	if(uMsg == WM_COMMAND)
	{
		if((HWND)lParam == ::GetDlgItem(hwnd, IDC_CHECK_SHOW_ICON_IN_NOTIFCATION_AREA))
		{
			bool fChecked = (::SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);

			HWND hShowTipCheck = ::GetDlgItem(hwnd, IDC_CHECK_SHOW_TIP_OF_THE_DAY);
			::EnableWindow(hShowTipCheck, fChecked);
			if(!fChecked)
			{
				::SendMessage(hShowTipCheck, BM_SETCHECK, BST_UNCHECKED, 0);
			}
		}
	}
}
