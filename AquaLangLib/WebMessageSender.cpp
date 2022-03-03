#include "Pch.h"
#include "WebMessageSender.h"
#include "HttpClient.h"
#include "StringConvert.h"

#define MAX_MESSAGE_LENGTH		1024	
#define	WAIT_RESPONSE_TIMEOUT	30		// Wait up to 30 seconds for response

 /* ----------------------------------------------------------- *\
| Construct WebMessageSender.									  |
| InstanceID identifies this WebMessageSender in messages		  |
| written on server side.										  |
| Provide a client address and/or port for client to bind to,	  |
| or leave at default values of 0 and -1 to allow OS to			  |
| select an available address + port							  |
 \* ----------------------------------------------------------- */
WebMessageSender::WebMessageSender(int InstanceID, const char *ClientAddress /*= NULL*/, int ClientPort /*= -1*/)
	: HttpClient(InstanceID, ClientAddress, ClientPort)
{
}

WebMessageSender::~WebMessageSender()
{
}

 /* ------------------------------------------------------------ *\
| Send a message to the server									   |
|																   |
| This method blocks until the message is successfully sent and	   |
| a response is received from the server, or an error occurs.	   |
|																   |
| ServerURL: In Browser-like format, i.e.						   |
| http://address:port/rest-of-url. address is either an IP		   |
| address or a host name to be resolved by Web Message Sener	   |
| package. Port is optional and defaults to 80.					   |
|																   |
| Message: ASCII text message of up to 1024 characters, 0-		   |
| terminated													   |
|																   |
| Returns 0 (=SEND_SUCCESS) on success or error otherwise (see	   |
| error codes in header file)		   |
 \* ------------------------------------------------------------ */
int WebMessageSender::SendHTTPMessage(const char *ServerURL, LPCTSTR Message)
{
#if false
	std::string MessageString;
	SetString(MessageString, Message);
	// Validate parameters
	if (MessageString.length() == 0 || MessageString.length() > MAX_MESSAGE_LENGTH)
		return HTTP_INVALID_MESSAGE;

	int Ret = StartSession(ServerURL);
	if(Ret != HTTP_NO_ERROR)
		return Ret;

	// Connected to server - build HTTP message and send
	std::string PostMsg;
	Ret = BuildPOSTMessage(MessageString.c_str(), PostMsg);
	if(Ret != HTTP_NO_ERROR)
	{
		EndSession();
		return Ret;
	}

	Ret = SendRequest(PostMsg.c_str());
	if(Ret != HTTP_NO_ERROR)
	{
		EndSession();
		return Ret;
	}

	char ReadBuf[256];
	int RecvRet;
	Ret = ReadResponse((unsigned char *)ReadBuf, sizeof(ReadBuf), WAIT_RESPONSE_TIMEOUT, RecvRet);
	EndSession();
	if(Ret != HTTP_NO_ERROR)
		return Ret;
#endif
	return HTTP_NO_ERROR;
}

 /* --------------------------------------------------------- *\
| Build an HTTP POST message using the provided POST			|
| and Message													|
|																|
| The POST message will hold the headers Host, Content-Type and	|	
| Content-Length.												|
| The body has three parameters including the validation key,	|
| the instance ID and the message itself.						|
|																|
| The built message is placed inside Buf						|
|																|
| Returns 0 on success											|
 \* ---------------------------------------------------------- */
int WebMessageSender::BuildPOSTMessage(const char *Message,	std::string &PostMsg)
{
	// Validate parameters
	if (!Message)
		return HTTP_INVALID_ARGS;

	// Encode the message
	std::string URLEncodedMessage;
	int Ret = HttpClient::URLEncode(Message, URLEncodedMessage);
	if(Ret != HTTP_NO_ERROR)
		return Ret;
	if(URLEncodedMessage.length() > MAX_MESSAGE_LENGTH * 3)
		return HTTP_INVALID_MESSAGE;

	// The message is:
	//
	// POST <page> HTTP/1.1								[15 chars + page]
	// Host: <HostName>:<Port>							[8 chars + HostName + Port]
	// Content-Type: application/x-www-form-urlencoded	[48 chars]
	// Content-Length: <body-length>					[up to 27 chars]
	//													[1 char]
	// validation_key=8092018385&instance_id=<instance_id>&text=<encoded message>
	// [up to 57 chars + encoded message length]
	// 
	char InstanceIdStr[20];
	sprintf_s(InstanceIdStr, sizeof(InstanceIdStr), "%d", GetInstanceId());

	std::string ContentString;
	ContentString += "validation_key=";
	ContentString += WEB_MESSAGE_SENDER_VALIDATION_KEY_STRING;
	ContentString += "&instance_id=";
	ContentString += InstanceIdStr;
	ContentString += "&text=";
	ContentString += URLEncodedMessage;

	char ContentLengthStr[20];
	sprintf_s(ContentLengthStr, sizeof(ContentLengthStr), "%d", ContentString.length());

	Ret = BuildMessageHeader("POST", PostMsg);
	if(Ret != HTTP_NO_ERROR)
		return Ret;

	PostMsg += "Content-Type: application/x-www-form-urlencoded\nContent-Length: ";
	PostMsg += ContentLengthStr;
	PostMsg += "\n\n";
	PostMsg += ContentString;

	return HTTP_NO_ERROR;
}
