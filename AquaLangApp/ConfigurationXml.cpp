#include "Pch.h"
#include "ConfigurationXml.h"
#include "AquaLangConfiguration.h"
#include "ApplicationDataPath.h"
#include "StringConvert.h"
#include "resource.h"

#define DEFAULT_TEXT_LANGUAGE_HOTKEY "++SHIFT--"
#define DEFAULT_WEB_BROWSE_HOTKEY "++CTRL--"
#define DEFAULT_CALC_REQUEST_HOTKEY "++CTRL--"
#define DEFAULT_CALC_POPUP_HOTKEY "++NUMLOCK--"
#define DEFAULT_CLIPBOARD_SWAPPER_HOTKEY "CTRL+SHIFT+R--"

//#define DOMAIN_URL _T("http://localhost")
#define DOMAIN_URL _T("http://www.aqualang.com")

#define APPCONFIGFILENAME _T("Config.xml")


ConfigurationXml::ConfigurationXml(LPCTSTR UserName)
	: m_xmlTree(),
	m_ConfigStruct(IDS_STRING_APPCONFIG),
	m_fDebugMode(IDS_STRING_DEBUGMODE, false),
	m_FileVersion(IDS_STRING_FILEVERSION),
	m_BalloonSettings(IDS_STRING_BALLONSETTINGS),
	m_ManageDataBase(IDS_STRING_MANAGE_DATABASE),
	m_BalloonShowMode(IDS_STRING_SHOWMODE, BALLOON_SHOW_NEVER),
	m_BalloonDuration100msec(IDS_STRING_DURATION100MSEC, 9),
	m_BalloonTimeBetweenTipsMin(IDS_STRING_TIMEBETWEENTIPSMIN, 5),
	m_TextConversionSettings(IDS_STRING_TEXTCONVERSIONSETTINGS),
	m_TextConvesionActive(IDS_STRING_CONVERSIONACTIVE, true),
	m_TextConversionHotKey(IDS_STRING_CONVERSIONHOTKEY, _T(DEFAULT_TEXT_LANGUAGE_HOTKEY)),
	m_WebBrowseSettings(IDS_STRING_WEBBROWSESETTINGS),
	m_WebBrowseActive(IDS_STRING_WEBBROWSEACTIVE, false),
	m_WebSearchOption(IDS_STRING_WEBSEARCHOPTION, WEB_SEARCH_GOOGLE),
	m_WebTranslateOption(IDS_STRING_WEBTRANSLATEOPTION, WEB_TRANSLATE_GOOGLE),
	m_WebBrowseHotKey(IDS_STRING_WEBBROWSEHOTKEY, _T(DEFAULT_WEB_BROWSE_HOTKEY)),
	m_CalcSettings(IDS_STRING_CALCSETTINGS),
	m_CalcActive(IDS_STRING_CALCACTIVE, false),
	m_CalcDuration100msec(IDS_STRING_DURATION100MSEC, 30),
	m_KbPauseBeforePopupIn100msec(IDS_STRING_PAUSEBEFOREPOPUP100MSEC, 35),
	m_CalcRequestHotKey(IDS_STRING_CALCREQUESTHOTKEY, _T(DEFAULT_CALC_REQUEST_HOTKEY)),
	m_CalcPopupHotKey(IDS_STRING_CALCPOPUPHOTKEY, _T(DEFAULT_CALC_POPUP_HOTKEY)),
	m_GuessLanguageSettings(IDS_STRING_GUESSLANGUAGESETTINGS),
	m_GuessLanguageActive(IDS_STRING_GUESSLANGUAGEACTIVE, false),
	m_GuessLanguageDuration100msec(IDS_STRING_GUESSLANGUAGEDURATION100MSEC, 30),
	m_GuessLanguageKbPauseBeforePopupIn100msec(IDS_STRING_GUESSLANGUAGEPAUSEBEFOREPOPUP100MSEC, 12),
	m_GuessLanguageMinimalNumberOfCharacters(IDS_STRING_GUESSLANGUAGEMINIMALNUMBEROFCHARACTERS, 4),
	m_GuessLanguageMaximalNumberOfCharacters(IDS_STRING_GUESSLANGUAGEMAXIMALNUMBEROFCHARACTERS, 50),
	m_ClipboardSwapperSettings(IDS_STRING_CLIPBOARDSWAPPERSETTINGS),
	m_ClipboardSwapperActive(IDS_STRING_CLIPBOARDSWAPPERACTIVE, false),
	m_ClipboardSwapperHotKey(IDS_STRING_CLIPBOARDSWAPPERHOTKEY, _T(DEFAULT_CLIPBOARD_SWAPPER_HOTKEY)),
	m_UserInfoSettings(IDS_STRING_USERINFOSETTINGS),
	m_UserInfoServer(IDS_STRING_SERVERURL, DOMAIN_URL _T("/wms/submit_file.php")),
	m_NotificationIntervalMinutes(IDS_STRING_NOTIFICATIONINTERVALMIN, 60 * 24 * 7),
	m_VersionUpdateSettings(IDS_STRING_VERSIONUPDATESETTINGS),
	m_DownloadServer(IDS_STRING_DOWNLOADSERVER, DOMAIN_URL _T("/fd/")),
	m_UpdateCheckIntervalMinutes(IDS_STRING_UPDATECHECKINTERVALMIN, 60 * 24 * 7),
	m_CheckForBetaVersion(IDS_STRING_CHECKBETAVERSION, false),
	m_NotificationAreaSettings(IDS_STRING_NOTIFICATIONAREASETTINGS),
	m_ShowIconInNotificationArea(IDS_STRING_SHOWICONINNOTIFICATIONAREA, true),
	m_ShowTipOfTheDay(IDS_STRING_SHOWTIPOFTHEDAY, false)
{
	m_ConfigStruct.AddItem(m_FileVersion);
	m_ConfigStruct.AddItem(m_fDebugMode);
	m_ConfigStruct.AddItem(m_BalloonSettings);
		m_BalloonSettings.AddItem(m_ManageDataBase);
		m_BalloonSettings.AddItem(m_BalloonShowMode);
		m_BalloonSettings.AddItem(m_BalloonDuration100msec);
		m_BalloonSettings.AddItem(m_BalloonTimeBetweenTipsMin);
	m_ConfigStruct.AddItem(m_TextConversionSettings);
		m_TextConversionSettings.AddItem(m_TextConvesionActive);
		m_TextConversionSettings.AddItem(m_TextConversionHotKey);
	m_ConfigStruct.AddItem(m_WebBrowseSettings);
		m_WebBrowseSettings.AddItem(m_WebBrowseActive);
		m_WebBrowseSettings.AddItem(m_WebSearchOption);
		m_WebBrowseSettings.AddItem(m_WebTranslateOption);
		m_WebBrowseSettings.AddItem(m_WebBrowseHotKey);
	m_ConfigStruct.AddItem(m_CalcSettings);
		m_CalcSettings.AddItem(m_CalcActive);
		m_CalcSettings.AddItem(m_CalcDuration100msec);
		m_CalcSettings.AddItem(m_KbPauseBeforePopupIn100msec);
		m_CalcSettings.AddItem(m_CalcRequestHotKey);
		m_CalcSettings.AddItem(m_CalcPopupHotKey);
	m_ConfigStruct.AddItem(m_GuessLanguageSettings);
		m_GuessLanguageSettings.AddItem(m_GuessLanguageActive);
		m_GuessLanguageSettings.AddItem(m_GuessLanguageDuration100msec);
		m_GuessLanguageSettings.AddItem(m_GuessLanguageKbPauseBeforePopupIn100msec);
		m_GuessLanguageSettings.AddItem(m_GuessLanguageMinimalNumberOfCharacters);
		m_GuessLanguageSettings.AddItem(m_GuessLanguageMaximalNumberOfCharacters);
	m_ConfigStruct.AddItem(m_ClipboardSwapperSettings);
		m_ClipboardSwapperSettings.AddItem(m_ClipboardSwapperActive);
		m_ClipboardSwapperSettings.AddItem(m_ClipboardSwapperHotKey);
	m_ConfigStruct.AddItem(m_UserInfoSettings);
		m_UserInfoSettings.AddItem(m_UserInfoServer);
		m_UserInfoSettings.AddItem(m_NotificationIntervalMinutes);
	m_ConfigStruct.AddItem(m_VersionUpdateSettings);
		m_VersionUpdateSettings.AddItem(m_DownloadServer);
		m_VersionUpdateSettings.AddItem(m_UpdateCheckIntervalMinutes);
		m_VersionUpdateSettings.AddItem(m_CheckForBetaVersion);
	m_ConfigStruct.AddItem(m_NotificationAreaSettings);
		m_NotificationAreaSettings.AddItem(m_ShowIconInNotificationArea);
		m_NotificationAreaSettings.AddItem(m_ShowTipOfTheDay);

	_tstring FilePath;
	GetFileName(UserName, FilePath);
	m_xmlTree.Initialize(m_ConfigStruct, FilePath.c_str(), IDS_STRING_AQUALANGCONFIGURATION);
}

