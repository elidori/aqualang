#ifndef _CONFIGURATION_XML_HEADER_
#define _CONFIGURATION_XML_HEADER_

#include "xmlStructure.h"

struct AquaLangConfiguration;

class ConfigurationXml
{
public:
	ConfigurationXml(LPCTSTR UserName);
	virtual ~ConfigurationXml();

	static void GetFileName(LPCTSTR UserName, _tstring &FileName);

	void Load(AquaLangConfiguration &rConfig, __int64 &rFileVersion, bool &rfDebug, bool &rfDebugModeValid);
	void Store(const AquaLangConfiguration &rConfig, __int64 FileVersion, bool fDebug, bool fDebugModeValid, bool RebuildXml);

	void GetCurrentConfig(AquaLangConfiguration &rConfig, __int64 &rFileVersion, bool &rfDebug, bool &rfDebugModeValid) const;
private:
	void VerifySettings(bool fDebugMode);
private:
	xmlTree m_xmlTree;
	xmlStructure m_ConfigStruct;

	xmlBool m_fDebugMode;

	xmlInt m_FileVersion;

	xmlStructure m_BalloonSettings;
	xmlBool m_ManageDataBase;
	xmlInt m_BalloonShowMode;
	xmlInt m_BalloonDuration100msec;
	xmlInt m_BalloonTimeBetweenTipsMin;

	xmlStructure m_TextConversionSettings;
	xmlBool m_TextConvesionActive;
	xmlString m_TextConversionHotKey;

	xmlStructure m_WebBrowseSettings;
	xmlBool m_WebBrowseActive;
	xmlInt m_WebSearchOption;
	xmlInt m_WebTranslateOption;
	xmlString m_WebBrowseHotKey;

	xmlStructure m_CalcSettings;
	xmlBool m_CalcActive;
	xmlInt m_CalcDuration100msec;
	xmlInt m_KbPauseBeforePopupIn100msec;
	xmlString m_CalcRequestHotKey;
	xmlString m_CalcPopupHotKey;

	xmlStructure m_GuessLanguageSettings;
	xmlBool m_GuessLanguageActive;
	xmlInt m_GuessLanguageDuration100msec;
	xmlInt m_GuessLanguageKbPauseBeforePopupIn100msec;
	xmlInt m_GuessLanguageMinimalNumberOfCharacters;
	xmlInt m_GuessLanguageMaximalNumberOfCharacters;

	xmlStructure m_ClipboardSwapperSettings;
	xmlBool m_ClipboardSwapperActive;
	xmlString m_ClipboardSwapperHotKey;

	xmlStructure m_UserInfoSettings;
	xmlString m_UserInfoServer;
	xmlInt m_NotificationIntervalMinutes;

	xmlStructure m_VersionUpdateSettings;
	xmlString m_DownloadServer;
	xmlInt m_UpdateCheckIntervalMinutes;
	xmlBool m_CheckForBetaVersion;

	xmlStructure m_NotificationAreaSettings;
	xmlBool m_ShowIconInNotificationArea;
	xmlBool m_ShowTipOfTheDay;
};

#endif // _CONFIGURATION_XML_HEADER_