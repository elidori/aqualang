#ifndef _AUTO_UPDATE_CONFIG_HEADER_
#define _AUTO_UPDATE_CONFIG_HEADER_

#include "xmlStructure.h"

class AutoUpdateConfig
{
public:
	AutoUpdateConfig();
	virtual ~AutoUpdateConfig();

	void Open(LPCTSTR UserName);

	bool Load();
	void Store();

	xmlTime m_NextTimeToCheck;
private:
	xmlTree m_xmlTree;
	xmlStructure m_AutoUpdateStruct;
};

#endif // _AUTO_UPDATE_CONFIG_HEADER_