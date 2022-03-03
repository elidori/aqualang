#ifndef _HTTP_CLIENT_HEADER_
#define _HTTP_CLIENT_HEADER_

class IProgressInfoReceiver;

enum HTTP_ERROR
{
	HTTP_NO_ERROR = 0,
	HTTP_INVALID_URL = -1,
	HTTP_UNKNOWN_HOST = -2,
	HTTP_OUT_OF_RESOURCES = -3,
	HTTP_PORT_BUSY = -4,
	HTTP_CANNOT_CONNECT = -5,
	HTTP_INTERNAL_ERROR = -6,
	HTTP_NO_RESPONSE = -7,
	HTTP_INVALID_MESSAGE = -8,
	HTTP_CANNOT_SEND = -9,
	HTTP_SERVER_ERROR = -10,
	HTTP_FILE_NOT_FOUND = -11,
	HTTP_SERVER_BUSY = -12,
	HTTP_INVALID_ARGS = -13,
	HTTP_INVALID_RESPONSE = -14,
	HTTP_FILE_WRITE_ERROR = -15,

	HTTP_LAST_ERROR_ENUM_VALUE = -15
};

class HttpClient
{
public:
	HttpClient(int InstanceID, LPCSTR ClientAddress = 0, int ClientPort = -1);
	virtual ~HttpClient();

	int StartSession(LPCSTR URL);

	int BuildMessageHeader(LPCSTR MessageType, std::string &Message);
	int SendRequest(LPCSTR Message);

	int ReadResponse(unsigned char *Buf, int zBuf, DWORD ResponseTimeoutSec, int &BytesRead);
	int ReadData(LPBYTE &ResponseData, int &ResponseDataLen, DWORD ResponseTimeoutSec);
	int DownloadData(LPCTSTR FileName, DWORD ResponseTimeoutSec, int DownloadReportChunk, IProgressInfoReceiver *piReceiver);

	void EndSession();

	static int URLEncode(LPCSTR Str, std::string &EncodedStr);
	static int URLEncode(LPCWSTR wStr, std::string &EncodedStr);
private:
	static int ParseURL(
					LPCSTR URL,
					std::string &HostName, 
					int &Port, 
					std::string &PageName
					);
	static int ConnectToServer(
						LPCSTR HostName,
						int ServerPort,
						LPCSTR ClientAddress,
						int ClientPort,
						SOCKET &Socket
						);
	static int BuildMessageHeader(
					LPCSTR MessageType, // GET, POST
					LPCSTR HostName, 
					int Port, 
					LPCSTR PageName,
					std::string &Message
					);
	static int SendRequest(SOCKET &s, LPCSTR Message);
	static int ReadResponse(SOCKET &s, LPBYTE Buf, int zBuf, bool fResponseHeader, DWORD ResponseTimeoutSec, int &BytesRead);
	static int ReadData(SOCKET &s, LPBYTE &ResponseData, int &ResponseDataLen, DWORD ResponseTimeoutSec);
	static int DownloadData(SOCKET &s, LPCTSTR FileName, DWORD ResponseTimeoutSec, int DownloadReportChunk, IProgressInfoReceiver *piReceiver);
	static void DisconnectFromServer(SOCKET &s);

	static int ReadFirstDataResponse(SOCKET &s, LPBYTE ReadBuffer, int ReadBufferSize, DWORD ResponseTimeoutSec, int &BytesRead, int &DataOffset, int &ResponseDataLen);

	static char hex_digit(int value);
	static int num_digits(int value);
	static bool is_all_digit(LPCSTR Str);
protected:
	int GetInstanceId() const	{ return m_InstanceID; }
private:
	const int m_InstanceID;
	std::string m_ClientAddress;
	const int m_ClientPort;

	SOCKET m_SessionSocket;
	std::string m_HostName;
	std::string m_PageName;
	int m_ServerPort;
	bool m_fResponseHeaderReceived;
};

#endif // _HTTP_CLIENT_HEADER_