ConfigurationXml::~ConfigurationXml()
{
}

void ConfigurationXml::GetFileName(LPCTSTR UserName, _tstring &FileName)
{
	ApplicationDataPath PathFinder(UserName);
	PathFinder.ComposePath(APPCONFIGFILENAME, FileName);
}

void ConfigurationXml::Load(AquaLangConfiguration &rConfig, __int64 &rFileVersion, bool &rfDebug, bool &rfDebugModeValid)
{
	m_xmlTree.LoadXml();

	// check if in debug mode
	bool fDebug = m_fDebugMode.GetBool(false);
	VerifySettings(fDebug);

	GetCurrentConfig(rConfig, rFileVersion, rfDebug, rfDebugModeValid);
}

void ConfigurationXml::Store(const AquaLangConfiguration &rConfig, __int64 FileVersion, bool fDebug, bool fDebugModeValid, bool RebuildXml)
{
	if(RebuildXml)
	{
		m_xmlTree.Clean();
	}

	m_fDebugMode.Clean();
	if(fDebugModeValid)
	{
		m_fDebugMode.SetBool(fDebug);
	}
	m_FileVersion.SetInt(FileVersion);
	
	const AquaLangBalloonTipSettings &rBalloonConfig = rConfig.BalloonTipSettings;
	m_ManageDataBase.SetBoolIfModified(rBalloonConfig.fActive);
	m_BalloonShowMode.SetIntIfModified(rBalloonConfig.ShowMode);
	m_BalloonDuration100msec.SetIntIfModified(rBalloonConfig.ApearanceDurationIn100msec);
	m_BalloonTimeBetweenTipsMin.SetIntIfModified(rBalloonConfig.MinTimeBetweenTwoSameTipsInMin);

	const AquaLangTextCoversionSettings &rTextConversion = rConfig.TextConversionSettings;
	m_TextConvesionActive.SetBoolIfModified(rTextConversion.fActive);
	if(strlen(rTextConversion.HotKey) < sizeof(rTextConversion.HotKey))
	{
		_tstring HotKey;
		SetString(HotKey, rTextConversion.HotKey, sizeof(rTextConversion.HotKey));
		m_TextConversionHotKey.SetStringIfModified(HotKey.c_str());
	}

	const AquaLangWebBrowseSettings &rWebBrowse = rConfig.WebBrowseSettings;
	m_WebBrowseActive.SetBoolIfModified(rWebBrowse.fActive);
	m_WebSearchOption.SetIntIfModified(rWebBrowse.nWebSearchOptionCode);
	m_WebTranslateOption.SetIntIfModified(rWebBrowse.nWebTranslateOptionCode);
	if(strlen(rWebBrowse.HotKey) < sizeof(rWebBrowse.HotKey))
	{
		_tstring HotKey;
		SetString(HotKey, rWebBrowse.HotKey, sizeof(rWebBrowse.HotKey));
		m_WebBrowseHotKey.SetStringIfModified(HotKey.c_str());
	}

	const AquaLangCalculatorSettings &rCalcConfig = rConfig.CalcSettings;
	m_CalcActive.SetBoolIfModified(rCalcConfig.fActive);
	m_CalcDuration100msec.SetIntIfModified(rCalcConfig.ApearanceDurationIn100msec);
	m_KbPauseBeforePopupIn100msec.SetIntIfModified(rCalcConfig.KbPauseBeforePopupIn100msec);
	if(strlen(rCalcConfig.HotKey) < sizeof(rCalcConfig.HotKey))
	{
		_tstring HotKey;
		SetString(HotKey, rCalcConfig.HotKey, sizeof(rCalcConfig.HotKey));
		m_CalcRequestHotKey.SetStringIfModified(HotKey.c_str());
	}
	if(strlen(rCalcConfig.PopupHotKey) < sizeof(rCalcConfig.PopupHotKey))
	{
		_tstring HotKey;
		SetString(HotKey, rCalcConfig.PopupHotKey, sizeof(rCalcConfig.PopupHotKey));
		m_CalcPopupHotKey.SetStringIfModified(HotKey.c_str());
	}

	const AquaLangGuessLanguageSettings &rGuessLanguageConfig = rConfig.GuessLanguageSettings;
	m_GuessLanguageActive.SetBoolIfModified(rGuessLanguageConfig.fActive);
	m_GuessLanguageDuration100msec.SetIntIfModified(rGuessLanguageConfig.ApearanceDurationIn100msec);
	m_GuessLanguageKbPauseBeforePopupIn100msec.SetIntIfModified(rGuessLanguageConfig.KbPauseBeforePopupIn100msec);
	m_GuessLanguageMinimalNumberOfCharacters.SetIntIfModified(rGuessLanguageConfig.MinimalNumberOfCharacters);
	m_GuessLanguageMaximalNumberOfCharacters.SetIntIfModified(rGuessLanguageConfig.MaximalNumberOfCharacters);

	const AquaLangSwapSettings &rSwapSettings = rConfig.SwapSettings;
	m_ClipboardSwapperActive.SetBoolIfModified(rSwapSettings.fActive);
	if(strlen(rSwapSettings.HotKey) < sizeof(rSwapSettings.HotKey))
	{
		_tstring HotKey;
		SetString(HotKey, rSwapSettings.HotKey, sizeof(rSwapSettings.HotKey));
		m_ClipboardSwapperHotKey.SetStringIfModified(HotKey.c_str());
	}

	const AquaLangUserInfoNotification &rUserInfoConfig = rConfig.UserInfoSettings;
	m_NotificationIntervalMinutes.SetIntIfModified(rUserInfoConfig.NotificationIntervalMinutes);

	if(strlen(rUserInfoConfig.ServerUrl) < sizeof(rUserInfoConfig.ServerUrl))
	{
		_tstring Url;
		SetString(Url, rUserInfoConfig.ServerUrl, sizeof(rUserInfoConfig.ServerUrl));
		m_UserInfoServer.SetStringIfModified(Url.c_str());
	}

	const AquaLangUpdate &rVersionUpdateConfig = rConfig.VersionUpdateSettings;
	m_UpdateCheckIntervalMinutes.SetIntIfModified(rVersionUpdateConfig.UpdateCheckIntervalMinutes);

	m_CheckForBetaVersion.SetBoolIfModified(rVersionUpdateConfig.fCheckForBetaVersion);

	if(strlen(rVersionUpdateConfig.ServerUrl) < sizeof(rVersionUpdateConfig.ServerUrl))
	{
		_tstring Url;
		SetString(Url, rVersionUpdateConfig.ServerUrl, sizeof(rVersionUpdateConfig.ServerUrl));
		m_DownloadServer.SetStringIfModified(Url.c_str());
	}

	const AquaLangNotificationArea &rNotificationArea = rConfig.NotificationArea;
	m_ShowIconInNotificationArea.SetBoolIfModified(rNotificationArea.fShowIcon);
	m_ShowTipOfTheDay.SetBoolIfModified(rNotificationArea.fShowTipOfTheDay);

	VerifySettings(fDebug);

	m_xmlTree.StoreXml(ENCODING_TYPE_UTF_16);
}

