#include "Pch.h"
#include "XmlParser.h"
#include "StringConvert.h"

#define INDENT_STEP 2

BYTE UTF_8_FileHeaderSize[] = { 0xEF, 0xBB, 0xBF };
BYTE UTF_16_FileHeaderSize[] = { 0xFF, 0xFE };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EncodedCharacters
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EncodedCharacters::EncodedCharacters()
{
	m_CharMap.insert(CharMap::value_type(_T('&'), _T("&amp;")));
	m_CharMap.insert(CharMap::value_type(_T('<'), _T("&lt;")));
	m_CharMap.insert(CharMap::value_type(_T('>'), _T("&gt;")));
	m_CharMap.insert(CharMap::value_type(_T('\"'), _T("&quot;")));
	m_CharMap.insert(CharMap::value_type(_T('\''), _T("&apos;")));
}

LPCTSTR EncodedCharacters::FindCharString(TCHAR Char) const
{
	CharMap::const_iterator Iterator = m_CharMap.find(Char);
	if(Iterator == m_CharMap.end())
		return NULL;
	return (*Iterator).second.c_str();
}

bool EncodedCharacters::FindStringChar(LPCTSTR String, size_t Length, TCHAR &Char, size_t &rnRefStringLength) const
{
	CharMap::const_iterator Iterator = m_CharMap.begin();
	for(; Iterator != m_CharMap.end(); Iterator++)
	{
		const _tstring &RefString = (*Iterator).second;

		if((int)RefString.length() <= Length)
		{
			if(_tcsncmp(String, &RefString[0], RefString.length()) == 0)
			{
				Char = (*Iterator).first;
				rnRefStringLength = RefString.length();
				return true;
			}
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// XmlObject
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EncodedCharacters XmlObject::m_gEncodedCharacters;

size_t XmlObject::GetTextLength(LPCTSTR XmlString, size_t Length, LPCTSTR TerminatingCharacters)
{
	size_t nTerminatingChars = _tcslen(TerminatingCharacters);
	index_t nChars;
	for(nChars = 0; nChars < (index_t)Length; nChars++)
	{
		for(index_t i = 0; i < (index_t)nTerminatingChars; i++)
		{
			if(XmlString[nChars] == TerminatingCharacters[i])
				return (size_t)nChars;
		}
	}
	return (size_t)nChars;
}

bool XmlObject::SkipCharcaters(LPCTSTR &XmlString, size_t &Length, LPCTSTR SkippedCharacters)
{
	size_t nSkippedChars = _tcslen(SkippedCharacters);
	size_t nChars;
	for(nChars = 0; nChars < Length; nChars++)
	{
		bool fMatch = false;
		for(index_t i = 0; i < (index_t)nSkippedChars; i++)
		{
			if(XmlString[nChars] == SkippedCharacters[i])
			{
				fMatch = true;
			}
		}
		if(!fMatch)
		{
			XmlString += nChars;
			Length -= nChars;
			break;
		}
	}
	if(Length == 0)
		return false;
	return true;
}

bool XmlObject::LoadText(LPCTSTR &XmlString, size_t &Length, LPCTSTR TerminatingCharacters, _tstring &Text)
{
	Text = _T("");
	if(Length == 0)
		return false;
	size_t nTextLength = GetTextLength(XmlString, Length, TerminatingCharacters);
	if(nTextLength > 0)
	{
		size_t nReducedLength = Length - nTextLength;
		while(nReducedLength < Length)
		{
			TCHAR Char;
			size_t nRefStringLength;
			if(m_gEncodedCharacters.FindStringChar(XmlString, Length, Char, nRefStringLength))
			{
				Text += Char;
				XmlString += nRefStringLength;
				Length -= nRefStringLength;
			}
			else
			{
				Text += XmlString[0];
				XmlString++;
				Length--;
			}
		}
	}
	return true;
}

bool XmlObject::StoreText(_tstring &XmlString, LPCTSTR Text)
{
	size_t nTextLength = _tcslen(Text);

	for(index_t i = 0; i < (index_t)nTextLength; i++)
	{
		LPCTSTR ReplacingString = m_gEncodedCharacters.FindCharString(Text[i]);
		if(ReplacingString != NULL)
		{
			XmlString += ReplacingString;
		}
		else
		{
			XmlString += Text[i];
		}
	}
	return true;
}

bool XmlObject::VerifyNextStringAndLoad(LPCTSTR &XmlString, size_t &Length, LPCTSTR RefString)
{
	size_t nTextLength = _tcslen(RefString);
	if(_tcsncmp(XmlString, RefString, nTextLength) != 0)
		return false;
	XmlString += nTextLength;
	Length -= nTextLength;
	return true;
}

void XmlObject::StoreNewLine(_tstring &XmlString) const
{
	XmlString += _T("\r\n");
}

void XmlObject::StoreIndentation(_tstring &XmlString, int nIndetation) const
{
	int nSpaceCharacters = nIndetation * INDENT_STEP;
	for(int i = 0; i < nSpaceCharacters; i++)
	{
		XmlString += _T(" ");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// XmlNode
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XmlNode::XmlNode()
	: m_fIsXmlDeclaration(false)
{
}

XmlNode::~XmlNode()
{
	Close();
}

bool XmlNode::DetectEncoding(LPBYTE pBuffer, int Size, int &EncodingType)
{
	if(memcmp(pBuffer, UTF_8_FileHeaderSize, sizeof(UTF_8_FileHeaderSize)) == 0)
	{
		EncodingType = ENCODING_TYPE_UTF_8;
		return true;
	}
	if(memcmp(pBuffer, UTF_16_FileHeaderSize, sizeof(UTF_16_FileHeaderSize)) == 0)
	{
		EncodingType = ENCODING_TYPE_UTF_16;
		return true;
	}

	// try wide-char
	LPCWSTR lpwString = (LPCWSTR)pBuffer;
	LPCWSTR wFirstChar = wcschr(lpwString, L'<');
	if(wFirstChar != NULL && (int)wFirstChar < (int)(pBuffer + Size))
	{
		if(wcsncmp(wFirstChar, L"<?xml", wcslen(L"<?xml")) == 0)
		{
			EncodingType = ENCODING_TYPE_UTF_16;
			return true;
		}
	}
	// try multi-byte
	LPCSTR lpString = (LPCSTR)pBuffer;
	LPCSTR FirstChar = strchr(lpString, '<');
	if(FirstChar != NULL && (int)FirstChar < (int)(pBuffer + Size))
	{
		if(strncmp(FirstChar, "<?xml", strlen("<?xml")) == 0)
		{
			EncodingType = ENCODING_TYPE_UTF_8;
			return true;
		}
	}
	return false;
}

bool XmlNode::Load(LPCTSTR &XmlString, size_t &Length)
{
	Close();

	// read prefix
	if(!XmlObject::VerifyNextStringAndLoad(XmlString, Length, _T("<")))
		return false;
	if(!XmlObject::LoadText(XmlString, Length, _T(" \n\r\t/><"), m_Name))
		return false;
	if(m_Name == _T("?xml"))
	{
		m_fIsXmlDeclaration = true;
	}
	if(!XmlObject::SkipCharcaters(XmlString, Length, _T(" \n\r\t")))
		return false;

	// read attributes
	while(_tcsncmp(XmlString, _T(">"), _tcslen(_T(">"))) != 0 && _tcsncmp(XmlString, _T("/>"), _tcslen(_T("/>"))) != 0 && _tcsncmp(XmlString, _T("?>"), _tcslen(_T("?>"))) != 0)
	{
		XmlAttribute *pAttr = AddAttribute();
		if(!pAttr)
			return false;
		if(!pAttr->Load(XmlString, Length))
			return false;
		if(!XmlObject::SkipCharcaters(XmlString, Length, _T(" \n\r\t")))
			return false;
	}

	if(m_fIsXmlDeclaration)
	{
		if(!XmlObject::VerifyNextStringAndLoad(XmlString, Length, _T("?>")))
			return false;
	}
	else if(XmlObject::VerifyNextStringAndLoad(XmlString, Length, _T("/>")))
	{
		return true;
	}
	else if(XmlObject::VerifyNextStringAndLoad(XmlString, Length, _T(">")))
	{
	}
	else
	{
		return false;
	}

	// rwead objects (nodes, text, comments, temination
	bool fEnded = false;
	while(!fEnded)
	{
		if(!XmlObject::SkipCharcaters(XmlString, Length, _T(" \n\r\t")))
		{
			if(m_fIsXmlDeclaration)
			{
				fEnded = true;
				break;
			}
			else
			{
				return false;
			}
		}
		if(_tcsncmp(XmlString, _T("<!--"), _tcslen(_T("<!--"))) == 0)
		{
			XmlComment *pComment = AddComment();
			if(!pComment)
				return false;
			if(!pComment->Load(XmlString, Length))
				return false;
		}
		else if(_tcsncmp(XmlString, _T("</"), _tcslen(_T("</"))) == 0)
		{
			if(m_fIsXmlDeclaration)
				return false;
			if(!XmlObject::VerifyNextStringAndLoad(XmlString, Length, _T("</")))
				return false;
			if(!XmlObject::VerifyNextStringAndLoad(XmlString, Length, m_Name.c_str()))
				return false;
			if(!XmlObject::SkipCharcaters(XmlString, Length, _T(" \n\r\t")))
				return false;
			if(!XmlObject::VerifyNextStringAndLoad(XmlString, Length, _T(">")))
				return false;
			fEnded = true;
		}
		else if(_tcsncmp(XmlString, _T("<"), _tcslen(_T("<"))) == 0)
		{
			XmlNode *pChild = AddChild();
			if(!pChild)
				return false;
			if(!pChild->Load(XmlString, Length))
				return false;
		}
		else
		{
			XmlText *pText = AddText();
			if(!pText)
				return false;
			if(!pText->Load(XmlString, Length))
				return false;
		}
	}
	return true;
}

bool XmlNode::Store(_tstring &XmlString, int nIndetation, bool fNewline)
{
	if(fNewline)
	{
		StoreNewLine(XmlString);
		StoreIndentation(XmlString, nIndetation);
	}

	// prefix
	XmlString += _T("<");
	XmlObject::StoreText(XmlString, m_Name.c_str());

	// attributes
	unsigned int i;
	for(i = 0; i < m_AttributeList.size(); i++)
	{
		XmlString += _T(" ");
		if(!m_AttributeList.at(i)->Store(XmlString))
			return false;
	}

	if(m_fIsXmlDeclaration)
	{
		XmlString += _T("?>");
	}
	else if(m_OrderedObjectList.size() == 0)
	{
		XmlString += _T("/>");
		return true;
	}
	else
	{
		XmlString += _T(">");
	}

	bool fShouldNotPutNewLine = (m_ChildList.size() + m_CommentList.size() == 0 && m_TextList.size() == 1);

	int ChildrenIndentation = (m_fIsXmlDeclaration) ? nIndetation : nIndetation + 1;
	for(i = 0; i < m_OrderedObjectList.size(); i++)
	{
		if(!m_OrderedObjectList.at(i)->Store(XmlString, ChildrenIndentation, !fShouldNotPutNewLine))
			return false;
	}

	if(!m_fIsXmlDeclaration)
	{
		if(!fShouldNotPutNewLine)
		{
			StoreNewLine(XmlString);
			StoreIndentation(XmlString, nIndetation);
		}
		XmlString += _T("</");
		XmlObject::StoreText(XmlString, m_Name.c_str());
		XmlString += _T(">");
	}
	return true;
}

bool XmlNode::SaveToFile(LPCTSTR FileName, int EncodingType)
{
	BYTE FileHeader[10];
	int FileHeaderSize = 0;

	_tstring TargetString;
	LPBYTE pWriteBuffer = NULL;
	size_t WriteBufferSize = 0;
	std::string mbString;
	std::wstring wString;

	switch(EncodingType)
	{
	case ENCODING_TYPE_UTF_8:
		if(!SetEncoding(_T("utf-8")))
			return false;
		if(!Store(TargetString, 0, false))
			return false;
		
		memcpy(FileHeader, UTF_8_FileHeaderSize, sizeof(UTF_8_FileHeaderSize));
		FileHeaderSize = sizeof(UTF_8_FileHeaderSize);

		SetString(mbString, TargetString.c_str());
		pWriteBuffer = (LPBYTE)(&mbString[0]);
		WriteBufferSize = mbString.length();
		break;
	case ENCODING_TYPE_UTF_16:
		if(!SetEncoding(_T("utf-16")))
			return false;
		if(!Store(TargetString, 0, false))
			return false;

		memcpy(FileHeader, UTF_16_FileHeaderSize, sizeof(UTF_16_FileHeaderSize));
		FileHeaderSize = sizeof(UTF_16_FileHeaderSize);

		SetString(wString, TargetString.c_str());
		pWriteBuffer = (LPBYTE)(&wString[0]);
		WriteBufferSize = wString.length() * sizeof(WCHAR);
		break;
	default:
		return false;
	}

	if(pWriteBuffer == NULL)
		return false;

	FILE *fp = NULL;
	_tfopen_s(&fp, FileName, _T("wb"));
	if(fp == NULL)
		return false;
	if(FileHeaderSize > 0)
	{
		fwrite(FileHeader, 1, FileHeaderSize, fp);
	}
	fwrite(pWriteBuffer, 1, WriteBufferSize, fp);
	fclose(fp);
	return true;
}

XmlAttribute *XmlNode::GetAttribute(index_t nIndex /*= 0*/) const
{
	if(nIndex < 0 || nIndex >= (index_t)m_AttributeList.size())
		return NULL;
	return m_AttributeList.at(nIndex);
}

int XmlNode::GetAttributeCount(LPCTSTR Name) const
{
	int nCount = 0;
	for(int i = 0; i < (int)m_AttributeList.size(); i++)
	{
		if(_tcscmp(m_AttributeList.at(i)->GetName(), Name) == 0)
		{
			nCount++;
		}
	}
	return nCount;
}

XmlAttribute *XmlNode::GetAttribute(LPCTSTR Name, index_t nIndex /*= 0*/) const
{
	index_t nCount = 0;
	for(index_t i = 0; i < (index_t)m_AttributeList.size(); i++)
	{
		if(Name == NULL || _tcslen(Name) == 0 || _tcscmp(m_AttributeList.at(i)->GetName(), Name) == 0)
		{
			if(nCount == nIndex)
			{
				return m_AttributeList.at(i);
			}
			else
			{
				nCount++;
			}
		}
	}
	return NULL;
}

XmlNode *XmlNode::GetChild(index_t nIndex /*= 0*/) const
{
	if(nIndex < 0 || nIndex >= (index_t)m_ChildList.size())
		return NULL;
	return m_ChildList.at(nIndex);
}

size_t XmlNode::GetChildCount(LPCTSTR Name) const
{
	size_t nCount = 0;
	for(index_t i = 0; i < (index_t)m_ChildList.size(); i++)
	{
		if(_tcscmp(m_ChildList.at(i)->GetName(), Name) == 0)
		{
			nCount++;
		}
	}
	return nCount;
}

XmlNode *XmlNode::GetChild(LPCTSTR Name, index_t nIndex /*= 0*/) const
{
	size_t nCount = 0;
	for(index_t i = 0; i < (index_t)m_ChildList.size(); i++)
	{
		if(Name == NULL || _tcslen(Name) == 0 || _tcscmp(m_ChildList.at(i)->GetName(), Name) == 0)
		{
			nCount++;
			if(nCount == (size_t)(nIndex + 1))
			{
				return m_ChildList.at(i);
			}
		}
	}
	return NULL;
}

XmlText *XmlNode::GetTextField(index_t nIndex /*= 0*/) const
{
	if(nIndex < 0 || nIndex >= (index_t)m_TextList.size())
		return NULL;
	return m_TextList.at(nIndex);
}

XmlComment *XmlNode::GetCommentField(index_t nIndex /*= 0*/) const
{
	if(nIndex < 0 || nIndex >= (index_t)m_CommentList.size())
		return NULL;
	return m_CommentList.at(nIndex);
}

bool XmlNode::SetAsXmlDecleration(LPCTSTR Version /*= _T("1.0")*/, LPCTSTR Encoding /*= _T("utf-8")*/)
{
	m_fIsXmlDeclaration = true;
	SetName(_T("?xml"));

	XmlAttribute *pAttr = AddAttribute(_T("version"));
	if(pAttr == NULL)
		return false;
	pAttr->SetValue(Version);

	pAttr = AddAttribute(_T("encoding"));
	if(pAttr == NULL)
		return false;
	pAttr->SetValue(Encoding);
	return true;
}

bool XmlNode::SetEncoding(LPCTSTR Encoding)
{
	if(!m_fIsXmlDeclaration)
		return false;
	XmlAttribute *pAttr = GetAttribute(_T("encoding"));
	if(pAttr == NULL)
		return false;
	pAttr->SetValue(Encoding);
	return true;
}

XmlAttribute *XmlNode::AddAttribute(LPCTSTR Name /*= NULL*/)
{
	XmlAttribute *pNew = new XmlAttribute;
	if(pNew == NULL)
		return NULL;
	m_AttributeList.push_back(pNew);
	if(Name != NULL)
	{
		pNew->SetName(Name);
	}
	return pNew;
}

XmlNode *XmlNode::AddChild(LPCTSTR Name /*= NULL*/)
{
	XmlNode *pNew = new XmlNode;
	if(pNew == NULL)
		return NULL;
	m_ChildList.push_back(pNew);
	m_OrderedObjectList.push_back(pNew);
	if(Name != NULL)
	{
		pNew->SetName(Name);
	}
	return pNew;
}

XmlText *XmlNode::AddText(LPCTSTR Text /*= NULL*/)
{
	XmlText *pNew = new XmlText;
	if(pNew == NULL)
		return NULL;
	m_TextList.push_back(pNew);
	m_OrderedObjectList.push_back(pNew);
	if(Text != NULL)
	{
		pNew->SetText(Text);
	}
	return pNew;
}

XmlComment *XmlNode::AddComment(LPCTSTR Text /*= NULL*/)
{
	XmlComment *pNew = new XmlComment;
	if(pNew == NULL)
		return NULL;
	m_CommentList.push_back(pNew);
	m_OrderedObjectList.push_back(pNew);
	if(Text != NULL)
	{
		pNew->SetText(Text);
	}
	return pNew;
}

void XmlNode::Close()
{
	m_fIsXmlDeclaration = false;

	unsigned int i;

	for(i = 0; i < m_AttributeList.size(); i++)
	{
		delete m_AttributeList.at(i);
	}
	m_AttributeList.erase(m_AttributeList.begin(), m_AttributeList.end());
	
	for(i = 0; i < m_ChildList.size(); i++)
	{
		delete m_ChildList.at(i);
	}
	m_ChildList.erase(m_ChildList.begin(), m_ChildList.end());

	for(i = 0; i < m_TextList.size(); i++)
	{
		delete m_TextList.at(i);
	}
	m_TextList.erase(m_TextList.begin(), m_TextList.end());

	for(i = 0; i < m_CommentList.size(); i++)
	{
		delete m_CommentList.at(i);
	}
	m_CommentList.erase(m_CommentList.begin(), m_CommentList.end());

	m_OrderedObjectList.erase(m_OrderedObjectList.begin(), m_OrderedObjectList.end());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// XmlAttribute
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool XmlAttribute::Load(LPCTSTR &XmlString, size_t &Length)
{
	// get name
	if(!XmlObject::LoadText(XmlString, Length, _T(" \n\r\t="), m_Name))
		return false;
	if(!XmlObject::SkipCharcaters(XmlString, Length, _T(" \n\r\t")))
		return false;

	if(!XmlObject::VerifyNextStringAndLoad(XmlString, Length, _T("=")))
		return false;
	if(!XmlObject::SkipCharcaters(XmlString, Length, _T(" \n\r\t")))
		return false;

	// get value
	if(!XmlObject::VerifyNextStringAndLoad(XmlString, Length, _T("\"")))
		return false;
	if(!XmlObject::LoadText(XmlString, Length, _T("\""), m_Value))
		return false;
	if(!XmlObject::VerifyNextStringAndLoad(XmlString, Length, _T("\"")))
		return false;
	return true;
}

bool XmlAttribute::Store(_tstring &XmlString)
{
	if(m_Name.length() == 0)
		return false;
	XmlObject::StoreText(XmlString, m_Name.c_str());
	XmlString += _T("=\"");
	XmlObject::StoreText(XmlString, m_Value.c_str());
	XmlString += _T("\"");
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// XmlText
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool XmlText::Load(LPCTSTR &XmlString, size_t &Length)
{
	if(!XmlObject::LoadText(XmlString, Length, _T("<"), m_Text))
		return false;
	return true;
}

bool XmlText::Store(_tstring &XmlString, int nIndetation, bool fNewline)
{
	if(fNewline)
	{
		StoreNewLine(XmlString);
		StoreIndentation(XmlString, nIndetation);
	}
	XmlObject::StoreText(XmlString, m_Text.c_str());
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// XmlComment
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool XmlComment::Load(LPCTSTR &XmlString, size_t &Length)
{
	size_t SuffixLength = _tcslen(_T("-->"));

	if(!XmlObject::VerifyNextStringAndLoad(XmlString, Length, _T("<!--")))
		return false;

	LPCTSTR TempXmlString = XmlString;
	size_t TempLength = Length;
	bool fFound = false;
	while(!fFound && Length > 0)
	{
		size_t nTextLength = XmlObject::GetTextLength(TempXmlString, TempLength, _T("-"));
		TempXmlString += nTextLength;
		TempLength -= nTextLength;

		if(_tcsncmp(TempXmlString, _T("-->"), SuffixLength) == 0)
		{
			fFound = true;
		}
	}
	if(!fFound)
		return false;

	size_t nTextLength = (Length - TempLength);
	m_Text.reserve(nTextLength + 1);
	m_Text.resize(nTextLength);
	_tcsncpy_s(&m_Text[0], nTextLength + 1, XmlString, nTextLength);
	
	XmlString = TempXmlString + SuffixLength;
	Length = TempLength - SuffixLength;

	return true;
}

bool XmlComment::Store(_tstring &XmlString, int nIndetation, bool fNewline)
{
	if(fNewline)
	{
		StoreNewLine(XmlString);
		StoreIndentation(XmlString, nIndetation);
	}
	XmlString += _T("<!--");
	XmlString += m_Text;
	XmlString += _T("-->");
	return true;
}
