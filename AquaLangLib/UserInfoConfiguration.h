#ifndef _USER_INFO_CONFIGURATION_HEADER_
#define _USER_INFO_CONFIGURATION_HEADER_

#include "xmlStructure.h"
#include "xmlValueTypes.h"

struct AquaLangStatistics;

class UserInfoConfiguration
{
public:
	UserInfoConfiguration();
	virtual ~UserInfoConfiguration();

	void Open(LPCTSTR UserName);

	bool Load();
	void Store();

	bool VerifyVersion(LPCTSTR VersionString) const;
	void IncreaseFocusEvents();
	void IncreaseLanguageChangeEvents();
	void IncreaseLanguageManualChangeEvents();
	void IncreaseLanguageManualFixEvents();
	void IncreaseTextLanguageChangeEvents();
	void IncreaseWebSearchEvents();
	void IncreaseCalcRequestEvents();
	void NotifyWatchdogRecovery();
	void IncreaseGuessLanguageAdviseEvents();
	void IncreaseGuessLanguageAcceptEvents();

	bool FillStatistics(AquaLangStatistics &rStatistics);

	xmlString m_Version;
	xmlInt m_FocusEvents;
	xmlInt m_LanguageChangeEvents;
	xmlInt m_ManualLanguageChangeEvents;
	xmlInt m_ManualLanguageFixEvents;
	xmlInt m_TextLanguageChangeEvents;
	xmlInt m_WebSearchEvents;
	xmlInt m_CalcRequestEvents;
	xmlInt m_WatchdogRecoveryEvents;
	xmlInt m_GuessLanguageAdviseEvents;
	xmlInt m_GuessLanguageAcceptEvents;
	xmlInt m_SequenceNumber;

	xmlTime m_NextTimeToNotify;
private:
	xmlTree m_xmlTree;
	xmlStructure m_UserInfoStruct;
};

#endif // _USER_INFO_CONFIGURATION_HEADER_