void ConfigurationXml::GetCurrentConfig(AquaLangConfiguration &rConfig, __int64 &rFileVersion, bool &rfDebug, bool &rfDebugModeValid) const
{
	rfDebugModeValid = m_fDebugMode.IsValid();
	rfDebug = m_fDebugMode.GetBool(false);
	rFileVersion = m_FileVersion.GetInt(0);

	AquaLangBalloonTipSettings &rBalloonConfig = rConfig.BalloonTipSettings;
	rBalloonConfig.fActive = m_ManageDataBase.GetBool();
	rBalloonConfig.ShowMode = (BallonTipShowOptions)m_BalloonShowMode.GetInt();
	rBalloonConfig.ApearanceDurationIn100msec = (int)m_BalloonDuration100msec.GetInt();
	rBalloonConfig.MinTimeBetweenTwoSameTipsInMin = (int)m_BalloonTimeBetweenTipsMin.GetInt();

	AquaLangTextCoversionSettings &rTextConversion = rConfig.TextConversionSettings;
	rTextConversion.fActive = m_TextConvesionActive.GetBool();
	_tstring HotKey;
	m_TextConversionHotKey.GetString(HotKey);
	if(HotKey.length() < sizeof(rTextConversion.HotKey))
	{
		SetString(rTextConversion.HotKey, sizeof(rTextConversion.HotKey), HotKey.c_str());
	}
	else
	{
		strcpy_s(rTextConversion.HotKey, sizeof(rTextConversion.HotKey), DEFAULT_TEXT_LANGUAGE_HOTKEY);
	}

	AquaLangWebBrowseSettings &rWebBrowse = rConfig.WebBrowseSettings;
	rWebBrowse.fActive = m_WebBrowseActive.GetBool();
	rWebBrowse.nWebSearchOptionCode = (int)m_WebSearchOption.GetInt();
	rWebBrowse.nWebTranslateOptionCode = (int)m_WebTranslateOption.GetInt();
	m_WebBrowseHotKey.GetString(HotKey);
	if(HotKey.length() < sizeof(rWebBrowse.HotKey))
	{
		SetString(rWebBrowse.HotKey, sizeof(rWebBrowse.HotKey), HotKey.c_str());
	}
	else
	{
		strcpy_s(rWebBrowse.HotKey, sizeof(rWebBrowse.HotKey), DEFAULT_WEB_BROWSE_HOTKEY);
	}

	AquaLangCalculatorSettings &rCalcConfig = rConfig.CalcSettings;
	rCalcConfig.fActive = m_CalcActive.GetBool();
	rCalcConfig.ApearanceDurationIn100msec = (int)m_CalcDuration100msec.GetInt();
	rCalcConfig.KbPauseBeforePopupIn100msec = (int)m_KbPauseBeforePopupIn100msec.GetInt();
	m_CalcRequestHotKey.GetString(HotKey);
	if(HotKey.length() < sizeof(rCalcConfig.HotKey))
	{
		SetString(rCalcConfig.HotKey, sizeof(rCalcConfig.HotKey), HotKey.c_str());
	}
	else
	{
		strcpy_s(rCalcConfig.HotKey, sizeof(rCalcConfig.HotKey), DEFAULT_CALC_REQUEST_HOTKEY);
	}
	m_CalcPopupHotKey.GetString(HotKey);
	if(HotKey.length() < sizeof(rCalcConfig.PopupHotKey))
	{
		SetString(rCalcConfig.PopupHotKey, sizeof(rCalcConfig.PopupHotKey), HotKey.c_str());
	}
	else
	{
		strcpy_s(rCalcConfig.PopupHotKey, sizeof(rCalcConfig.PopupHotKey), DEFAULT_CALC_POPUP_HOTKEY);
	}

	AquaLangGuessLanguageSettings &rGuessLanguageConfig = rConfig.GuessLanguageSettings;
	rGuessLanguageConfig.fActive = m_GuessLanguageActive.GetBool();
	rGuessLanguageConfig.ApearanceDurationIn100msec = (int)m_GuessLanguageDuration100msec.GetInt();
	rGuessLanguageConfig.KbPauseBeforePopupIn100msec = (int)m_GuessLanguageKbPauseBeforePopupIn100msec.GetInt();
	rGuessLanguageConfig.MinimalNumberOfCharacters = (int)m_GuessLanguageMinimalNumberOfCharacters.GetInt();
	rGuessLanguageConfig.MaximalNumberOfCharacters = (int)m_GuessLanguageMaximalNumberOfCharacters.GetInt();

	AquaLangSwapSettings &rSwapSettings = rConfig.SwapSettings;
	rSwapSettings.fActive = m_ClipboardSwapperActive.GetBool();
	m_ClipboardSwapperHotKey.GetString(HotKey);
	if(HotKey.length() < sizeof(rSwapSettings.HotKey))
	{
		SetString(rSwapSettings.HotKey, sizeof(rSwapSettings.HotKey), HotKey.c_str());
	}
	else
	{
		strcpy_s(rSwapSettings.HotKey, sizeof(rSwapSettings.HotKey), DEFAULT_CLIPBOARD_SWAPPER_HOTKEY);
	}

	AquaLangUserInfoNotification &rUserInfoConfig = rConfig.UserInfoSettings;
	rUserInfoConfig.NotificationIntervalMinutes = (int)m_NotificationIntervalMinutes.GetInt();
	_tstring Url;
	m_UserInfoServer.GetString(Url);
	if(Url.length() < sizeof(rUserInfoConfig.ServerUrl))
	{
		SetString(rUserInfoConfig.ServerUrl, sizeof(rUserInfoConfig.ServerUrl), Url.c_str());
	}
	else
	{
		strcpy_s(rUserInfoConfig.ServerUrl, sizeof(rUserInfoConfig.ServerUrl), "");
	}

	AquaLangUpdate &rVersionUpdateConfig = rConfig.VersionUpdateSettings;
	rVersionUpdateConfig.UpdateCheckIntervalMinutes = (int)m_UpdateCheckIntervalMinutes.GetInt();
	rVersionUpdateConfig.fCheckForBetaVersion = m_CheckForBetaVersion.GetBool();
	m_DownloadServer.GetString(Url);
	if(Url.length() < sizeof(rVersionUpdateConfig.ServerUrl))
	{
		SetString(rVersionUpdateConfig.ServerUrl, sizeof(rVersionUpdateConfig.ServerUrl), Url.c_str());
	}
	else
	{
		strcpy_s(rVersionUpdateConfig.ServerUrl, sizeof(rVersionUpdateConfig.ServerUrl), "");
	}

	AquaLangNotificationArea &rNotificationArea = rConfig.NotificationArea;
	rNotificationArea.fShowIcon = m_ShowIconInNotificationArea.GetBool();
	rNotificationArea.fShowTipOfTheDay = m_ShowTipOfTheDay.GetBool();
}

void ConfigurationXml::VerifySettings(bool fDebugMode)
{
	if(!fDebugMode)
	{
		// invalidate unauthorized settings
		m_UserInfoServer.Clean();
		m_NotificationIntervalMinutes.Clean();

		m_DownloadServer.Clean();
		m_UpdateCheckIntervalMinutes.Clean();

		if(m_BalloonShowMode.GetInt(BALLOON_SHOW_NEVER) == BALLOON_SHOW_DEBUG)
		{
			m_BalloonShowMode.SetInt(BALLOON_SHOW_ALWAYS);
		}
	}
}
