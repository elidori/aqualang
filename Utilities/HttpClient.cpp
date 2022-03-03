#include "Pch.h"
#include "HttpClient.h"
#include "StringConvert.h"
#include "IProgressInfoReceiver.h"
#include "Log.h"

#define MAX_HOST_NAME_LEGNTH	255		// From RFC 1123
#define MAX_PAGE_NAME_LENGTH	1024	// Common practice
#define MAX_RESPONSE_DATA_LENGTH (1024 * 1024)

#define DEFAULT_SERVER_PORT 80

#define HTTP_RESPONSE_200 "HTTP/1.1 200 OK"
#define HTTP_RESPONSE_404 "HTTP/1.1 404 Not Found"
#define HTTP_RESPONSE_503 "HTTP/1.1 503 Service Unavailable"

HttpClient::HttpClient(int InstanceID, LPCSTR ClientAddress /*= 0*/, int ClientPort /*= -1*/)
	: m_InstanceID(InstanceID),
	m_ClientPort(ClientPort)
{
	if(ClientAddress)
	{
		m_ClientAddress = ClientAddress;
	}
	m_SessionSocket = INVALID_SOCKET;
	m_HostName = "";
	m_PageName = "";
	m_ServerPort = -1;
	m_fResponseHeaderReceived = false;


	// Start WinSock
	WSADATA	WSAData;
	memset(&WSAData, 0, sizeof(WSAData));
	if (WSAStartup(MAKEWORD(2, 2), &WSAData))
	{
		assert(0);
	}
}

HttpClient::~HttpClient()
{
	DisconnectFromServer(m_SessionSocket);

	// Shutdown WinSock
	WSACleanup();
}

int HttpClient::StartSession(LPCSTR URL)
{
	EndSession();

	int Ret = ParseURL(URL, m_HostName, m_ServerPort, m_PageName);
	if(Ret != HTTP_NO_ERROR)
		return Ret;

	Ret = ConnectToServer(m_HostName.c_str(), m_ServerPort, m_ClientAddress.c_str(), m_ClientPort, m_SessionSocket);
	if(Ret != HTTP_NO_ERROR)
		return Ret;
	
	return HTTP_NO_ERROR;
}

int HttpClient::BuildMessageHeader(LPCSTR MessageType, std::string &Message)
{
	int Ret = BuildMessageHeader(MessageType, m_HostName.c_str(), m_ServerPort, m_PageName.c_str(), Message);
	return Ret;
}

int HttpClient::SendRequest(LPCSTR Message)
{
	m_fResponseHeaderReceived = false;
	int Ret = SendRequest(m_SessionSocket, Message);
	return Ret;
}

int HttpClient::ReadResponse(unsigned char *Buf, int zBuf, DWORD ResponseTimeoutSec, int &BytesRead)
{
	int Ret = ReadResponse(m_SessionSocket, Buf, zBuf, !m_fResponseHeaderReceived, ResponseTimeoutSec, BytesRead);
	m_fResponseHeaderReceived = true;
	return Ret;
}

int HttpClient::ReadData(LPBYTE &ResponseData, int &ResponseDataLen, DWORD ResponseTimeoutSec)
{
	return ReadData(m_SessionSocket, ResponseData, ResponseDataLen, ResponseTimeoutSec);
}

int HttpClient::DownloadData(LPCTSTR FileName, DWORD ResponseTimeoutSec, int DownloadReportChunk, IProgressInfoReceiver *piReceiver)
{
	return DownloadData(m_SessionSocket, FileName, ResponseTimeoutSec, DownloadReportChunk, piReceiver);
}

void HttpClient::EndSession()
{
	DisconnectFromServer(m_SessionSocket);
	m_HostName = "";
	m_PageName = "";
	m_ServerPort = -1;
	m_fResponseHeaderReceived = false;
}

