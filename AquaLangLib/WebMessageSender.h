#ifndef _WEB_MESSAGE_SENDER_HEADER_
#define _WEB_MESSAGE_SENDER_HEADER_

// Construct WebMessageSender. 
// InstanceID identifies this WebMessageSender in messages 
// written on server side.
// Provide a client address and/or port for client to bind to, 
// or leave at default values of 0 and -1 to allow OS to 
// select an available address + port
// -------------------------------------------------------------
//
// SendHTTPMessage:
// Send a message to the server
//
// This method blocks until the message is successfully sent and
// a response is received from the server, or an error occurs.
//
// ServerURL: In Browser-like format, i.e. 
// http://address:port/rest-of-url. address is either an IP
// address or a host name to be resolved by Web Message Sender
// package. Port is optional and defaults to 80.
//
// Message: ASCII text message of up to 1024 characters, 0- 
// terminated
//
// Returns 0 (=SEND_SUCCESS) on success or error otherwise
// (see error codes above)
// ---------------------------------------------------------
//
// ParseURL:
// Parse a URL into the host name, port and page. If protocol
// is mentioned it must be HTTP
//
// Returns 0 for success
// ----------------------------------------------------------
//
// BuildPOSTMessage:
// Build an HTTP POST message using the provided POST
// and Message
//
// The POST message will hold the headers Host, Content-Type 
// and Content-Length.
// The body has three parameters including the validation key,
// the instance ID and the message itself.
// 
// The built message is placed inside Buf
//
// Returns 0 on success
// ----------------------------------------------------------
//
// URLEncode
// Encode Str in URL-encoding (i.e. space becomes %20 etc.)
//
// Returns 0 on success
// ---------------------------------------------------------------

#include "HttpClient.h"

// The validation key that appears in all messages.
#define WEB_MESSAGE_SENDER_VALIDATION_KEY	809201838
#define WEB_MESSAGE_SENDER_VALIDATION_KEY_STRING "809201838"

class WebMessageSender : public HttpClient
{
public:
	WebMessageSender(int InstanceID, const char* ClientAddress = 0, int ClientPort = -1);
	virtual ~WebMessageSender();

	int SendHTTPMessage(const char *ServerURL, LPCTSTR Message);
private:
	int BuildPOSTMessage(const char *Message, std::string &PostMsg);
};

#endif // _WEB_MESSAGE_SENDER_HEADER_