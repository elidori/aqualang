#ifndef _DOWNLOAD_FILE_LIST_XML_HEADER_
#define _DOWNLOAD_FILE_LIST_XML_HEADER_

#include "xmlStructure.h"

// for ListFiles
class DownloadFileInfoXml : public xmlStructure
{
public:
	DownloadFileInfoXml(int StructNameId);
	virtual ~DownloadFileInfoXml();

	void Release();
public:
	xmlString m_FileName;
	xmlString m_FileVersion;
};

class DownloadFileArray : public xmlItemArray<DownloadFileInfoXml>
{
public:
	DownloadFileArray();
	virtual ~DownloadFileArray();

	virtual DownloadFileInfoXml *CreateFromXml(
						XmlNode &rxItemNode,
						int NameId
						);
	virtual DownloadFileInfoXml *CreateFromBinStorage(
						ReadableBinStorage &rBinStorage,
						int NameId
						);
};

class DownloadFileListXml
{
public:
	DownloadFileListXml();
	virtual ~DownloadFileListXml();

	bool Load(LPCTSTR xml);
public:
	xmlTree m_xmlTree;
	xmlStructure m_ConfigStruct;
	xmlString m_ModifiedDate;
	DownloadFileArray m_ItemList;
};

// for GetFileInformation
class FileDescriptionXml
{
public:
	FileDescriptionXml();
	virtual ~FileDescriptionXml();

	bool Load(LPCTSTR xml);
public:
	xmlTree m_xmlTree;
	xmlStructure m_DescriptionStruct;

	xmlString m_FileName;
	xmlString m_FileVersion;
	xmlString m_DateAdded;
	xmlString m_FileChecksum;
	xmlString m_FileDescription;
};

#endif // _DOWNLOAD_FILE_LIST_XML_HEADER_