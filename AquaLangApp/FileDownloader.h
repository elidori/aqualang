#ifndef _FILE_DOWNLOADER_HEADER_
#define _FILE_DOWNLOADER_HEADER_

#include "HttpClient.h"
#include "DownloadFileListXml.h"

class IProgressInfoReceiver;

#define MAX_FILE_NAME_LEN			512
#define	MAX_FILE_VERSION_LEN		32
#define MAX_FILE_DATE_LEN			32
#define MAX_FILE_CHECKSUM_LEN		64
#define MAX_FILE_DESCRIPTION_LEN	1024
#define MAX_SERVER_URL_LEN			512
#define MAX_FILE_FOLDER_LEN			512
#define MAX_CLIENT_ADDRESS_LEN		32

// Error codes
enum FileDownloadErrors
{
	FD_SUCCESS = 0,
	FD_INVALID_ARGUMENT = HTTP_LAST_ERROR_ENUM_VALUE - 1,
	FD_SERVER_ERROR = HTTP_LAST_ERROR_ENUM_VALUE - 2,
	FD_ARRAY_TOO_SMALL = HTTP_LAST_ERROR_ENUM_VALUE - 3
};

// Server validation key
#define FILE_DOWNLOAD_VALIDATION_KEY	819306821

class IProgressInfoReceiver;

class FileDownloader : public HttpClient
{
public:
	typedef struct tagFileInfo {
		char	FileName[MAX_FILE_NAME_LEN];
		char	Version[MAX_FILE_VERSION_LEN];
	} FileInfo;

	typedef struct tagDetailedFileInfo {
		char	FileChecksum[MAX_FILE_CHECKSUM_LEN];
		char	DateAdded[MAX_FILE_DATE_LEN];
		char	Description[MAX_FILE_DESCRIPTION_LEN];
	} DetailedFileInfo;

	// Construct FileDownloader. 
	// ServerURL is the URL of the server from which to download 
	// files not including the PHP script name
	// InstanceID identifies this FileDownloader in requests to
	// the server.
	// LocalFileFolder is where downloaded files are placed
	// DownloadReportChunk is the number of bytes downloaded between
	// two successive progress reports made during download
	// Provide a client address and/or port for client to bind to, 
	// or leave at default values of 0 and -1 to allow OS to 
	// select an available address + port
	// -------------------------------------------------------------
	FileDownloader(
			const char* ServerURL,
			int InstanceID,
			const char* ClientAddress = 0,
			int ClientPort = -1
			);
	virtual ~FileDownloader();

	// Get the list of avaialable files from the server
	//
	// This method blocks until the a response is received from the 
	// server, or an error occurs.
	//
	// FileInfos is an array of zFileInfos to be allocated by the 
	// caller. zFileInfos is in term of array elements (i.e. there
	// are zFileInfos elements in the array)
	//
	// If not null, nFileInfos will hold the number of elements
	// filled by the method.
	//
	// If not NULL, ListChangeDate is considered to be of zListChangeDate
	// bytes, and will hold the last modification date of the list as
	// reported by the server.
	//
	// Returns 0 (=FD_SUCCESS) on success or error (see above)
	// ---------------------------------------------------------
	int ListFiles(FileInfo *FileInfos, size_t zFileInfos, size_t *nFileInfos, _tstring &ListChangeDate);

	// Get information about a single file
	//
	// This method blocks until the a response is received from the 
	// server, or an error occurs.
	//
	// FileName - the 0-terminated name of the file (as provided in 
	// a previous return value for ListFiles())
	//
	// The DetFileInfo structure is filled with the information on
	// successful return of this method.
	//
	// Returns 0 (=FD_SUCCESS) on success or error (see above)
	// -----------------------------------------------------------
	int GetFileInformation(const char* FileName, DetailedFileInfo &DetFileInfo);

	// Download a single file
	//
	// FileName - the 0-terminated name of the file (as provided in 
	// a previous return value for ListFiles())
	// 
	// TargetFullFileName - the 0-terminated name of the target file. This must not be empty
	// 
	// piReceiver - can be NULL or an implementation of the abstract
	// class IProgressInfoReceiver, for callbacks on the download's
	// progress
	//
	// Returns 0 (=FD_SUCCESS) on success or error (see above)
	// -----------------------------------------------------------
	int DownloadFile(
			LPCTSTR FileName,
			LPCTSTR TargetFullFileName,
			int DownloadReportChunk,
			IProgressInfoReceiver *piReceiver
			);
private:
 
	// Send a request to the server
	// 
	// This method blocks until the request is successfully sent and
	// does not read the response. It returns the socket on which the
	// request was made so that the response may be read by the caller
	// 
	// RequestURL: In Browser-like format, i.e.
	// http://address:port/rest-of-url. address is either an IP
	// address or a host name to be resolved by Web Message Sener
	// package. Port is optional and defaults to 80. The request URL
	// contains any query parameters that are needed for the request
	// 
	// Socket: On success, and if not NULL, this argument holds the
	// socket on which the request was successfully sent
	// 
	// Returns 0 (=FD_SUCCESS) on success or error otherwise. The error
	// codes are the ones that should be returned to the original
	// public-method caller.
	// ------------------------------------------------------------
	int _SendHTTPRequest(const char *RequestURL);
private:
	char	_ServerURL[MAX_SERVER_URL_LEN];
};

#endif // _FILE_DOWNLOADER_HEADER_