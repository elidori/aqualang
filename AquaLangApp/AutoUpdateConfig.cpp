#include "Pch.h"
#include "AutoUpdateConfig.h"
#include "ApplicationDataPath.h"
#include "resource.h"

#define AUTOUPDATEFILENAME _T("VersionUpdateCheck.xml")

AutoUpdateConfig::AutoUpdateConfig()
	: m_AutoUpdateStruct(IDS_STRING_UPDATECHECK),
	m_NextTimeToCheck(IDS_STRING_CHECLTIMEFIELD)
{
	m_AutoUpdateStruct.AddItem(m_NextTimeToCheck);
}

AutoUpdateConfig::~AutoUpdateConfig()
{
}

void AutoUpdateConfig::Open(LPCTSTR UserName)
{
	ApplicationDataPath PathFinder(UserName);
	_tstring FilePath;
	PathFinder.ComposePath(AUTOUPDATEFILENAME, FilePath);

	m_xmlTree.Initialize(m_AutoUpdateStruct, FilePath.c_str(), IDS_STRING_ROOTFIELD);
}

bool AutoUpdateConfig::Load()
{
	return m_xmlTree.LoadXml();
}

void AutoUpdateConfig::Store()
{
	m_xmlTree.StoreXml(ENCODING_TYPE_UTF_16);
}
