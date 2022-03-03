#include "Pch.h"
#include "StringTable.h"

extern HMODULE g_hModule;

StringTable::StringTable()
{
}

StringTable::~StringTable()
{
}

LPCTSTR StringTable::GetString(int StringId)
{
	LockScope Lock(m_UsageLock);
	
	StringMap::iterator Iterator = m_StringMap.find(StringId);
	if(Iterator != m_StringMap.end())
	{
		return (*Iterator).second.c_str();
	}
	TCHAR String[256];
	if(::LoadString(g_hModule, StringId, String, sizeof(String) / sizeof(String[0])) == 0)
		return _T("");
	m_StringMap.insert(StringMap::value_type(StringId, String));

	Iterator = m_StringMap.find(StringId);
	return (*Iterator).second.c_str();
}

StringTable g_StringTable;

LPCTSTR GetResourceString(int StringId)
{
	return g_StringTable.GetString(StringId);
}
