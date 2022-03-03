#ifndef _XML_VALUE_TYPES_HEADER_
#define _XML_VALUE_TYPES_HEADER_

#include "xmlStructure.h"

class LanguageStringTable;

class xmlLanguageValue : public xmlInt
{
public:
	xmlLanguageValue(int NameId);

	static void SetLanguageTable(const LanguageStringTable &rLanguageStringTable);
protected:
	virtual bool ConvertFromString(__int64 &IntVal, LPCTSTR Value) const;
	virtual bool ConvertToString(__int64 IntVal, _tstring &Value) const;
	virtual bool CheckValueValidity(__int64 Value) const;
private:
	static const LanguageStringTable *m_g_pLanguageStringTable;
};

class WindowItemPropertyTypes;

class xmlPropertyTypeValue : public xmlString
{
public:
	xmlPropertyTypeValue(int NameId, const WindowItemPropertyTypes &rWindowItemPropertyTypes);

	static bool PeekType(ReadableBinStorage &rBinStorage, int &rType);
protected:
	virtual bool Serialize(WritableBinStorage &rBinStorage);
	virtual bool Deserialize(ReadableBinStorage &rBinStorage);
private:
	const WindowItemPropertyTypes &m_rWindowItemPropertyTypes;
};


#endif // _XML_VALUE_TYPES_HEADER_