#include "Pch.h"
#include "TipOfTheDayXml.h"
#include "ApplicationDataPath.h"
#include "resource.h"

// tip status
#define TIP_STATUS_FILE_NAME _T("TipOfTheDayStatus.xml")

// tip list
#define TIP_LIST_FILE_NAME _T("TipOfTheDay.xml")

////////////////////////////////////////////////////////////////////////////////////////
// TipOfTheDayStatusXml
////////////////////////////////////////////////////////////////////////////////////////
TipOfTheDayStatusXml::TipOfTheDayStatusXml()
	: m_LatestIndexPresented(IDS_STRING_LATESTTIPINDEX, -1),
	m_NextTimeToPresentTip(IDS_STRING_NEXTTIMETOPRESENTTIP),
	m_TipOfTheDayStatusStructure(0)
{
	m_TipOfTheDayStatusStructure.AddItem(m_LatestIndexPresented);
	m_TipOfTheDayStatusStructure.AddItem(m_NextTimeToPresentTip);
}

TipOfTheDayStatusXml::~TipOfTheDayStatusXml()
{
}

bool TipOfTheDayStatusXml::Open(LPCTSTR UserName)
{
	_tstring FilePath;
	ApplicationDataPath PathFinder(UserName);
	PathFinder.ComposePath(TIP_STATUS_FILE_NAME, FilePath);

	m_xmlTree.Initialize(m_TipOfTheDayStatusStructure, FilePath.c_str(), IDS_STRING_TIPSTATUSROOTFIELD);
	return m_xmlTree.LoadXml();
}

bool TipOfTheDayStatusXml::StoreState()
{
	return m_xmlTree.StoreXml(ENCODING_TYPE_UTF_16);
}

////////////////////////////////////////////////////////////////////////////////////////
// TipOfTheDayListXml
////////////////////////////////////////////////////////////////////////////////////////
TipOfTheDayListXml::TipOfTheDayListXml()
	: TipListXml(IDS_STRING_TIPLIST),
	m_TipOfTheDayXmlStructure(0)
{
	SetItemNameId(IDS_STRING_TIP);
	m_TipOfTheDayXmlStructure.AddItem(*this);
}

TipOfTheDayListXml::~TipOfTheDayListXml()
{
}

bool TipOfTheDayListXml::LoadTipList(LPCTSTR UserName)
{
	_tstring FilePath;
	ApplicationDataPath PathFinder(UserName);
	PathFinder.ComposeRelativePath(TIP_LIST_FILE_NAME, FilePath);

	m_xmlTree.Initialize(m_TipOfTheDayXmlStructure, FilePath.c_str(), IDS_STRING_TIPLISTROOTFIELD);
	return m_xmlTree.LoadXml();
}

LPCTSTR TipOfTheDayListXml::GetText(index_t nIndex, _tstring &rText) const
{
	const xmlString *pXmlString = GetItemFromIndex(nIndex);
	if(pXmlString == NULL)
		return _T("");
	return pXmlString->GetAssignedString(rText);
}

xmlString *TipOfTheDayListXml::CreateFromXml(
					XmlNode & UNUSED(rxItemNode),
					int NameId
					)
{
	xmlString *pNew = new xmlString(NameId);
	if(!pNew)
		return NULL;
	return pNew;
}

xmlString *TipOfTheDayListXml::CreateFromBinStorage(
					ReadableBinStorage &rBinStorage,
					int NameId
					)
{
	xmlString *pNew = new xmlString(NameId);
	if(!pNew)
		return NULL;
	if(!pNew->Deserialize(rBinStorage))
	{
		delete pNew;
		return NULL;
	}
	return pNew;
}
