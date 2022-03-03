#include "Pch.h"
#include "LanguageStringTable.h"
#include "ApplicationDataPath.h"
#include "resource.h"
#include "StringConvert.h"

extern HMODULE g_hModule;

LanguageStringTable::LanguageStringTable()
{
}

LanguageStringTable::~LanguageStringTable()
{
}

Result LanguageStringTable::Open()
{
	m_LanguageHKLToStringMap.erase(m_LanguageHKLToStringMap.begin(), m_LanguageHKLToStringMap.end());
	m_LanguageHKLToFullStringMap.erase(m_LanguageHKLToFullStringMap.begin(), m_LanguageHKLToFullStringMap.end());
	m_LanguageStringToHKLMap.erase(m_LanguageStringToHKLMap.begin(), m_LanguageStringToHKLMap.end());

	// load file resource
	HRSRC hrsRC = ::FindResource (g_hModule, MAKEINTRESOURCE(IDB_FILE_LANGUAGE_TABLE), _T("BIN"));
	if(!hrsRC)
		return Result_ResourceNotFound;
	DWORD dwResourceSize = SizeofResource (g_hModule, hrsRC);
	HGLOBAL hglbRC = LoadResource (g_hModule, hrsRC);
	if(!hglbRC)
		return Result_ResourceAllocFailed;
	LPVOID lpRsc = LockResource(hglbRC);
	if(!lpRsc)
		return Result_ResourceAllocFailed;

	// create string from resource
	_tstring TableString;
	SetString(TableString, (LPCSTR)lpRsc, dwResourceSize);

	// parse the string
	LPTSTR CurrentLine = &TableString[0];
	while(CurrentLine)
	{
		LPTSTR NextLine = _tcschr(CurrentLine, _T('\n'));
		if(NextLine)
		{
			NextLine[0] = _T('\0');
		}
		int intVal;
		TCHAR LangStr[256];
		TCHAR LangFullStr[256];
		if(_stscanf_s(CurrentLine, _T("%s %x %s"), LangStr, sizeof(LangStr) / sizeof(LangStr[0]), &intVal, LangFullStr, sizeof(LangFullStr) / sizeof(LangFullStr[0])) == 3)
		{
			m_LanguageHKLToStringMap.insert(LanguageHKLToStringMap::value_type(intVal, LangStr));
			m_LanguageHKLToFullStringMap.insert(LanguageHKLToStringMap::value_type(intVal, LangFullStr));
			m_LanguageStringToHKLMap.insert(LanguageStringToHKLMap::value_type(LangStr, intVal));
		}
		CurrentLine = NextLine;
		if(CurrentLine)
		{
			CurrentLine++;
		}
	}
	return Result_Success;
}

bool LanguageStringTable::GetLanguageString(HKL hKL, _tstring &LanguageIdString, _tstring &LanguageSpecificString, int LanguageNameFormat /*= LANGUAGE_FORMAT_NORMAL*/) const
{
	bool fSuccess = true;

	if(LanguageNameFormat == LANGUAGE_FORMAT_FULL_NAME)
	{
		fSuccess &= FillLongLanguageString(LOWORD(hKL), LanguageIdString);
		fSuccess &= FillLongLanguageString(HIWORD(hKL), LanguageSpecificString);
	}
	else
	{
		fSuccess &= FillLanguageString(LOWORD(hKL), LanguageIdString, LanguageNameFormat == LANGUAGE_FORMAT_SHORT_NAME);
		fSuccess &= FillLanguageString(HIWORD(hKL), LanguageSpecificString, LanguageNameFormat == LANGUAGE_FORMAT_SHORT_NAME);
	}
	return fSuccess;
}

bool LanguageStringTable::GetLanguageHandle(LPCTSTR LanguageIdString, LPCTSTR LanguageSpecificString, HKL &rhKL) const
{
	rhKL = NULL;

	int LoValue;
	if(!GetValueFromLanguageString(LanguageIdString, LoValue))
		return false;
	int HiValue;
	if(!GetValueFromLanguageString(LanguageSpecificString, HiValue))
		return false;
	rhKL = (HKL)(MAKELONG(LoValue, HiValue));
	return true;
}

bool LanguageStringTable::FillLongLanguageString(int Value, _tstring &LanguageString) const
{
	LanguageHKLToStringMap::const_iterator Iterator = m_LanguageHKLToFullStringMap.find(Value);
	if(Iterator == m_LanguageHKLToFullStringMap.end())
	{
		TCHAR DefaultString[256];

		_stprintf_s(DefaultString, sizeof(DefaultString) / sizeof(DefaultString[0]), _T("%04X"), Value);
		LanguageString = DefaultString;
		return false;
	}
	else
	{
		LanguageString = (*Iterator).second;
	}
	return true;
}

bool LanguageStringTable::FillLanguageString(int Value, _tstring &LanguageString, bool fShort) const
{
	LanguageHKLToStringMap::const_iterator Iterator = m_LanguageHKLToStringMap.find(Value);
	if(Iterator == m_LanguageHKLToStringMap.end())
	{
		TCHAR DefaultString[256];

		_stprintf_s(DefaultString, sizeof(DefaultString) / sizeof(DefaultString[0]), _T("%04X"), Value);
		LanguageString = DefaultString;
		return false;
	}
	else
	{
		LanguageString = (*Iterator).second;

		if(fShort)
		{
			size_t Offset = LanguageString.find(_T('-'));
			if(Offset != _tstring::npos)
			{
				LanguageString = LanguageString.substr(0, Offset);
			}
		}
	}
	return true;
}

bool LanguageStringTable::GetValueFromLanguageString(LPCTSTR LanguageString, int &rnValue) const
{
	rnValue = 0;

	LanguageStringToHKLMap::const_iterator Iterator = m_LanguageStringToHKLMap.find(LanguageString);
	if(Iterator == m_LanguageStringToHKLMap.end())
	{
		if(_tcslen(LanguageString) != 4)
			return false;
		if(_stscanf_s(LanguageString, _T("%X"), &rnValue) == 0)
			return false;
	}
	else
	{
		rnValue = (*Iterator).second;
	}
	return true;
}
