#include "Pch.h"
#include "UserInfoConfiguration.h"
#include "ApplicationDataPath.h"
#include "AquaLangStatistics.h"
#include "resource.h"

#define USERINFOFILENAME _T("UserInfo.xml")

UserInfoConfiguration::UserInfoConfiguration()
	: m_UserInfoStruct(IDS_STRING_USERINFO),
	m_Version(IDS_STRING_VERSION),
	m_FocusEvents(IDS_STRING_FOCUSEVENTS),
	m_LanguageChangeEvents(IDS_STRING_LANGUAGECHANGEEVENTS),
	m_ManualLanguageChangeEvents(IDS_STRING_MANUALLANGUAGECHANGEEVENTS),
	m_ManualLanguageFixEvents(IDS_STRING_MANUALLANGUAGEFIXEVENTS),
	m_TextLanguageChangeEvents(IDS_STRING_TEXTLANGUAGECHANGEEVENTS),
	m_WebSearchEvents(IDS_STRING_WEBSEARCHEVENTS),
	m_CalcRequestEvents(IDS_STRING_CALCREQUESTEVENTS),
	m_WatchdogRecoveryEvents(IDS_STRING_WATCHDOGRECOVERYEVENTS),
	m_NextTimeToNotify(IDS_STRING_NOTIFICATIONTIMEFIELD),
	m_GuessLanguageAdviseEvents(IDS_STRING_GUESSLANGUAGEADVISEEVENTS),
	m_GuessLanguageAcceptEvents(IDS_STRING_GUESSLANGUAGEACCEPTEVENTS),
	m_SequenceNumber(IDS_STRING_USERINFOSEQUENCENNUMBER)
{
	m_UserInfoStruct.AddItem(m_Version);
	m_UserInfoStruct.AddItem(m_FocusEvents);
	m_UserInfoStruct.AddItem(m_LanguageChangeEvents);
	m_UserInfoStruct.AddItem(m_ManualLanguageChangeEvents);
	m_UserInfoStruct.AddItem(m_ManualLanguageFixEvents);
	m_UserInfoStruct.AddItem(m_TextLanguageChangeEvents);
	m_UserInfoStruct.AddItem(m_WebSearchEvents);
	m_UserInfoStruct.AddItem(m_CalcRequestEvents);
	m_UserInfoStruct.AddItem(m_NextTimeToNotify);
	m_UserInfoStruct.AddItem(m_WatchdogRecoveryEvents);
	m_UserInfoStruct.AddItem(m_GuessLanguageAdviseEvents);
	m_UserInfoStruct.AddItem(m_GuessLanguageAcceptEvents);
	m_UserInfoStruct.AddItem(m_SequenceNumber);
}

UserInfoConfiguration::~UserInfoConfiguration()
{
}

void UserInfoConfiguration::Open(LPCTSTR UserName)
{
	ApplicationDataPath PathFinder(UserName);
	_tstring FilePath;
	PathFinder.ComposePath(USERINFOFILENAME, FilePath);

	m_xmlTree.Initialize(m_UserInfoStruct, FilePath.c_str(), IDS_STRING_ROOTFIELD);
}

bool UserInfoConfiguration::Load()
{
	return m_xmlTree.LoadXml();
}

void UserInfoConfiguration::Store()
{
	m_xmlTree.StoreXml(ENCODING_TYPE_UTF_16);
}

bool UserInfoConfiguration::VerifyVersion(LPCTSTR VersionString) const
{
	if(!m_Version.IsValid())
		return false;
	_tstring AssignedString;
	if(_tcscmp(m_Version.GetAssignedString(AssignedString), VersionString) != 0)
		return false;
	return true;
}

void UserInfoConfiguration::IncreaseFocusEvents()
{
	m_FocusEvents.SetInt(m_FocusEvents.GetInt(0) + 1);
}

void UserInfoConfiguration::IncreaseLanguageChangeEvents()
{
	m_LanguageChangeEvents.SetInt(m_LanguageChangeEvents.GetInt(0) + 1);
}

void UserInfoConfiguration::IncreaseLanguageManualChangeEvents()
{
	m_ManualLanguageChangeEvents.SetInt(m_ManualLanguageChangeEvents.GetInt(0) + 1);
}

void UserInfoConfiguration::IncreaseLanguageManualFixEvents()
{
	m_ManualLanguageFixEvents.SetInt(m_ManualLanguageFixEvents.GetInt(0) + 1);
}

void UserInfoConfiguration::IncreaseTextLanguageChangeEvents()
{
	m_TextLanguageChangeEvents.SetInt(m_TextLanguageChangeEvents.GetInt(0) + 1);
}

void UserInfoConfiguration::IncreaseWebSearchEvents()
{
	m_WebSearchEvents.SetInt(m_WebSearchEvents.GetInt(0) + 1);
}

void UserInfoConfiguration::IncreaseCalcRequestEvents()
{
	m_CalcRequestEvents.SetInt(m_CalcRequestEvents.GetInt(0) + 1);
}

void UserInfoConfiguration::NotifyWatchdogRecovery()
{
	m_WatchdogRecoveryEvents.SetInt(m_WatchdogRecoveryEvents.GetInt(0) + 1);
}

void UserInfoConfiguration::IncreaseGuessLanguageAdviseEvents()
{
	m_GuessLanguageAdviseEvents.SetInt(m_GuessLanguageAdviseEvents.GetInt(0) + 1);
}

void UserInfoConfiguration::IncreaseGuessLanguageAcceptEvents()
{
	m_GuessLanguageAcceptEvents.SetInt(m_GuessLanguageAcceptEvents.GetInt(0) + 1);
}

bool UserInfoConfiguration::FillStatistics(AquaLangStatistics &rStatistics)
{
	rStatistics.FocusEvents = m_FocusEvents.GetInt();
	rStatistics.LanguageChangeEvents = m_LanguageChangeEvents.GetInt();
	rStatistics.ManualLanguageChangeEvents = m_ManualLanguageChangeEvents.GetInt();
	rStatistics.ManualLanguageFixEvents = m_ManualLanguageFixEvents.GetInt();
	rStatistics.TextLanguageChangeEvents = m_TextLanguageChangeEvents.GetInt();
	rStatistics.WebBrowseEvents = m_WebSearchEvents.GetInt();
	rStatistics.CalcRequestEvents = m_CalcRequestEvents.GetInt();
	return true;
}
