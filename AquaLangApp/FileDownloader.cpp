#include "Pch.h"
#include "FileDownloader.h"
#include "StringConvert.h"
#include "HttpClient.h"

#define MAX_GET_MESSAGE_LENGTH	1024
#define WAIT_RESPONSE_TIMEOUT	30

#define LIST_FILES_PAGE			"list_files.php"
#define FILE_INFO_PAGE			"file_info.php"
#define DOWNLOAD_FILE_PAGE		"download_file.php"

 /* ------------------------------------------------------------------ *\
|	 Construct FileDownloader.											 |
|	 ServerURL is the URL of the server from which to download			 |
|	 files not including the PHP script name							 |
|	 InstanceID identifies this FileDownloader in requests to			 |
|	 the server.														 |
|	 Provide a client address and/or port for client to bind to,		 |
|	 or leave at default values of 0 and -1 to allow OS to				 |
|	 select an available address + port									 |
 \* ------------------------------------------------------------------ */
FileDownloader::FileDownloader(
					const char* ServerURL,
					int InstanceID,
					const char* ClientAddress,
					int ClientPort)
	: HttpClient(InstanceID, ClientAddress, ClientPort)
{
	size_t Len = 0;
	memset(this->_ServerURL, 0, sizeof(this->_ServerURL));

	if (ServerURL != 0 && ((Len = strlen(ServerURL)) < MAX_SERVER_URL_LEN))
	{
		memcpy(this->_ServerURL, ServerURL, Len);
	}
}

FileDownloader::~FileDownloader()
{
}

 /* ------------------------------------------------------------------ *\
|	 Get the list of avaialable files from the server					 |
|																		 |
|	 This method blocks until the a response is received from the		 |
|	 server, or an error occurs.										 |
|																		 |
|	 FileInfos is an array of zFileInfos to be allocated by the			 |
|	 caller. zFileInfos is in term of array elements (i.e. there		 |
|	 are zFileInfos elements in the array)								 |
|																		 |
|	 If not null, nFileInfos will hold the number of elements			 |
|	 filled by the method.												 |
|																		 |
|	If not NULL, ListChangeDate is considered to be of zListChangeDate	 |
|	bytes, and will hold the last modification date of the list as		 |
|	reported by the server.												 |
|																		 |
|	 Returns 0 (=FD_SUCCESS) on success or error (see above)			 |
 \* ------------------------------------------------------------------ */
