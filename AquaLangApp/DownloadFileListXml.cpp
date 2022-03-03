#include "Pch.h"
#include "DownloadFileListXml.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DownloadFileInfoXml
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DownloadFileInfoXml::DownloadFileInfoXml(int StructNameId)
	: xmlStructure(StructNameId),
	m_FileName(IDS_STRING_LIST_FILES_FILE_NAME_ATTRIBUTE, NULL, true), // attribute
	m_FileVersion(IDS_STRING_LIST_FILES_FILE_VERSION_ATTRIBUTE, NULL, true) // attribute
{
	AddItem(m_FileName);
	AddItem(m_FileVersion);
}

DownloadFileInfoXml::~DownloadFileInfoXml()
{
}

void DownloadFileInfoXml::Release()
{
	delete this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DownloadFileArray
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DownloadFileArray::DownloadFileArray()
	: xmlItemArray<DownloadFileInfoXml>(0)
{
	SetItemNameId(IDS_STRING_LIST_FILES_FILE_NODE);
}

DownloadFileArray::~DownloadFileArray()
{
}

DownloadFileInfoXml *DownloadFileArray::CreateFromXml(
						XmlNode & UNUSED(rxItemNode),
						int NameId
						)
{
	DownloadFileInfoXml *pNew = new DownloadFileInfoXml(NameId);
	if(!pNew)
		return NULL;
	return pNew;
}

DownloadFileInfoXml *DownloadFileArray::CreateFromBinStorage(
						ReadableBinStorage &rBinStorage,
						int NameId
						)
{
	DownloadFileInfoXml *pNew = new DownloadFileInfoXml(NameId);
	if(!pNew)
		return NULL;
	if(!pNew->GetXmlItem().Deserialize(rBinStorage))
	{
		delete pNew;
		return NULL;
	}
	return pNew;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DownloadFileInfoXml
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DownloadFileListXml::DownloadFileListXml()
	: m_xmlTree(),
	m_ConfigStruct(IDS_STRING_LIST_FILES_MAIN_NODE),
	m_ModifiedDate(IDS_STRING_LIST_FILES_LIST_DATE_ATTRIBUTE, NULL, true), // attribute
	m_ItemList()
{
	m_ConfigStruct.AddItem(m_ModifiedDate);
	m_ConfigStruct.AddItem(m_ItemList);
	m_xmlTree.Initialize(m_ConfigStruct, _T(""), 0);
}

DownloadFileListXml::~DownloadFileListXml()
{
	m_xmlTree.Clean();
}

bool DownloadFileListXml::Load(LPCTSTR xml)
{
	return m_xmlTree.LoadXml(xml);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FileDescriptionXml
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FileDescriptionXml::FileDescriptionXml()
	: m_xmlTree(),
	m_DescriptionStruct(IDS_STRING_FILE_INFO_MAIN_NODE),
	m_FileName(IDS_STRING_LIST_FILES_FILE_NAME_ATTRIBUTE, NULL, true), // attribute
	m_FileVersion(IDS_STRING_LIST_FILES_FILE_VERSION_ATTRIBUTE, NULL, true), // attribute
	m_DateAdded(IDS_STRING_FILE_INFO_DATE_ADDED_ATTRIBUTE, NULL, true), // attribute
	m_FileChecksum(IDS_STRING_FILE_INFO_CHECKSUM_ATTRIBUTE, NULL, true), // attribute
	m_FileDescription(0)
{
	m_DescriptionStruct.AddItem(m_FileName);
	m_DescriptionStruct.AddItem(m_FileVersion);
	m_DescriptionStruct.AddItem(m_DateAdded);
	m_DescriptionStruct.AddItem(m_FileChecksum);
	m_DescriptionStruct.AddItem(m_FileDescription);

	m_xmlTree.Initialize(m_DescriptionStruct, _T(""), 0);
}

FileDescriptionXml::~FileDescriptionXml()
{
}

bool FileDescriptionXml::Load(LPCTSTR xml)
{
	return m_xmlTree.LoadXml(xml);
}
