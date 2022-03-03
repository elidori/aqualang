#ifndef _AQUALANG_CONFIGURATION_HEADER_
#define _AQUALANG_CONFIGURATION_HEADER_

#define HOT_KEY_STRING_MAX_LENGTH 128

enum BallonTipShowOptions
{
	BALLOON_SHOW_NEVER = 0,
	BALLOON_SHOW_WHEN_MODIFIED = 1,
	BALLOON_SHOW_ALWAYS = 2,
	BALLOON_SHOW_DEBUG = 3,

	BALLOON_SHOW_COUNT = 4
};

struct AquaLangBalloonTipSettings
{
	bool fActive;
	BallonTipShowOptions ShowMode;
	int ApearanceDurationIn100msec;
	int MinTimeBetweenTwoSameTipsInMin;
};

struct AquaLangTextCoversionSettings
{
	bool fActive;
	char HotKey[HOT_KEY_STRING_MAX_LENGTH];
};

enum AquaLangWebBrowsOptions
{
	WEB_SEARCH_GOOGLE = 0,
	WEB_SEARCH_YAHOO = 1,
	WEB_SEARCH_BING = 2,
	WEB_SEARCH_ASK = 3,

	WEB_TRANSLATE_GOOGLE = 4,
	WEB_TRANSLATE_BABYLON = 5,
	WEB_TRANSLATE_BING = 6
};

struct AquaLangWebBrowseSettings
{
	bool fActive;
	int nWebSearchOptionCode;
	int nWebTranslateOptionCode;
	char HotKey[HOT_KEY_STRING_MAX_LENGTH];
};

struct AquaLangCalculatorSettings
{
	bool fActive;
	int ApearanceDurationIn100msec;
	int KbPauseBeforePopupIn100msec;
	char HotKey[HOT_KEY_STRING_MAX_LENGTH];
	char PopupHotKey[HOT_KEY_STRING_MAX_LENGTH];
};

struct AquaLangGuessLanguageSettings
{
	bool fActive;
	int ApearanceDurationIn100msec;
	int KbPauseBeforePopupIn100msec;
	int MinimalNumberOfCharacters;
	int MaximalNumberOfCharacters;
};

struct AquaLangSwapSettings
{
	bool fActive;
	char HotKey[HOT_KEY_STRING_MAX_LENGTH];
};

#define URL_MAX_LENGTH 1024

struct AquaLangUserInfoNotification
{
	char ServerUrl[URL_MAX_LENGTH];
	int NotificationIntervalMinutes;
};

struct AquaLangUpdate
{
	char ServerUrl[URL_MAX_LENGTH];
	int UpdateCheckIntervalMinutes;
	bool fCheckForBetaVersion;
};

struct AquaLangNotificationArea
{
	bool fShowIcon;
	bool fShowTipOfTheDay;
};

struct AquaLangConfiguration
{
	AquaLangBalloonTipSettings BalloonTipSettings;
	AquaLangTextCoversionSettings TextConversionSettings;
	AquaLangWebBrowseSettings WebBrowseSettings;
	AquaLangCalculatorSettings CalcSettings;
	AquaLangGuessLanguageSettings GuessLanguageSettings;
	AquaLangSwapSettings SwapSettings;
	AquaLangUserInfoNotification UserInfoSettings;
	AquaLangUpdate VersionUpdateSettings;
	AquaLangNotificationArea NotificationArea;
};

#endif // _AQUALANG_CONFIGURATION_HEADER_