int FileDownloader::ListFiles(FileInfo *FileInfos, size_t zFileInfos, size_t *nFileInfos, _tstring &ListChangeDate)
{
	char	ListFilesQuery[512];
	int		Ret					= 0;

	ListChangeDate = _T("");

	if (!FileInfos || !zFileInfos)
	{
		return FD_INVALID_ARGUMENT;
	}

	// Send the request
	memset(ListFilesQuery, 0, sizeof(ListFilesQuery));
	sprintf_s(ListFilesQuery, sizeof(ListFilesQuery),
		"%s%s?validationKey=%d&instanceId=%d", this->_ServerURL,
		LIST_FILES_PAGE, FILE_DOWNLOAD_VALIDATION_KEY, GetInstanceId());
	if ((Ret = this->_SendHTTPRequest(ListFilesQuery)) != 0)
	{
		return Ret;
	}

	LPBYTE ResponseBody	= NULL;
	int	ContentLen = 0;
	if ((Ret = ReadData(ResponseBody, ContentLen, WAIT_RESPONSE_TIMEOUT)) != 0)
	{
		// Could not read response
		EndSession();
		return Ret;
	}
	// Read entire body of response - close socket and parse the response
	EndSession();

	_tstring ResponseBodyStr;
	SetString(ResponseBodyStr, (LPCSTR)ResponseBody);
	delete[] ResponseBody;
	ResponseBody = NULL;

	DownloadFileListXml Xml;
	if(!Xml.Load(ResponseBodyStr.c_str()))
	{
		// Invalid response XML - cannot parse it
		return FD_SERVER_ERROR;
	}

	Xml.m_ModifiedDate.GetStringValue(ListChangeDate, _T(""));
	const size_t nItems = Xml.m_ItemList.GetCount();
	if(nItems > zFileInfos)
		return FD_ARRAY_TOO_SMALL;

	int RetIdx = 0;
	for(index_t i = 0; i < (index_t)nItems; i++)
	{
		const DownloadFileInfoXml *pItem = Xml.m_ItemList.GetItemFromIndex(i);
		assert(pItem);

		_tstring FileName;
		pItem->m_FileName.GetStringValue(FileName, _T(""));
		if(FileName.length() >= sizeof(FileInfos[RetIdx].FileName))
			continue;
		SetString(FileInfos[RetIdx].FileName, sizeof(FileInfos[RetIdx].FileName), FileName.c_str());

		_tstring FileVersion;
		pItem->m_FileVersion.GetStringValue(FileVersion, _T(""));
		if(FileVersion.length() >= sizeof(FileInfos[RetIdx].Version))
			continue;
		SetString(FileInfos[RetIdx].Version, sizeof(FileInfos[RetIdx].Version), FileVersion.c_str());

		RetIdx++;
	}

	if (nFileInfos)
	{
		*nFileInfos = RetIdx;
	}
	return FD_SUCCESS;
}

 /* ------------------------------------------------------------------ *\
|	 Get information about a single file								 |
|																		 |
|	 This method blocks until the a response is received from the		 |
|	 server, or an error occurs.										 |
|																		 |
|	 FileName - the 0-terminated name of the file (as provided in		 |
|	 a previous return value for ListFiles())							 |
|																		 |
|	 The DetFileInfo structure is filled with the information on		 |
|	 successful return of this method.									 |
|																		 |
|	 Returns 0 (=FD_SUCCESS) on success or error (see above)			 |
 \* ------------------------------------------------------------------ */
int FileDownloader::GetFileInformation(const char* FileName, DetailedFileInfo &DetFileInfo)
{
	char	FileInfoQuery[512];
	std::string URLEncFileName;
	int		ContentLen			= 0;
	
	if (!FileName)
	{
		return FD_INVALID_ARGUMENT;
	}

	// Send the request
	int Ret = HttpClient::URLEncode(FileName, URLEncFileName);
	if(Ret != HTTP_NO_ERROR)
		return Ret;

	memset(FileInfoQuery, 0, sizeof(FileInfoQuery));
	sprintf_s(FileInfoQuery, sizeof(FileInfoQuery),
		"%s%s?validationKey=%d&instanceId=%d&fileName=%s", this->_ServerURL,
		FILE_INFO_PAGE, FILE_DOWNLOAD_VALIDATION_KEY, GetInstanceId(),
		URLEncFileName.c_str());

	if ((Ret = this->_SendHTTPRequest(FileInfoQuery)) != 0)
	{
		return Ret;
	}

	LPBYTE ResponseBody	= NULL;
	if ((Ret = ReadData(ResponseBody, ContentLen, WAIT_RESPONSE_TIMEOUT)) != 0)
	{
		EndSession();
		return Ret;
	}

	EndSession();

	_tstring ResponseBodyStr;
	SetString(ResponseBodyStr, (LPCSTR)ResponseBody);
	delete[] ResponseBody;
	ResponseBody = NULL;

	FileDescriptionXml Xml;
	if(!Xml.Load(ResponseBodyStr.c_str()))
	{
		// Invalid response XML - cannot parse it
		return FD_SERVER_ERROR;
	}

	_tstring Checksum;
	Xml.m_FileChecksum.GetStringValue(Checksum, _T(""));
	if(Checksum.length() >= sizeof(DetFileInfo.FileChecksum))
		return FD_SERVER_ERROR;
	SetString(DetFileInfo.FileChecksum, sizeof(DetFileInfo.FileChecksum), Checksum.c_str());

	_tstring DateAdded;
	Xml.m_DateAdded.GetStringValue(DateAdded, _T(""));
	if(DateAdded.length() >= sizeof(DetFileInfo.DateAdded))
		return FD_SERVER_ERROR;
	SetString(DetFileInfo.DateAdded, sizeof(DetFileInfo.DateAdded), DateAdded.c_str());

	_tstring Description;
	Xml.m_FileDescription.GetStringValue(Description, _T(""));
	if(Description.length() >= sizeof(DetFileInfo.Description))
		return FD_SERVER_ERROR;
	SetString(DetFileInfo.Description, sizeof(DetFileInfo.Description), Description.c_str());

	return FD_SUCCESS;
}

 /* ------------------------------------------------------------------ *\
|	 Download a single file												 |
|																		 |
|	 FileName - the 0-terminated name of the file (as provided in		 |
|	 a previous return value for ListFiles())							 |
|																		 |
|	 piReceiver - can be NULL or an implementation of the abstract		 |
|	 class ProgressInfoReceiver, for callbacks on the download's		 |
|	 progress															 |
|																		 |
|	 Returns 0 (=FD_SUCCESS) on success or error (see above)			 |
 \* ------------------------------------------------------------------ */
