#include "Pch.h"
#include "xmlValueTypes.h"
#include "LanguageStringTable.h"
#include "WindowItemPropertyTypes.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlLanguageValue
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const LanguageStringTable *xmlLanguageValue::m_g_pLanguageStringTable = NULL;

xmlLanguageValue::xmlLanguageValue(int NameId)
	: xmlInt(NameId)
{
}

void xmlLanguageValue::SetLanguageTable(const LanguageStringTable &rLanguageStringTable)
{
	m_g_pLanguageStringTable = &rLanguageStringTable;
}

bool xmlLanguageValue::ConvertFromString(__int64 &IntVal, LPCTSTR Value) const
{
	assert(m_g_pLanguageStringTable != NULL);

	_tstring LanguageIdString = Value;
	_tstring LanguageSpecificString = Value;
	size_t Separator = LanguageIdString.find(_T(':'));
	if(Separator != _tstring::npos)
	{
		LanguageSpecificString = LanguageIdString.substr(Separator + 1, LanguageIdString.length());
		LanguageIdString = LanguageIdString.substr(0, Separator);
	}

	HKL hLanguage;
	if(!m_g_pLanguageStringTable->GetLanguageHandle(LanguageIdString.c_str(), LanguageSpecificString.c_str(), hLanguage))
		return false;
	IntVal = (int)hLanguage;
	return true;
}

bool xmlLanguageValue::ConvertToString(__int64 IntVal, _tstring &Value) const
{
	assert(m_g_pLanguageStringTable != NULL);

	if(IntVal == 0)
		return false;

	_tstring LanguageIdString;
	_tstring LanguageSpecificString;
	m_g_pLanguageStringTable->GetLanguageString((HKL)IntVal, LanguageIdString, LanguageSpecificString);
	Value = LanguageIdString + _T(":") + LanguageSpecificString;
	return true;
}

bool xmlLanguageValue::CheckValueValidity(__int64 Value) const
{
	return (Value != 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlPropertyTypeValue
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
xmlPropertyTypeValue::xmlPropertyTypeValue(int NameId, const WindowItemPropertyTypes &rWindowItemPropertyTypes)
	: xmlString(NameId),
	m_rWindowItemPropertyTypes(rWindowItemPropertyTypes)
{
}

bool xmlPropertyTypeValue::PeekType(ReadableBinStorage &rBinStorage, int &rType)
{
	if(!rBinStorage.Peek((LPBYTE)&rType, sizeof(rType)))
		return false;

	return true;
}

bool xmlPropertyTypeValue::Serialize(WritableBinStorage &rBinStorage)
{
	int Type;
	T_WindowItemPropertyCreator Creator;
	bool fIsReadOnly;
	if(!m_rWindowItemPropertyTypes.Find(GetValue(), Type, Creator, fIsReadOnly))
		return false;
	return rBinStorage.Put((LPBYTE)&Type, sizeof(Type));
}

bool xmlPropertyTypeValue::Deserialize(ReadableBinStorage &rBinStorage)
{
	int Type;
	if(!rBinStorage.Get((LPBYTE)&Type, sizeof(Type)))
		return false;

	_tstring TypeString;
	if(!m_rWindowItemPropertyTypes.Find(Type, TypeString))
		return false;

	if(!SetString(TypeString.c_str()))
		return false;

	return true;
}
