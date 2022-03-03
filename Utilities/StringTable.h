#ifndef _STRING_TABLE_HEADER_
#define _STRING_TABLE_HEADER_

#include "SyncHelpers.h"

LPCTSTR GetResourceString(int StringId);

class StringTable
{
public:
	StringTable();
	virtual ~StringTable();

	LPCTSTR GetString(int StringId);
private:
	CriticalSec m_UsageLock;

	typedef std::map<int, _tstring> StringMap;
	StringMap m_StringMap;
};

#endif // _STRING_TABLE_HEADER_