#ifndef _LANGUAGE_STRING_TABLE_HEADER_
#define _LANGUAGE_STRING_TABLE_HEADER_

#include "WinHookResult.h"

using namespace WinHook;

enum LanguageStringFormat
{
	LANGUAGE_FORMAT_NORMAL,
	LANGUAGE_FORMAT_FULL_NAME,
	LANGUAGE_FORMAT_SHORT_NAME
};

class LanguageStringTable
{
public:
	LanguageStringTable();
	virtual ~LanguageStringTable();
	Result Open();

	bool GetLanguageString(HKL hKL, _tstring &LanguageIdString, _tstring &LanguageSpecificString, int LanguageNameFormat = LANGUAGE_FORMAT_NORMAL) const;
	bool GetLanguageHandle(LPCTSTR LanguageIdString, LPCTSTR LanguageSpecificString, HKL &rhKL) const;
private:
	bool FillLongLanguageString(int Value, _tstring &LanguageString) const;
	bool FillLanguageString(int Value, _tstring &LanguageString, bool fShort) const;
	bool GetValueFromLanguageString(LPCTSTR LanguageString, int &rnValue) const;
private:
	typedef std::map<int, _tstring> LanguageHKLToStringMap;
	LanguageHKLToStringMap m_LanguageHKLToStringMap;

	LanguageHKLToStringMap m_LanguageHKLToFullStringMap;

	typedef std::map<_tstring, int> LanguageStringToHKLMap;
	LanguageStringToHKLMap m_LanguageStringToHKLMap;
};

#endif // _LANGUAGE_STRING_TABLE_HEADER_