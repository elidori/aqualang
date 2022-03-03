#ifndef _I_NOTIFICATION_AREA_REQUESTS_LISTENER_HEADER_
#define _I_NOTIFICATION_AREA_REQUESTS_LISTENER_HEADER_

class INotificationAreaRequestsListener
{
protected:
	virtual ~INotificationAreaRequestsListener() {}
public:
	virtual bool PresentTextRequest(const char *Title, const char *Text) = 0;
	virtual bool PresentTextRequest(const wchar_t *Title, const wchar_t *Text) = 0;
};

#endif // _I_NOTIFICATION_AREA_REQUESTS_LISTENER_HEADER_