#ifndef _WINDOW_ITEM_PROPERTY_TYPES_HEADER_
#define _WINDOW_ITEM_PROPERTY_TYPES_HEADER_

class WindowItemProperty;
typedef WindowItemProperty *(*T_WindowItemPropertyCreator)(int XmlNameId);
class xmlPropertyTypeValue;

class WindowItemPropertyTypes
{
public:
	WindowItemPropertyTypes();
	virtual ~WindowItemPropertyTypes();

	void AddType(int Type, LPCTSTR TypeString, T_WindowItemPropertyCreator Creator, bool fIsReadOnly = false);

	bool Find(int Type, _tstring &TypeString) const;
	bool Find(int Type, T_WindowItemPropertyCreator &Creator, bool &rfIsReadOnly) const;
	bool Find(LPCTSTR TypeString, int &Type, T_WindowItemPropertyCreator &Creator, bool &rfIsReadOnly) const;
	xmlPropertyTypeValue *FindXmlString(int Type) const;

	size_t GetCount() const;
	bool GetType(int Index, int &Type, T_WindowItemPropertyCreator &Creator) const;
private:
	void Close();
private:
	// factory maps
	struct CreatorInfo
	{
		T_WindowItemPropertyCreator Function;
		int Type;
		_tstring TypeString;
		bool fIsReadOnly;
	};

	typedef std::map<int, CreatorInfo> PropertyCreatorMap;
	PropertyCreatorMap m_PropertyCreatorMap;

	typedef std::map<_tstring, CreatorInfo> PropertyStringCreatorMap;
	PropertyStringCreatorMap m_PropertyStringCreatorMap;

	typedef std::map<int, xmlPropertyTypeValue *> xmlStringMap;
	xmlStringMap m_xmlStringMap;

	std::vector<int> m_PropertyTypeList; // not for read only properties
};

#endif // _WINDOW_ITEM_PROPERTY_TYPES_HEADER_