int HttpClient::ConnectToServer(
						LPCSTR HostName,
						int ServerPort,
						LPCSTR ClientAddress,
						int ClientPort,
						SOCKET &Socket
						)
{
	Socket = INVALID_SOCKET;

	// Validate parameters
	if (HostName == NULL || strlen(HostName) == 0)
		return HTTP_INVALID_URL;

	// Resolve host name and build the server address struct
	struct sockaddr_in	ServerAddr;
	memset(&ServerAddr, 0, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;

	// Get IP Address from host name
	if (isalpha(HostName[0]))
	{
		// HostName is a name
		struct hostent *h = gethostbyname(HostName);
		if (h == NULL)
			return HTTP_UNKNOWN_HOST;
		ServerAddr.sin_addr.s_addr = *(unsigned long*)h->h_addr_list[0];
	}
	else
	{
		// HostName is dotted IP (x.y.z.w)
		ServerAddr.sin_addr.s_addr = inet_addr(HostName);
	}
	ServerAddr.sin_port = htons((USHORT)ServerPort);

	// Create the socket
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		return HTTP_OUT_OF_RESOURCES;

	// Bind socket if configured to
	if ((ClientAddress && strlen(ClientAddress) > 0) || ClientPort != -1)
	{
		struct sockaddr_in	BindAddr;
		memset(&BindAddr, 0, sizeof(BindAddr));
		BindAddr.sin_family = AF_INET;

		if (ClientPort != -1)
		{
			// Specified port
			BindAddr.sin_port = htons((USHORT)ClientPort);
		}
		else
		{
			// Any port
			BindAddr.sin_port = 0;
		}

		if (ClientAddress && strlen(ClientAddress) > 0)
		{
			// Specified address
			BindAddr.sin_addr.s_addr = inet_addr(ClientAddress);
		}
		else
		{
			// Any address
			BindAddr.sin_addr.s_addr = INADDR_ANY;
		}

		// Bind the socket to the specified address
		if (bind(s, (const sockaddr *)&BindAddr, sizeof(BindAddr)) == SOCKET_ERROR)
		{
			closesocket(s);
			return HTTP_PORT_BUSY;
		}
	}

	// Now, socket is bound, or it doesn't need to be bound.
	// Connect to server
	if (connect(s, (const sockaddr *)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
	{
		closesocket(s);
		return HTTP_CANNOT_CONNECT;
	}

	Socket = s;
	return HTTP_NO_ERROR;
}

int HttpClient::BuildMessageHeader(
					LPCSTR MessageType, // GET, POST
					LPCSTR HostName, 
					int Port, 
					LPCSTR PageName,
					std::string &Message
					)
{
	// Validate parameters
	if (!MessageType || !HostName || !PageName)
		return HTTP_INVALID_ARGS;

	// The message is:
	//
	// <MessageType> <PageName> HTTP/1.1				[14 chars + page]
	// Host: <HostName>:<Port>							[8 chars + HostName + Port]
	//													[1 for new line]

	char PortStr[20];
	sprintf_s(PortStr, sizeof(PortStr), "%d", Port);

	Message = MessageType;
	Message += " ";
	Message += PageName;
	Message += " HTTP/1.1\nHost: ";
	Message += HostName;
	Message += ":";
	Message += PortStr;
	Message += "\n";

	return HTTP_NO_ERROR;
}

int HttpClient::SendRequest(SOCKET &s, LPCSTR Message)
{
	if (Message == NULL || strlen(Message) == 0)
		return HTTP_INVALID_MESSAGE;

	if (s == INVALID_SOCKET)
		return HTTP_INTERNAL_ERROR;

	int BytesToSend = (int)strlen(Message);
	int BytesSent = send(s, Message, BytesToSend, 0);
	if (BytesSent != BytesToSend)
		return HTTP_CANNOT_SEND;

	return HTTP_NO_ERROR;
}

int HttpClient::ReadResponse(SOCKET &s, LPBYTE Buf, int zBuf, bool fResponseHeader, DWORD ResponseTimeoutSec, int &BytesRead)
{
	BytesRead = 0;
	if (!Buf || zBuf == 0 || s == INVALID_SOCKET)
	{
		Log(_T("HttpClient::ReadResponse - target buffer is NULL or Socket is closed\n"));
		return HTTP_INTERNAL_ERROR;
	}

	// Call select to get indication of a response, otherwise
	// recv() may wait too long (minutes)
	struct	fd_set		ReadFds;
	memset(&ReadFds, 0, sizeof(ReadFds));
	ReadFds.fd_count = 1;
	ReadFds.fd_array[0] = s;

	struct	fd_set		ExceptFds;
	memset(&ExceptFds, 0, sizeof(ExceptFds));
	ExceptFds.fd_count = 1;
	ExceptFds.fd_array[0] = s;

	struct	timeval		Timeout;
	Timeout.tv_sec = ResponseTimeoutSec;
	Timeout.tv_usec = 0;

	int SelectRet = select(1, &ReadFds, NULL, &ExceptFds, &Timeout);
	if (SelectRet == SOCKET_ERROR)
	{
		Log(_T("HttpClient::ReadResponse - error in calling select. WSAGetLastError=%d\n"), WSAGetLastError());
		return HTTP_INTERNAL_ERROR;
	}

	if (SelectRet == 0)
	{
		Log(_T("HttpClient::ReadResponse - timeout waiting for data\n"));
		return HTTP_NO_RESPONSE;
	}

	if (SelectRet > 0 && ReadFds.fd_count == 0)
	{
		Log(_T("HttpClient::ReadResponse - internal error: SelectRet=%d, fd_count=%d\n"), SelectRet, ReadFds.fd_count);
		return HTTP_INTERNAL_ERROR;
	}

	// Now we can do a recv() knowing that it will return immediately
	int RecvRet = recv(s, (char *)Buf, zBuf, 0);

	if (RecvRet == SOCKET_ERROR || RecvRet == 0)
	{
		Log(_T("HttpClient::ReadResponse - recv call failed. Ret=%d\n"), RecvRet);
		if(RecvRet == SOCKET_ERROR)
		{
			Log(_T("HttpClient::ReadResponse - last error=%d\n"), WSAGetLastError());
		}
		return HTTP_NO_RESPONSE;
	}

	if(fResponseHeader)
	{
		if (RecvRet < (int)strlen(HTTP_RESPONSE_200))
		{
			Log(_T("HttpClient::ReadResponse - received data is too short\n"));
			return HTTP_SERVER_ERROR;
		}

		if (_strnicmp(HTTP_RESPONSE_200, (LPCSTR)Buf, 15) != 0)
		{
			if (RecvRet >= (int)strlen(HTTP_RESPONSE_404) && (_strnicmp(HTTP_RESPONSE_404, (LPCSTR)Buf, strlen(HTTP_RESPONSE_404)) == 0))
			{
				Log(_T("HttpClient::ReadResponse - received server error 404\n"));
				return HTTP_FILE_NOT_FOUND;
			}
			else if (RecvRet >= (int)strlen(HTTP_RESPONSE_503) && (_strnicmp(HTTP_RESPONSE_503, (LPCSTR)Buf, strlen(HTTP_RESPONSE_503)) == 0))
			{
				// Special case of the 503 return code
				Log(_T("HttpClient::ReadResponse - received server error 503\n"));
				return HTTP_SERVER_BUSY;
			}
			else
			{
				LogA("HttpClient::ReadResponse - received server error - %s\n", (LPCSTR)Buf);
				return HTTP_SERVER_ERROR;
			}
		}
	}

	BytesRead = RecvRet;
	return HTTP_NO_ERROR;
}

int HttpClient::ReadData(SOCKET &s, LPBYTE &ResponseData, int &ResponseDataLen, DWORD ResponseTimeoutSec)
{
	ResponseData = NULL;
	ResponseDataLen = 0;

	int BytesRead = 0;
	BYTE ReadBuffer[1024];
	int DataOffset = 0;
	int Ret = ReadFirstDataResponse(s, ReadBuffer, sizeof(ReadBuffer), ResponseTimeoutSec, BytesRead, DataOffset, ResponseDataLen);
	if(Ret != HTTP_NO_ERROR)
	{
		Log(_T("HttpClient::ReadData - failed reading first data response. Ret=%d\n"), Ret);
		return Ret;
	}
	if(ResponseDataLen > MAX_RESPONSE_DATA_LENGTH)
	{
		Log(_T("HttpClient::ReadData - data length (%d) exceeded the limit\n"), ResponseDataLen);
		return HTTP_INVALID_RESPONSE;
	}

	// allocate buffer with the required size and start reading
	ResponseData = new BYTE[ResponseDataLen + 1];
	if(ResponseData == NULL)
		return HTTP_OUT_OF_RESOURCES;
	memset(ResponseData, 0, ResponseDataLen + 1);

	int TotalBytesRead = BytesRead - DataOffset;
	memcpy(ResponseData, ReadBuffer + DataOffset, TotalBytesRead);
	
	while(TotalBytesRead < ResponseDataLen)
	{
		BytesRead = 0;
		int Ret = ReadResponse(s, ResponseData + TotalBytesRead, ResponseDataLen - TotalBytesRead, false, ResponseTimeoutSec, BytesRead);
		if(Ret != HTTP_NO_ERROR)
		{
			delete[] ResponseData;
			ResponseData = NULL;
			return Ret;
		}
		TotalBytesRead += BytesRead;
	}
	return HTTP_NO_ERROR;
}

int HttpClient::DownloadData(SOCKET &s, LPCTSTR FileName, DWORD ResponseTimeoutSec, int DownloadReportChunk, IProgressInfoReceiver *piReceiver)
{
	int BytesRead = 0;
	BYTE ReadBuffer[1024];
	int DataOffset = 0;
	int ResponseDataLen = 0;
	int Ret = ReadFirstDataResponse(s, ReadBuffer, sizeof(ReadBuffer), ResponseTimeoutSec, BytesRead, DataOffset, ResponseDataLen);
	if(Ret != HTTP_NO_ERROR)
	{
		Log(_T("HttpClient::DownloadData - failed reading first response\n"));
		return Ret;
	}

	// allocate buffer with the required size and start reading
	FILE *pWriteFile = NULL;
	_tfopen_s(&pWriteFile, FileName, _T("wb"));
	if(pWriteFile == NULL)
	{
		Log(_T("HttpClient::DownloadData - failed opening file %s\n"), FileName);
		return HTTP_FILE_WRITE_ERROR;
	}

	int TotalBytesWritten = BytesRead - DataOffset;
	if(fwrite(ReadBuffer + DataOffset, 1, TotalBytesWritten, pWriteFile) != (size_t)TotalBytesWritten)
	{
		fclose(pWriteFile);
		::DeleteFile(FileName);
		Log(_T("HttpClient::DownloadData - failed writing first %d bytes to file\n"), TotalBytesWritten);
		return HTTP_FILE_WRITE_ERROR;
	}

	int LastReport = 0;
	if(piReceiver)
	{
		// Always call on first chunk, now start minding report chunk size
		piReceiver->ProgressReport(TotalBytesWritten, ResponseDataLen);
		LastReport = TotalBytesWritten;
	}

	Log(_T("HttpClient::DownloadData - start fetching %d bytes\n"), ResponseDataLen);

	while(TotalBytesWritten < ResponseDataLen)
	{
		BytesRead = 0;
		int Ret = ReadResponse(s, ReadBuffer, sizeof(ReadBuffer), false, ResponseTimeoutSec, BytesRead);
		if(Ret != HTTP_NO_ERROR)
		{
			fclose(pWriteFile);
			::DeleteFile(FileName);
			Log(_T("HttpClient::DownloadData - failed reading response. %d bytes successfully written\n"), TotalBytesWritten);
			return Ret;
		}
		if(fwrite(ReadBuffer, 1, BytesRead, pWriteFile) != (size_t)BytesRead)
		{
			fclose(pWriteFile);
			::DeleteFile(FileName);
			Log(_T("HttpClient::DownloadData - failed writing %d bytes to file. %d bytes successfully written\n"), BytesRead, TotalBytesWritten);
			return HTTP_FILE_WRITE_ERROR;
		}

		TotalBytesWritten += BytesRead;
		if (piReceiver && ((TotalBytesWritten - LastReport) >= DownloadReportChunk))
		{
			piReceiver->ProgressReport(TotalBytesWritten, ResponseDataLen);
			LastReport = TotalBytesWritten;
		}
	}
	fclose(pWriteFile);

	Log(_T("HttpClient::DownloadData - Total %d bytes fetched\n"), TotalBytesWritten);

	return HTTP_NO_ERROR;
}

int HttpClient::ReadFirstDataResponse(SOCKET &s, LPBYTE ReadBuffer, int ReadBufferSize, DWORD ResponseTimeoutSec, int &BytesRead, int &DataOffset, int &ResponseDataLen)
{
	BytesRead = 0;
	DataOffset = -1;
	ResponseDataLen = 0;
	memset(ReadBuffer, 0, ReadBufferSize);

	int Ret = ReadResponse(s, ReadBuffer, ReadBufferSize, true, ResponseTimeoutSec, BytesRead);
	if(Ret != HTTP_NO_ERROR)
		return Ret;

	// parse the content length
	LPCSTR PacketString = (LPCSTR)ReadBuffer;
	LPCSTR EndOfString = (LPCSTR)(ReadBuffer + BytesRead);

	while(PacketString < EndOfString)
	{
		PacketString = strchr(PacketString, '\n');
		if(PacketString == NULL)
			break;
		PacketString++;

		if(_strnicmp(PacketString, "Content-Length", strlen("Content-Length")) == 0)
		{
			// found the line containing the length. Now look for the value
			PacketString = strchr(PacketString, ':');
			if(PacketString == NULL)
				break;
			PacketString++;

			// eat spaces
			while(PacketString < EndOfString && isspace(PacketString[0]))
			{
				PacketString++;
			}
			if(PacketString == EndOfString)
				break;

			LPCSTR EndOfLine = strchr(PacketString, '\n');
			if(EndOfLine == NULL)
				break;
			int NumberStringSize = (int)(EndOfLine - PacketString);

			char NumberString[32];
			if(NumberStringSize >= sizeof(NumberString))
				break; // number string is too large
			strncpy_s(NumberString, sizeof(NumberString), PacketString, NumberStringSize);
			NumberString[NumberStringSize] = '\0';
			sscanf_s(NumberString, "%d", &ResponseDataLen);
			break;
		}
	}
	if(ResponseDataLen <= 0)
		return HTTP_INVALID_RESPONSE;

	// Found Content-Length - now go to beginning of body - after one empty line
	LPCSTR StartOfData = strstr(PacketString, "\n\n");
	if(StartOfData != NULL)
	{
		StartOfData += strlen("\n\n");
	}
	else
	{
		StartOfData = strstr(PacketString, "\n\r\n\r");
		if(StartOfData != NULL)
		{
			StartOfData += strlen("\n\r\n\r");
		}
		else
		{
			StartOfData = strstr(PacketString, "\r\n\r\n");
			if(StartOfData != NULL)
			{
				StartOfData += strlen("\r\n\r\n");
			}
			else
			{
				return HTTP_INVALID_RESPONSE;
			}
		}
	}
	int TotalBytesRead = (int)(EndOfString - StartOfData);
	if(TotalBytesRead > ResponseDataLen)
		return HTTP_INVALID_RESPONSE;

	DataOffset = (int)((LPBYTE)StartOfData - ReadBuffer);
	return HTTP_NO_ERROR;
}

void HttpClient::DisconnectFromServer(SOCKET &s)
{
	if(s != INVALID_SOCKET)
	{
		closesocket(s);
		s = INVALID_SOCKET;
	}
}

int HttpClient::ParseURL(
					LPCSTR URL,
					std::string &HostName, 
					int &Port, 
					std::string &PageName
					)
{
	char	PortStr[8];
	char	*StartPort;
	char	*StartPage;

	// initialize settings
	HostName = "";
	Port = DEFAULT_SERVER_PORT;
	PageName = "";

	// verify http protocol
	if (URL == NULL)
		return HTTP_INVALID_URL;
	const size_t URLLen = strlen(URL);
	LPSTR StartHostName = (LPSTR)URL;
	if (strstr(URL, "://"))
	{
		// URL includes protocol - must be http://
		if (URLLen < strlen("http://"))
			return HTTP_INVALID_URL;

		if (_strnicmp(URL, "http://", strlen("http://")) != 0)
		{
			// Non-HTTP protocol such as ftp://
			return HTTP_INVALID_URL;
		}

		// Skip the http:// prefix
		StartHostName = StartHostName + strlen("http://");
	}

	// Now we separate the host name from the port as in localhost:80
	LPSTR Colon = strchr(StartHostName, ':');
	LPSTR Slash = strchr(StartHostName, '/');

	if (Colon && (Colon < Slash))
	{
		// Port mentioned - www.example.com:8000
		HostName.assign(StartHostName, (int)(Colon - StartHostName));

		Colon++; // skip ':'
		StartPort = Colon;
		while ((Slash && (Colon < Slash)) ||
			(!Slash && (*Colon)))
		{
			Colon++;	// continue until slash or end of string
		}

		if (Colon - StartPort == 0 || 
			((Colon - StartPort) > (sizeof(PortStr) - 1)))
		{
			// Ports are from 0 to 65535, so PortStr should suffice
			return HTTP_INVALID_URL;
		}
		strncpy_s(PortStr, sizeof(PortStr), Colon, (int)(Colon - StartPort));

		if (!is_all_digit(PortStr))
			return HTTP_INVALID_URL;

		Port = atoi(PortStr);

		// Indicate where Page information begins for next section
		StartPage = 0;
		if (*Colon && Colon == Slash)
		{
			StartPage = Slash;
		}
	}
	else
	{
		// Port not mentioned - www.example.com
		if (Slash)
		{
			// Indicate where Page information begins for next section
			StartPage = Slash;

			// Host name is until slash
			HostName.assign(StartHostName, (int)(Slash - StartHostName));
		}
		else
		{
			// Indicate that there is no Page information for next section
			StartPage = 0;

			// No slash - entire string is host name
			HostName = StartHostName;
		}
	}

	// Left to fill in page information
	if (StartPage)
	{
		PageName = StartPage;
	}
	if (PageName.length() == 0)
	{
		PageName = "/";	// Default page is root 
	}

	if(HostName.length() >= MAX_HOST_NAME_LEGNTH || PageName.length() >= MAX_PAGE_NAME_LENGTH)
		return HTTP_INVALID_URL;

	return HTTP_NO_ERROR;
}

int HttpClient::URLEncode(LPCSTR Str, std::string &EncodedStr)
{
	EncodedStr = "";

	if (!Str)
		return HTTP_INVALID_MESSAGE;

	const size_t Len = strlen(Str);

	for(index_t Idx = 0; Idx < (index_t)Len; Idx++)
	{
		char c = Str[Idx];
		if ( (48 <= c && c <= 57) ||//0-9
             (65 <= c && c <= 90) ||//abc...xyz
             (97 <= c && c <= 122) || //ABC...XYZ
             (c=='~' || c=='!' || c=='*' || c=='(' || c==')' || c=='\'')
			 )
		{
			// Allowed characters - copy
			EncodedStr += c;
		}
		else
		{
			// Not allowed - encode using '%' notation
			EncodedStr += '%';
			EncodedStr += hex_digit((c & 0xF0) >> 4);
			EncodedStr += hex_digit(c & 0x0F);
		}
	}
	return HTTP_NO_ERROR;
}

int HttpClient::URLEncode(LPCWSTR wStr, std::string &EncodedStr)
{
	EncodedStr = "";
	if (!wStr)
		return HTTP_INVALID_MESSAGE;

	std::string Str;
	SetString(Str, wStr);

	return URLEncode(Str.c_str(), EncodedStr);
}

char HttpClient::hex_digit(int value)
{
	char ret;

	ret = '?';

	if (value >= 0 && value <= 9)
	{
		ret = (char)('0' + value);
	}
	else if (value >= 10 && value <= 15)
	{
		ret = (char)('A' + value - 10);
	}

	return ret;
}

int HttpClient::num_digits(int value)
{
	if (value == 0)
		return 1;

	assert(value > 0);

	int int_log_val = (int)log10((double)value);
	return int_log_val + 1;
}

bool HttpClient::is_all_digit(LPCSTR Str)
{
	if(!Str)
		return false;
	const size_t Len = strlen(Str);
	if(Len == 0)
		return false;

	for(index_t i = 0; i < (index_t)Len; i++)
	{
		if(!isdigit(Str[i]))
			return false;
	}
	return true;
}
