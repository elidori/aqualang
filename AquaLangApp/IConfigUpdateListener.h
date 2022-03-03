#ifndef _I_CONFIG_UPDATE_LISTENER_HEADER_
#define _I_CONFIG_UPDATE_LISTENER_HEADER_

class IConfigUpdateListener
{
protected:
	virtual ~IConfigUpdateListener() {}
public:
	virtual void ConfigurationChanged() = 0;
};

#endif // _I_CONFIG_UPDATE_LISTENER_HEADER_