#ifndef _XML_PARSER_HEADER_
#define _XML_PARSER_HEADER_

enum EncodingType
{
	ENCODING_TYPE_UTF_8 = 0,
	ENCODING_TYPE_UTF_16 = 1,
};

class EncodedCharacters
{
public:
	EncodedCharacters();
	virtual ~EncodedCharacters() {}

	LPCTSTR FindCharString(TCHAR Char) const;
	bool FindStringChar(LPCTSTR String, size_t Length, TCHAR &Char, size_t &rnRefStringLength) const;
private:
	typedef std::map<TCHAR, _tstring> CharMap;
	CharMap m_CharMap;
};

class XmlObject
{
public:
	XmlObject()				{}
	virtual ~XmlObject()	{}

	virtual bool Store(_tstring &XmlString, int nIndetation, bool fNewline) = 0;

	static size_t GetTextLength(LPCTSTR XmlString, size_t Length, LPCTSTR TerminatingCharacters);
	static bool SkipCharcaters(LPCTSTR &XmlString, size_t &Length, LPCTSTR SkippedCharacters);

	static bool LoadText(LPCTSTR &XmlString, size_t &Length, LPCTSTR TerminatingCharacters, _tstring &Text);
	static bool StoreText(_tstring &XmlString, LPCTSTR Text);
	static bool VerifyNextStringAndLoad(LPCTSTR &XmlString, size_t &Length, LPCTSTR RefString);
protected:
	void StoreNewLine(_tstring &XmlString) const;
	void StoreIndentation(_tstring &XmlString, int nIndetation) const;
private:
	static EncodedCharacters m_gEncodedCharacters;
};

class XmlAttribute;
class XmlText;
class XmlComment;

class XmlNode : public XmlObject
{
public:
	XmlNode();
	virtual ~XmlNode();

	static bool DetectEncoding(LPBYTE pBuffer, int Size, int &EncodingType);

	bool Load(LPCTSTR &XmlString, size_t &Length);
	virtual bool Store(_tstring &XmlString, int nIndetation, bool fNewline);
	bool SaveToFile(LPCTSTR FileName, int EncodingType);

	// query
	bool IsXmlDecleration() const						{ return m_fIsXmlDeclaration; }
	LPCTSTR GetName() const								{ return m_Name.c_str(); }

	size_t GetAttributeCount() const						{ return m_AttributeList.size(); }
	XmlAttribute *GetAttribute(index_t nIndex = 0) const;
	int GetAttributeCount(LPCTSTR Name) const;
	XmlAttribute *GetAttribute(LPCTSTR Name, index_t nIndex = 0) const;

	size_t GetChildCount() const							{ return m_ChildList.size(); }
	XmlNode *GetChild(index_t nIndex = 0) const;
	size_t GetChildCount(LPCTSTR Name) const;
	XmlNode *GetChild(LPCTSTR Name, index_t nIndex = 0) const;

	size_t GetTextCount() const							{ return m_TextList.size(); }
	XmlText *GetTextField(index_t nIndex = 0) const;

	size_t GetCommentCount() const							{ return m_CommentList.size(); }
	XmlComment *GetCommentField(index_t nIndex = 0) const;

	// set
	bool SetAsXmlDecleration(LPCTSTR Version = _T("1.0"), LPCTSTR Encoding = _T("utf-8"));
	bool SetEncoding(LPCTSTR Encoding);
	void SetName(LPCTSTR Name)							{ m_Name = Name; }

	XmlAttribute *AddAttribute(LPCTSTR Name = NULL);
	XmlNode *AddChild(LPCTSTR Name = NULL);
	XmlText *AddText(LPCTSTR Text = NULL);
	XmlComment *AddComment(LPCTSTR Text = NULL);
private:
	void Close();
private:
	bool m_fIsXmlDeclaration;

	_tstring m_Name;

	std::vector<XmlAttribute *> m_AttributeList;
	std::vector<XmlNode *> m_ChildList;
	std::vector<XmlText *> m_TextList;
	std::vector<XmlComment *> m_CommentList;

	std::vector<XmlObject *> m_OrderedObjectList;
};

class XmlAttribute
{
public:
	XmlAttribute()			{}
	virtual ~XmlAttribute()	{}

	bool Load(LPCTSTR &XmlString, size_t &Length);
	bool Store(_tstring &XmlString);

	LPCTSTR GetName() const	{ return m_Name.c_str(); }
	LPCTSTR GetValue() const { return m_Value.c_str(); }

	void SetName(LPCTSTR Name)	{ m_Name = Name; }
	void SetValue(LPCTSTR Value) { m_Value = Value; }
private:
	_tstring m_Name;
	_tstring m_Value;
};

class XmlText : public XmlObject
{
public:
	XmlText()			{}
	virtual ~XmlText()	{}

	bool Load(LPCTSTR &XmlString, size_t &Length);
	virtual bool Store(_tstring &XmlString, int nIndetation, bool fNewline);

	LPCTSTR GetText() const	{ return m_Text.c_str(); }

	void SetText(LPCTSTR Text)	{ m_Text = Text; }
private:
	_tstring m_Text;
};

class XmlComment : public XmlObject
{
public:
	XmlComment()			{}
	virtual ~XmlComment()	{}

	bool Load(LPCTSTR &XmlString, size_t &Length);
	virtual bool Store(_tstring &XmlString, int nIndetation, bool fNewline);

	LPCTSTR GetText() const	{ return m_Text.c_str(); }

	void SetText(LPCTSTR Text)	{ m_Text = Text; }
private:
	_tstring m_Text;
};

#endif // _XML_PARSER_HEADER_
