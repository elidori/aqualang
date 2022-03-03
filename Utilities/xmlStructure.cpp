#include "Pch.h"
#include "xmlStructure.h"
#include "StringConvert.h"
#include "BinStorage.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlItem
///////////////////////////////////////////////////////////////////////////////////////////////////////////
xmlItem::xmlItem(int NameId)
	: m_NameId(NameId)
{
}

xmlItem::xmlItem(const xmlItem &r)
	: m_NameId(r.m_NameId)
{
}

xmlItem::~xmlItem()
{
}

bool xmlItem::SetItem(const xmlItem &rRef)
{
	if(m_NameId != rRef.m_NameId)
		return false;
	return true;
}

int xmlItem::GetXmlNameId() const
{
	return m_NameId;
}

LPCTSTR xmlItem::GetXmlName() const
{
	return GetResourceString(m_NameId);
}

bool xmlItem::IsEmptyXmlName() const
{
	return (m_NameId == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlValue
///////////////////////////////////////////////////////////////////////////////////////////////////////////
xmlValue::xmlValue(int NameId, bool fAttribute)
	: xmlItem(NameId),
	m_fAttribute(fAttribute),
	m_fValueExists(false)
{
}

xmlValue::xmlValue(const xmlValue &r)
	: xmlItem(r),
	m_fAttribute(r.m_fAttribute),
	m_fValueExists(r.m_fValueExists)
{
}

bool xmlValue::Load(XmlNode &rxParent, index_t Index /*= 0*/)
{
	m_fValueExists = false;
	if(m_fAttribute)
	{
		XmlAttribute *pAttr = rxParent.GetAttribute(GetXmlName(), Index);
		if(pAttr && _tcslen(pAttr->GetValue()) > 0)
		{
			SetString(pAttr->GetValue());
		}
	}
	else
	{
		XmlNode *pxNode = NULL;
		if(_tcslen(GetXmlName()) == 0)
		{
			pxNode = &rxParent;
		}
		else
		{
			pxNode = rxParent.GetChild(GetXmlName(), Index);
		}
		if(pxNode == NULL || pxNode->GetTextField() == NULL || _tcslen(pxNode->GetTextField()->GetText()) == 0)
			return false;
		SetString(pxNode->GetTextField()->GetText());
	}
	return true;
}

void xmlValue::Store(XmlNode &rxParent)
{
	if(IsValid())
	{
		_tstring AllocatedValue;
		LPCTSTR pValue = GetAssignedString(AllocatedValue);

		if(m_fAttribute)
		{
			XmlAttribute *pAttr = rxParent.AddAttribute();
			if(pAttr)
			{
				pAttr->SetName(GetXmlName());
				pAttr->SetValue(pValue);
			}
		}
		else
		{
			XmlNode *pxNode = NULL;
			if(_tcslen(GetXmlName()) == 0)
			{
				pxNode = &rxParent;
			}
			else
			{
				pxNode = rxParent.AddChild(GetXmlName());
			}
			if(pxNode != NULL)
			{
				pxNode->AddText(pValue);
			}
		}
	}
}

bool xmlValue::GetStringValue(_tstring &rValue, LPCTSTR Default) const
{
	if(!IsValid())
	{
		if(Default != NULL)
		{
			rValue = Default;
		}
		return false;
	}
	GetAssignedString(rValue);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlString
///////////////////////////////////////////////////////////////////////////////////////////////////////////
xmlString::xmlString(int NameId, LPCTSTR Default /*= NULL*/, bool fAttribute /*= false*/)
	: xmlValue(NameId, fAttribute)
{
	if(Default)
	{
		m_DefaultValue = Default;
	}
}

xmlString::xmlString(const xmlString &r)
	: xmlValue(r),
	m_Value(r.m_Value),
	m_DefaultValue(r.m_DefaultValue)
{
}

xmlString::~xmlString()
{
}

bool xmlString::SetString(LPCTSTR Value)
{
	if(Value == NULL)
	{
		Clean();
	}
	else
	{
		m_Value = Value;
		Validate();
	}
	return true;
}

LPCTSTR xmlString::GetAssignedString(_tstring &rValue) const
{
	rValue = m_Value;
	return rValue.c_str();
}

bool xmlString::Serialize(WritableBinStorage &rBinStorage)
{
	return rBinStorage.PutString(m_Value.c_str(), (int)m_Value.length());
}

bool xmlString::Deserialize(ReadableBinStorage &rBinStorage)
{
	if(!rBinStorage.GetString(m_Value))
		return false;
	Validate();
	return true;
}

void xmlString::SetStringIfModified(LPCTSTR Value)
{
	assert(Value != NULL);
	if(IsValid() || m_DefaultValue != Value)
	{
		SetString(Value);
	}
}

bool xmlString::GetString(_tstring &rValue) const
{
	rValue = m_DefaultValue;
	if(!IsValid())
		return false;
	rValue = m_Value;
	return true;
}

bool xmlString::Set(const xmlString &rValue)
{
	if(!xmlItem::SetItem(rValue))
		return false;
	if(!rValue.IsValid())
	{
		xmlValue::Clean();
	}
	else
	{
		SetString(rValue.m_Value.c_str());
	}
	return true;
}

bool xmlString::IsEqualeTo(const xmlString &rValue) const
{
	if(!IsValid() || !rValue.IsValid())
		return false;
	return (m_Value == rValue.m_Value);
}

bool xmlString::IsCaseInsensitiveEqualeTo(const xmlString &rValue) const
{
	if(!IsValid() || !rValue.IsValid())
		return false;
	return (_tcsicmp(m_Value.c_str(), rValue.m_Value.c_str()) == 0);
}

bool xmlString::CheckValue(XmlNode &rxParent, int NameId, _tstring &Value, bool fAttribute /*= false*/)
{
	LPCTSTR Name = GetResourceString(NameId);

	if(fAttribute)
	{
		XmlAttribute *pAttr = rxParent.GetAttribute(Name);
		if(pAttr == NULL || _tcslen(pAttr->GetValue()) == 0)
			return false;
		Value = pAttr->GetValue();
	}
	else
	{
		XmlNode *pxNode = NULL;
		if(!Name || _tcslen(Name) == 0)
		{
			pxNode = &rxParent;
		}
		else
		{
			pxNode = rxParent.GetChild(Name);
		}
		if(pxNode == NULL || pxNode->GetTextField() == NULL || _tcslen(pxNode->GetTextField()->GetText()) == 0)
			return false;
		Value = pxNode->GetTextField()->GetText();
	}
	return true;
}

void xmlString::SetDefault(LPCTSTR Value)
{
	if(Value == NULL)
	{
		m_DefaultValue = _T("");
	}
	else
	{
		m_DefaultValue = Value;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlInt
///////////////////////////////////////////////////////////////////////////////////////////////////////////
xmlInt::xmlInt(int NameId, __int64 Default /*= 0*/)
	: xmlValue(NameId, false),
	m_DefaultValue(Default),
	m_Value(Default)
{
}

xmlInt::xmlInt(const xmlInt &r)
	: xmlValue(r)
{
	m_Value = r.m_Value;
	m_DefaultValue = r.m_DefaultValue;
}

bool xmlInt::Set(const xmlInt &rValue)
{
	if(!xmlItem::SetItem(rValue))
		return false;
	if(!rValue.IsValid())
	{
		xmlValue::Clean();
	}
	else
	{
		SetInt(rValue.m_Value);
	}
	return true;
}

void xmlInt::SetInt(__int64 Value)
{
	m_Value = Value;
	Validate();
}

void xmlInt::SetIntIfModified(__int64 Value)
{
	if(IsValid() || m_DefaultValue != Value)
	{
		SetInt(Value);
	}
}

__int64 xmlInt::GetInt() const
{
	if(!IsValid())
	{
		return m_DefaultValue;
	}
	return m_Value;
}

__int64 xmlInt::GetInt(__int64 Default) const
{
	if(!IsValid())
		return Default;
	return m_Value;
}

bool xmlInt::IsEqualeTo(const xmlInt &rValue) const
{
	if(!IsValid() || !rValue.IsValid())
		return false;
	return (m_Value == rValue.m_Value);
}

bool xmlInt::IsCaseInsensitiveEqualeTo(const xmlInt &rValue) const
{
	return IsEqualeTo(rValue);
}

bool xmlInt::SetString(LPCTSTR Value)
{
	__int64 intVal;

	if(!ConvertFromString(intVal, Value))
		return false;

	m_Value = intVal;
	Validate();
	return true;
}

LPCTSTR xmlInt::GetAssignedString(_tstring &rValue) const
{
	if(!IsValid())
		return NULL;
	ConvertToString(m_Value, rValue);
	return rValue.c_str();
}

bool xmlInt::Serialize(WritableBinStorage &rBinStorage)
{
	return rBinStorage.Put((LPBYTE)&m_Value, sizeof(m_Value));
}

bool xmlInt::Deserialize(ReadableBinStorage &rBinStorage)
{
	Clean();

	__int64 Value;
	if(!rBinStorage.Get((LPBYTE)&Value, sizeof(Value)))
		return false;
	if(CheckValueValidity(Value))
	{
		m_Value = Value;
		Validate();
	}
	return true;
}

bool xmlInt::ConvertFromString(__int64 &intVal, LPCTSTR Value) const
{
	if(_stscanf_s(Value, _T("%I64d"), &intVal) < 1)
		return false;
	return true;
}

bool xmlInt::ConvertToString(__int64 IntVal, _tstring &Value) const
{
	TCHAR StringVal[20];
	_stprintf_s(StringVal, sizeof(StringVal) / sizeof(StringVal[0]), _T("%I64d"), IntVal);

	Value = StringVal;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlBool
///////////////////////////////////////////////////////////////////////////////////////////////////////////
xmlBool::xmlBool(int NameId, bool Default /*= false*/)
	: xmlValue(NameId, false),
	m_DefaultValue(Default),
	m_Value(Default)
{
}

xmlBool::xmlBool(const xmlBool &r)
	: xmlValue(r)
{
	m_Value = r.m_Value;
	m_DefaultValue = r.m_DefaultValue;
}

bool xmlBool::Set(const xmlBool &rValue)
{
	if(!xmlItem::SetItem(rValue))
		return false;
	if(!rValue.IsValid())
	{
		xmlValue::Clean();
	}
	else
	{
		SetBool(rValue.m_Value);
	}
	return true;
}

void xmlBool::SetBool(bool Value)
{
	m_Value = Value;
	Validate();
}

void xmlBool::SetBoolIfModified(bool Value)
{
	if(IsValid() || m_DefaultValue != Value)
	{
		SetBool(Value);
	}
}

bool xmlBool::GetBool() const
{
	if(!IsValid())
	{
		return m_DefaultValue;
	}
	return m_Value;
}

bool xmlBool::GetBool(bool Default) const
{
	if(!IsValid())
		return Default;
	return m_Value;
}

bool xmlBool::IsEqualeTo(const xmlBool &rValue) const
{
	if(!IsValid() || !rValue.IsValid())
		return false;
	return (m_Value == rValue.m_Value);
}

bool xmlBool::IsCaseInsensitiveEqualeTo(const xmlBool &rValue) const
{
	return IsEqualeTo(rValue);
}

bool xmlBool::SetString(LPCTSTR Value)
{
	int intVal;
	if(_stscanf_s(Value, _T("%d"), &intVal) < 1)
		return false;
	m_Value = (intVal != FALSE);
	Validate();
	return true;
}

LPCTSTR xmlBool::GetAssignedString(_tstring &rValue) const
{
	if(!IsValid())
		return NULL;
	TCHAR String[256];
	_stprintf_s(String, sizeof(String) / sizeof(String[0]), _T("%d"), m_Value);
	rValue = String;
	return rValue.c_str();
}

bool xmlBool::Serialize(WritableBinStorage &rBinStorage)
{
	return rBinStorage.Put((LPBYTE)&m_Value, sizeof(m_Value));
}

bool xmlBool::Deserialize(ReadableBinStorage &rBinStorage)
{
	if(!rBinStorage.Get((LPBYTE)&m_Value, sizeof(m_Value)))
		return false;
	Validate();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlTime
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void xmlTime::SetTime(const FILETIME &Time)
{
	LARGE_INTEGER lintTime;
	lintTime.HighPart = Time.dwHighDateTime;
	lintTime.LowPart = Time.dwLowDateTime;
	SetInt(lintTime.QuadPart);
}

bool xmlTime::GetTime(FILETIME &Time) const
{
	if(!IsValid())
		return false;
	LARGE_INTEGER lintTime;
	lintTime.QuadPart = GetInt();
	Time.dwHighDateTime = lintTime.HighPart;
	Time.dwLowDateTime = lintTime.LowPart;
	return true;
}

bool xmlTime::ConvertFromString(__int64 &IntVal, LPCTSTR Value) const
{
	int Year = 0;
	int Month = 0;
	int DayOfWeek = 0;
	int Day = 0;
	int Hour = 0;
	int Minute = 0;
	int Second = 0;
	int Milliseconds = 0;

	if(_stscanf_s(
			Value, 
			_T("%d:%d:%d:%d:%d:%d:%d:%d"),
			&Year,
			&Month,
			&DayOfWeek,
			&Day,
			&Hour,
			&Minute,
			&Second,
			&Milliseconds
			) != 8)
	{
		return false;
	}

	SYSTEMTIME SysTime;
	SysTime.wYear = (WORD)Year;
	SysTime.wMonth = (WORD)Month;
	SysTime.wDayOfWeek = (WORD)DayOfWeek;
	SysTime.wDay = (WORD)Day;
	SysTime.wHour = (WORD)Hour;
	SysTime.wMinute = (WORD)Minute;
	SysTime.wSecond = (WORD)Second;
	SysTime.wMilliseconds = (WORD)Milliseconds;

	FILETIME FileTime;
	if(!::SystemTimeToFileTime(&SysTime, &FileTime))
		return false;

	LARGE_INTEGER lintVal;
	lintVal.HighPart = FileTime.dwHighDateTime;
	lintVal.LowPart = FileTime.dwLowDateTime;

	IntVal = lintVal.QuadPart;
	return true;
}

bool xmlTime::ConvertToString(__int64 IntVal, _tstring &Value) const
{
	LARGE_INTEGER lintVal;
	lintVal.QuadPart = IntVal;

	FILETIME FileTime;
	FileTime.dwHighDateTime = lintVal.HighPart;
	FileTime.dwLowDateTime = lintVal.LowPart;

	SYSTEMTIME SysTime;
	if(!::FileTimeToSystemTime(&FileTime, &SysTime))
		return false;

	TCHAR TimeString[256];
	_stprintf_s(
			TimeString, sizeof(TimeString) / sizeof(TimeString[0]),
			_T("%d:%d:%d:%d:%d:%d:%d:%d"),
			SysTime.wYear,
			SysTime.wMonth,
			SysTime.wDayOfWeek,
			SysTime.wDay,
			SysTime.wHour,
			SysTime.wMinute,
			SysTime.wSecond,
			SysTime.wMilliseconds
			);
	Value = TimeString;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlTree
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void xmlTree::Initialize(xmlItem &rXmlRoot, LPCTSTR FileName, int xmlRootFieldNameId)
{
	m_pXmlRoot = &rXmlRoot;
	m_FileName = FileName;
	m_xmlRootFieldNameId = xmlRootFieldNameId;
}

void xmlTree::Clean()
{
	if(m_pXmlRoot)
	{
		m_pXmlRoot->Clean();
	}
}

bool xmlTree::LoadXml(LPCTSTR xml /*= NULL*/)
{
	if(!m_pXmlRoot)
		return false;

	m_pXmlRoot->Clean();

	LPCTSTR XMLWorkString = NULL;
	_tstring XmlStringContainer;
	
	if(xml)
	{
		XMLWorkString = xml;
	}
	else
	{
		// create a string from the xml file
		FILE *XMLFile = NULL;
		_tfopen_s(&XMLFile, m_FileName.c_str(), _T("rb"));
		if(!XMLFile)
			return false;
		fseek(XMLFile, 0, SEEK_END);
		int FileSize = ftell(XMLFile);
		fseek(XMLFile, 0, SEEK_SET);

		LPBYTE XMLBuff = new BYTE[FileSize + 1];
		if(!XMLBuff)
		{
			fclose(XMLFile);
			return false;
		}
		fread(XMLBuff, 1, FileSize, XMLFile);
		fclose(XMLFile);

		int Encoding;
		if(!XmlNode::DetectEncoding(XMLBuff, FileSize, Encoding))
		{
			delete []XMLBuff;
			return false;
		}

		switch(Encoding)
		{
		case ENCODING_TYPE_UTF_8:
			SetString(XmlStringContainer, (LPCSTR)XMLBuff, FileSize);
			break;
		case ENCODING_TYPE_UTF_16:
			SetString(XmlStringContainer, (LPCWSTR)XMLBuff, FileSize / sizeof(WCHAR));
			break;
		}

		delete []XMLBuff;

		XMLWorkString = XmlStringContainer.c_str();
	}

	if(!XMLWorkString)
		return false;
	XMLWorkString = _tcschr(XMLWorkString, _T('<')); // skip garbage before first expression
	if(!XMLWorkString)
		return false;

	// parse xml string
	XmlNode xRootNode;
	size_t Length = _tcslen(XMLWorkString);
	xRootNode.Load(XMLWorkString, Length);
	XmlNode *pWorkingNode = NULL;
	if(m_xmlRootFieldNameId == 0)
	{
		pWorkingNode = &xRootNode;
	}
	else
	{
		pWorkingNode = xRootNode.GetChild(GetResourceString(m_xmlRootFieldNameId));
	}

	if(pWorkingNode == NULL)
		return false;

	if(!m_pXmlRoot->Load(*pWorkingNode))
		return false;
	return true;
}

bool xmlTree::StoreXml(int Encoding)
{
	if(!m_pXmlRoot)
		return false;

	XmlNode xRootNode;
	xRootNode.SetAsXmlDecleration();
	XmlNode *pWorkingNode = xRootNode.AddChild(GetResourceString(m_xmlRootFieldNameId));
	if(pWorkingNode == NULL)
		return false;

	m_pXmlRoot->Store(*pWorkingNode);
	if(!xRootNode.SaveToFile(m_FileName.c_str(), Encoding))
		return false;
	return true;
}

bool xmlTree::Serialize()
{
	if(!m_pXmlRoot)
		return false;

	WritableBinStorage Storage;
	if(!m_pXmlRoot->Serialize(Storage))
		return false;

	if(!Storage.WriteToFile(m_FileName.c_str()))
		return false;

	return true;
}

bool xmlTree::Deserialize()
{
	if(!m_pXmlRoot)
		return false;

	m_pXmlRoot->Clean();

	ReadableBinStorage Storage;
	if(!Storage.ReadFromFile(m_FileName.c_str()))
		return false;

	if(!m_pXmlRoot->Deserialize(Storage))
		return false;

	return true;
}