int FileDownloader::DownloadFile(
						LPCTSTR FileName,
						LPCTSTR TargetFullFileName,
						int DownloadReportChunk,
						IProgressInfoReceiver *piReceiver
						)
{
	std::string URLEncFileName;
	char	DownloadFileQuery[512];

	if (!FileName)
	{
		return FD_INVALID_ARGUMENT;
	}

	// Send the request
	int Ret = HttpClient::URLEncode(FileName, URLEncFileName);
	if(Ret != HTTP_NO_ERROR)
		return Ret;

	memset(DownloadFileQuery, 0, sizeof(DownloadFileQuery));
	sprintf_s(DownloadFileQuery, sizeof(DownloadFileQuery),
		"%s%s?validationKey=%d&instanceId=%d&fileName=%s", this->_ServerURL,
		DOWNLOAD_FILE_PAGE, FILE_DOWNLOAD_VALIDATION_KEY, GetInstanceId(),
		URLEncFileName.c_str());
	if ((Ret = this->_SendHTTPRequest(DownloadFileQuery)) != 0)
	{
		return Ret;
	}

	Ret = HttpClient::DownloadData(TargetFullFileName, WAIT_RESPONSE_TIMEOUT, DownloadReportChunk, piReceiver);
	EndSession();
	if(Ret != HTTP_NO_ERROR)
		return Ret;

	return FD_SUCCESS;
}

 /* ------------------------------------------------------------ *\
| Send a request to the server									   |
|																   |
| This method blocks until the request is successfully sent and	   |
| does not read the response. It returns the socket on which the   |
| request was made so that the response may be read by the caller  |
|
| RequestURL: In Browser-like format, i.e.						   |
| http://address:port/rest-of-url. address is either an IP		   |
| address or a host name to be resolved by Web Message Sener	   |
| package. Port is optional and defaults to 80.	The request URL	   |
| contains any query parameters that are needed for the request	   |
|																   |
| Socket: On success, and if not NULL, this argument holds the 	   |
| socket on which the request was successfully sent				   |
|																   |
| Returns 0 (=FD_SUCCESS) on success or error otherwise. The error |
| codes are the ones that should be returned to the original	   |
| public-method caller.											   |
 \* ------------------------------------------------------------ */
int FileDownloader::_SendHTTPRequest(const char *RequestURL)
{
	int Ret = StartSession(RequestURL);
	if(Ret != HTTP_NO_ERROR)
		return Ret;

	// Connected to server - build HTTP message and send
	std::string GetMsg;
	Ret = BuildMessageHeader("GET", GetMsg);
	if(Ret != HTTP_NO_ERROR)
	{
		EndSession();
		return Ret;
	}

	GetMsg += '\n';
	if(GetMsg.length() > MAX_GET_MESSAGE_LENGTH)
		return HTTP_INTERNAL_ERROR;

	Ret = SendRequest(GetMsg.c_str());
	if(Ret != HTTP_NO_ERROR)
	{
		EndSession();
		return Ret;
	}
	return HTTP_NO_ERROR;
}
