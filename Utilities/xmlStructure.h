#ifndef _XML_STRUCTURE_HEADER_
#define _XML_STRUCTURE_HEADER_

#include "XmlParser.h"
#include "StringTable.h"
#include "Log.h"
#include "BinStorage.h"
#include "TimeHelpers.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlItem:
// This is the base class for all xml items
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class xmlItem
{
public:
	void SetNameId(int NameId)		{ m_NameId = NameId; }

	virtual xmlItem &GetXmlItem()	{ return *this; }
	bool CheckAge(const FileTime & UNUSED(rCurrentTime))	{ return true; }
	bool IsOutOfDate() const		{ return false; }

	void Release()	{ delete this; }

	virtual bool Load(XmlNode &rxParent, index_t Index = 0) = 0;
	virtual void Store(XmlNode &rxParent) = 0;

	virtual bool Serialize(WritableBinStorage &rBinStorage) = 0;
	virtual bool Deserialize(ReadableBinStorage &rBinStorage) = 0;

	virtual void Clean() = 0;
	virtual bool IsValid() const = 0;
protected:
	bool SetItem(const xmlItem &rRef);

	xmlItem(int NameId);
	xmlItem(const xmlItem &r);
	virtual ~xmlItem();

	friend class xmlTree;

	int GetXmlNameId() const;
	LPCTSTR GetXmlName() const;
	bool IsEmptyXmlName() const;
private:
	int m_NameId;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlValue:
// This is the base class for all xml leaves. It also represents a string value
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class xmlValue : public xmlItem
{
public:
	xmlValue(int NameId, bool fAttribute);
	xmlValue(const xmlValue &r);
	virtual ~xmlValue()	{}

	virtual bool IsValid() const	{ return m_fValueExists; }
	virtual void Clean() { m_fValueExists = false; }

	bool GetStringValue(_tstring &rValue, LPCTSTR Default) const;
protected:
	virtual bool Load(XmlNode &rxParent, index_t Index = 0);
	virtual void Store(XmlNode &rxParent);

	virtual bool SetString(LPCTSTR Value) = 0;
	virtual LPCTSTR GetAssignedString(_tstring &rValue) const = 0;

	void Validate()	{ m_fValueExists = true; }
private:
	const bool m_fAttribute;
	bool m_fValueExists;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlString:
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class xmlString : public xmlValue
{
public:
	xmlString(int NameId, LPCTSTR Default = NULL, bool fAttribute = false);
	xmlString(const xmlString &r);
	virtual ~xmlString();

	virtual bool SetString(LPCTSTR Value);
	virtual LPCTSTR GetAssignedString(_tstring &rValue) const;

	virtual bool Serialize(WritableBinStorage &rBinStorage);
	virtual bool Deserialize(ReadableBinStorage &rBinStorage);

	void SetStringIfModified(LPCTSTR Value);
	bool GetString(_tstring &rValue) const;

	bool Set(const xmlString &rValue);
	bool IsEqualeTo(const xmlString &rValue) const;
	bool IsCaseInsensitiveEqualeTo(const xmlString &rValue) const;

	static bool CheckValue(XmlNode &rxParent, int NameId, _tstring &Value, bool fAttribute = false);
protected:
	void SetDefault(LPCTSTR Value);
	LPCTSTR GetDefault() const	{ return m_DefaultValue.c_str(); }
	LPCTSTR GetValue() const	{ return m_Value.c_str(); }
private:
	_tstring m_Value;
	_tstring m_DefaultValue;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlInt:
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class xmlInt : public xmlValue
{
public:
	xmlInt(int NameId, __int64 Default = 0);
	xmlInt(const xmlInt &r);
	virtual ~xmlInt() {}

	bool Set(const xmlInt &rValue);

	virtual bool SetString(LPCTSTR Value);
	virtual LPCTSTR GetAssignedString(_tstring &rValue) const;

	virtual bool Serialize(WritableBinStorage &rBinStorage);
	virtual bool Deserialize(ReadableBinStorage &rBinStorage);

	void SetInt(__int64 Value);
	void SetIntIfModified(__int64 Value);
	__int64 GetInt() const;
	__int64 GetInt(__int64 Default) const;

	bool IsEqualeTo(const xmlInt &rValue) const;
	bool IsCaseInsensitiveEqualeTo(const xmlInt &rValue) const;
protected:
	virtual bool ConvertFromString(__int64 &IntVal, LPCTSTR Value) const;
	virtual bool ConvertToString(__int64 IntVal, _tstring &Value) const;
	virtual bool CheckValueValidity(__int64 UNUSED(Value)) const	{ return true; }
private:
	__int64 m_Value;
	__int64 m_DefaultValue;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlBool:
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class xmlBool : public xmlValue
{
public:
	xmlBool(int NameId, bool Default = false);
	xmlBool(const xmlBool &r);
	virtual ~xmlBool() {}

	bool Set(const xmlBool &rValue);

	virtual bool SetString(LPCTSTR Value);
	virtual LPCTSTR GetAssignedString(_tstring &rValue) const;

	virtual bool Serialize(WritableBinStorage &rBinStorage);
	virtual bool Deserialize(ReadableBinStorage &rBinStorage);

	void SetBool(bool Value);
	void SetBoolIfModified(bool Value);
	bool GetBool() const;
	bool GetBool(bool Default) const;

	bool IsEqualeTo(const xmlBool &rValue) const;
	bool IsCaseInsensitiveEqualeTo(const xmlBool &rValue) const;
private:
	bool m_Value;
	bool m_DefaultValue;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlTime
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class xmlTime : public xmlInt
{
public:
	xmlTime(int NameId) : xmlInt(NameId) {}

	void SetTime(const FILETIME &Time);
	bool GetTime(FILETIME &Time) const;
protected:
	virtual bool ConvertFromString(__int64 &IntVal, LPCTSTR Value) const;
	virtual bool ConvertToString(__int64 IntVal, _tstring &Value) const;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlFixedSizeStructure:
// This class represents a structure of other xml items of any types. MAXCOUNT represents the limit count of children
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T, size_t MAXCOUNT>
class xmlFixedSizeStructure : public xmlItem
{ 
public:
	xmlFixedSizeStructure(int NameId);
	virtual ~xmlFixedSizeStructure();

	bool AddItem(T &rItem);
	bool IsEmpty() const;

	virtual void Clean();
protected:
	virtual bool Load(XmlNode &rxParent, index_t Index = 0);
	virtual void Store(XmlNode &rxParent);

	virtual bool Serialize(WritableBinStorage &rBinStorage);
	virtual bool Deserialize(ReadableBinStorage &rBinStorage);

	virtual bool IsValid() const;

	bool LoadChildren(XmlNode &rxParent, index_t Index, bool fUseChildIndex);
protected:
	size_t m_nCount;
	T *m_ItemList[MAXCOUNT];
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlItemParent:
// This class represents a structure of other xml items of any types
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class xmlItemParent : public xmlItem
{ 
public:
	xmlItemParent(int NameId);
	virtual ~xmlItemParent();

	void Allocate(int nChildren);

	void AddItem(T &rItem);
protected:
	virtual bool Load(XmlNode &rxParent, index_t Index = 0);
	virtual void Store(XmlNode &rxParent);

	virtual bool Serialize(WritableBinStorage &rBinStorage);
	virtual bool Deserialize(ReadableBinStorage &rBinStorage);

	void RemoveItem(T &rItem);

	virtual void Clean();
	virtual bool IsValid() const;

	bool IsEmpty() const;

	bool LoadChildren(XmlNode &rxParent, index_t Index, bool fUseChildIndex);
protected:
	std::vector<T *> m_ItemList;
};

typedef xmlItemParent<xmlItem> xmlStructure;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlItemArray:
// This class represents a specific case of parent, where items are dynamically generated by Load function
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class xmlItemArray : public xmlItemParent<T>
{
public:
	xmlItemArray(int ArrayNameId = 0);
	virtual ~xmlItemArray();

	void SetItemNameId(int ItemNameId)	{ m_ItemNameId = ItemNameId; }
	size_t GetCount() const	{ return m_ItemList.size(); }
	const T *GetItemFromIndex(index_t nIndex) const;
	T *GetItemFromIndex(index_t nIndex);

	virtual void Clean();
protected:
	virtual void Store(XmlNode &rxParent);
	virtual bool Load(XmlNode &rxParent, index_t Index = 0);

	virtual bool Serialize(WritableBinStorage &rBinStorage);
	virtual bool Deserialize(ReadableBinStorage &rBinStorage);

	virtual bool IsValid() const;
protected:
	virtual T *CreateFromXml(
						XmlNode &rxItemNode,
						int NameId
						) = 0;
	virtual T *CreateFromBinStorage(
						ReadableBinStorage &rBinStorage,
						int NameId
 						) = 0;
private:
	int m_ItemNameId;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlFixedSizeMap:
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T, size_t MAXCOUNT>
class xmlFixedSizeMap : public xmlFixedSizeStructure<T, MAXCOUNT>
{
public:
	xmlFixedSizeMap(int ArrayNameId = 0);
	virtual ~xmlFixedSizeMap();

	void SetItemNameId(int ItemNameId)	{ m_ItemNameId = ItemNameId; }
	size_t GetCount() const				{ return m_nCount; }
	const T *GetItemFromIndex(index_t nIndex) const;
	T *GetItemFromIndex(index_t nIndex);
	const T *GetItemFromKey(int nKey) const;
	T *GetItemFromKey(int nKey);

	bool AddTItem(T *pItem, int nKey);

	virtual void Clean();
protected:
	virtual void Store(XmlNode &rxParent);
	virtual bool Load(XmlNode &rxParent, index_t Index = 0);

	virtual bool Serialize(WritableBinStorage &rBinStorage);
	virtual bool Deserialize(ReadableBinStorage &rBinStorage);

	virtual bool IsValid() const;
protected:
	virtual T *CreateFromXml(
						XmlNode &rxItemNode,
						int NameId,
						int &rnKey
						) = 0;
	virtual T *CreateFromBinStorage(
						ReadableBinStorage &rBinStorage,
						int NameId,
						int &rnKey
 						) = 0;
private:
	int m_ItemNameId;

	T *m_xmlItemMap[MAXCOUNT];
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlTree:
// This class represents the root of the xml
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class xmlTree
{
public:
	xmlTree() : m_pXmlRoot(NULL) {}
	virtual ~xmlTree() {}

	void Initialize(xmlItem &rXmlRoot, LPCTSTR FileName, int xmlRootFieldNameId);

	void Clean();

	bool LoadXml(LPCTSTR xml = NULL);
	bool StoreXml(int Encoding);

	bool Serialize();
	bool Deserialize();
public:
	xmlItem *m_pXmlRoot;
	_tstring m_FileName;
	int m_xmlRootFieldNameId;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlItemParent implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
inline xmlItemParent<T>::xmlItemParent(int NameId)
	: xmlItem(NameId)
{
}

template <class T>
inline xmlItemParent<T>::~xmlItemParent()
{
}

template <class T>
inline bool xmlItemParent<T>::Load(XmlNode &rxParent, index_t Index /*= 0*/)
{
	return LoadChildren(rxParent, Index, false);
}

template <class T>
inline bool xmlItemParent<T>::LoadChildren(XmlNode &rxParent, index_t Index, bool fUseChildIndex)
{
	XmlNode *pxNode = NULL;
	if(IsEmptyXmlName())
	{
		assert(Index == 0);
		pxNode = &rxParent;
	}
	else
	{
		pxNode = rxParent.GetChild(GetXmlName(), Index);
	}

	if(pxNode == NULL)
		return false;
	bool fSuccess = true;
	for(unsigned int i = 0; i < m_ItemList.size(); i++)
	{
		fSuccess &= m_ItemList.at(i)->GetXmlItem().Load(*pxNode, fUseChildIndex ? i : 0);
	}
	return fSuccess;
}

template <class T>
inline void xmlItemParent<T>::Store(XmlNode &rxParent)
{
	if(!IsValid())
		return;

	XmlNode *pxNode = NULL;
	if(IsEmptyXmlName())
	{
		pxNode = &rxParent;
	}
	else
	{
		pxNode = rxParent.AddChild(GetXmlName());
	}

	if(pxNode != NULL)
	{
		for(unsigned int i = 0; i < m_ItemList.size(); i++)
		{
			m_ItemList.at(i)->GetXmlItem().Store(*pxNode);
		}
	}
}

template <class T>
inline bool xmlItemParent<T>::Serialize(WritableBinStorage &rBinStorage)
{
	for(unsigned int i = 0; i < m_ItemList.size(); i++)
	{
		if(!m_ItemList.at(i)->GetXmlItem().Serialize(rBinStorage))
			return false;
	}
	return true;
}

template <class T>
inline bool xmlItemParent<T>::Deserialize(ReadableBinStorage &rBinStorage)
{
	for(unsigned int i = 0; i < m_ItemList.size(); i++)
	{
		if(!m_ItemList.at(i)->GetXmlItem().Deserialize(rBinStorage))
			return false;
	}
	return true;
}

template <class T>
inline void xmlItemParent<T>::Allocate(int nChildren)
{
	m_ItemList.get_allocator().allocate(nChildren);
}

template <class T>
inline void xmlItemParent<T>::AddItem(T &rItem)
{
	m_ItemList.push_back(&rItem);
}

template <class T>
inline void xmlItemParent<T>::RemoveItem(T &rItem)
{
	for(unsigned int i = 0; i < m_ItemList.size(); i++)
	{
		if(&rItem == m_ItemList.at(i))
		{
			m_ItemList.erase(m_ItemList.begin() + i);
		}
	}
}

template <class T>
inline void xmlItemParent<T>::Clean()
{
	for(unsigned int i = 0; i < m_ItemList.size(); i++)
	{
		m_ItemList.at(i)->GetXmlItem().Clean();
	}
}

template <class T>
inline bool xmlItemParent<T>::IsValid() const
{
	for(unsigned int i = 0; i < m_ItemList.size(); i++)
	{
		if(m_ItemList.at(i)->GetXmlItem().IsValid())
			return true;
	}
	return false;
}

template <class T>
inline bool xmlItemParent<T>::IsEmpty() const
{
	return (m_ItemList.size() == 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlFixedSizeStructure implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T, size_t MAXCOUNT>
inline xmlFixedSizeStructure<T, MAXCOUNT>::xmlFixedSizeStructure(int NameId)
	: xmlItem(NameId),
	m_nCount(0)
{
}

template <class T, size_t MAXCOUNT>
inline xmlFixedSizeStructure<T, MAXCOUNT>::~xmlFixedSizeStructure()
{
}

template <class T, size_t MAXCOUNT>
inline bool xmlFixedSizeStructure<T, MAXCOUNT>::Load(XmlNode &rxParent, index_t Index /*= 0*/)
{
	return LoadChildren(rxParent, Index, false);
}

template <class T, size_t MAXCOUNT>
inline bool xmlFixedSizeStructure<T, MAXCOUNT>::LoadChildren(XmlNode &rxParent, index_t Index, bool fUseChildIndex)
{
	XmlNode *pxNode = NULL;
	if(IsEmptyXmlName())
	{
		assert(Index == 0);
		pxNode = &rxParent;
	}
	else
	{
		pxNode = rxParent.GetChild(GetXmlName(), Index);
	}

	if(pxNode == NULL)
		return false;
	bool fSuccess = true;
	for(index_t i = 0; i < (index_t)m_nCount; i++)
	{
		fSuccess &= m_ItemList[i]->GetXmlItem().Load(*pxNode, fUseChildIndex ? i : 0);
	}
	return fSuccess;
}

template <class T, size_t MAXCOUNT>
inline void xmlFixedSizeStructure<T, MAXCOUNT>::Store(XmlNode &rxParent)
{
	if(!IsValid())
		return;

	XmlNode *pxNode = NULL;
	if(IsEmptyXmlName())
	{
		pxNode = &rxParent;
	}
	else
	{
		pxNode = rxParent.AddChild(GetXmlName());
	}

	if(pxNode != NULL)
	{
		for(index_t i = 0; i < (index_t)m_nCount; i++)
		{
			m_ItemList[i]->GetXmlItem().Store(*pxNode);
		}
	}
}

template <class T, size_t MAXCOUNT>
inline bool xmlFixedSizeStructure<T, MAXCOUNT>::Serialize(WritableBinStorage &rBinStorage)
{
	for(index_t i = 0; i < (index_t)m_nCount; i++)
	{
		if(!m_ItemList[i]->GetXmlItem().Serialize(rBinStorage))
			return false;
	}
	return true;
}

template <class T, size_t MAXCOUNT>
inline bool xmlFixedSizeStructure<T, MAXCOUNT>::Deserialize(ReadableBinStorage &rBinStorage)
{
	for(index_t i = 0; i < (index_t)m_nCount; i++)
	{
		if(!m_ItemList[i]->GetXmlItem().Deserialize(rBinStorage))
			return false;
	}
	return true;
}

template <class T, size_t MAXCOUNT>
inline bool xmlFixedSizeStructure<T, MAXCOUNT>::AddItem(T &rItem)
{
	if(m_nCount >= MAXCOUNT)
	{
		assert(0);
		return false;
	}
	m_ItemList[m_nCount] = &rItem;
	m_nCount++;
	return true;
}

template <class T, size_t MAXCOUNT>
inline void xmlFixedSizeStructure<T, MAXCOUNT>::Clean()
{
	m_nCount = 0;
}

template <class T, size_t MAXCOUNT>
inline bool xmlFixedSizeStructure<T, MAXCOUNT>::IsValid() const
{
	for(index_t i = 0; i < (index_t)m_nCount; i++)
	{
		if(m_ItemList[i]->GetXmlItem().IsValid())
			return true;
	}
	return false;
}

template <class T, size_t MAXCOUNT>
inline bool xmlFixedSizeStructure<T, MAXCOUNT>::IsEmpty() const
{
	return (m_nCount == 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlItemArray implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
inline xmlItemArray<T>::xmlItemArray(int ArrayNameId /*= 0*/)
	: xmlItemParent<T>(ArrayNameId),
	m_ItemNameId(-1)
{
}

template <class T>
inline xmlItemArray<T>::~xmlItemArray()
{
	Clean();
}

template <class T>
inline const T *xmlItemArray<T>::GetItemFromIndex(index_t nIndex) const
{
	if(nIndex < 0 || nIndex >= (index_t)m_ItemList.size())
		return NULL;
	return m_ItemList.at(nIndex);
}

template <class T>
inline T *xmlItemArray<T>::GetItemFromIndex(index_t nIndex)
{
	if(nIndex < 0 || nIndex >= (int)m_ItemList.size())
		return NULL;
	return m_ItemList.at(nIndex);
}

template <class T>
inline void xmlItemArray<T>::Store(XmlNode &rxParent)
{
	FileTime CurrentTime;

	if(!IsValid())
		return;

	XmlNode *pxNode = NULL;
	if(IsEmptyXmlName())
	{
		pxNode = &rxParent;
	}
	else
	{
		pxNode = rxParent.AddChild(GetXmlName());
	}

	if(pxNode != NULL)
	{
		for(unsigned int i = 0; i < m_ItemList.size(); i++)
		{
			if(m_ItemList.at(i)->CheckAge(CurrentTime))
			{
				m_ItemList.at(i)->GetXmlItem().Store(*pxNode);
			}
		}
	}
}

template <class T>
inline bool xmlItemArray<T>::Load(XmlNode &rxParent, index_t Index /*= 0*/)
{
	// clean is done somewhere else. Probably by the root object
	XmlNode *pxNode = NULL;
	if(IsEmptyXmlName())
	{
		assert(Index == 0);
		pxNode = &rxParent;
	}
	else
	{
		pxNode = rxParent.GetChild(GetXmlName(), Index);
	}
	if(pxNode == NULL)
		return false;
	// first build the tree
	size_t nItems = pxNode->GetChildCount(GetResourceString(m_ItemNameId));
	if(nItems == 0)
		return true;
	for(index_t i = 0; i < (index_t)nItems; i++)
	{
		XmlNode *pxChild = pxNode->GetChild(GetResourceString(m_ItemNameId), i);
		if(pxChild != NULL)
		{
			T *pNew = CreateFromXml(*pxChild, m_ItemNameId);
			if(pNew)
			{
				AddItem(*pNew);
			}
		}
	}

	// than load the children
	LoadChildren(rxParent, Index, true);
	return true;
}

template <class T>
inline bool xmlItemArray<T>::Serialize(WritableBinStorage &rBinStorage)
{
	FileTime CurrentTime;
	int ItemCount = 0;

	// don't include items, which are out of date
	unsigned int i;
	for(i = 0; i < m_ItemList.size(); i++)
	{
		if(m_ItemList.at(i)->CheckAge(CurrentTime))
		{
			ItemCount++;
		}
	}

	// serialize the count of relevant items
	if(!rBinStorage.Put((LPBYTE)&ItemCount, sizeof(ItemCount)))
		return false;

	// serialize each item
	for(i = 0; i < m_ItemList.size(); i++)
	{
		if(!m_ItemList.at(i)->IsOutOfDate()) // correlates with the previous call of CheckAge
		{
			if(!m_ItemList.at(i)->GetXmlItem().Serialize(rBinStorage))
				return false;
		}
	}
	return true;
}

template <class T>
inline bool xmlItemArray<T>::Deserialize(ReadableBinStorage &rBinStorage)
{
	int Count = 0;
	if(!rBinStorage.Get((LPBYTE)&Count, sizeof(Count)))
		return false;

	m_ItemList.get_allocator().allocate(Count);

	for(int i = 0; i < Count; i++)
	{
		T *pNew = CreateFromBinStorage(rBinStorage, m_ItemNameId);
		if(pNew)
		{
			AddItem(*pNew);
		}
	}
	return true;
}

template <class T>
inline void xmlItemArray<T>::Clean()
{
	for(unsigned int i = 0; i < m_ItemList.size(); i++)
	{
		m_ItemList.at(i)->Release();
	}
	m_ItemList.erase(m_ItemList.begin(), m_ItemList.end());
}

template <class T>
inline bool xmlItemArray<T>::IsValid() const
{
	return m_ItemList.size() > 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xmlFixedSizeMap implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T, size_t MAXCOUNT>
inline xmlFixedSizeMap<T, MAXCOUNT>::xmlFixedSizeMap(int ArrayNameId /*= 0*/)
	: xmlFixedSizeStructure<T, MAXCOUNT>(ArrayNameId),
	m_ItemNameId(-1)
{
	memset(m_xmlItemMap, 0, sizeof(m_xmlItemMap));
}

template <class T, size_t MAXCOUNT>
inline xmlFixedSizeMap<T, MAXCOUNT>::~xmlFixedSizeMap()
{
	Clean();
}

template <class T, size_t MAXCOUNT>
inline const T *xmlFixedSizeMap<T, MAXCOUNT>::GetItemFromIndex(index_t nIndex) const
{
	if(nIndex < 0 || nIndex >= (index_t)m_nCount)
		return NULL;
	return m_ItemList[nIndex];
}

template <class T, size_t MAXCOUNT>
inline T *xmlFixedSizeMap<T, MAXCOUNT>::GetItemFromIndex(index_t nIndex)
{
	if(nIndex < 0 || nIndex >= (index_t)m_nCount)
		return NULL;
	return m_ItemList[nIndex];
}

template <class T, size_t MAXCOUNT>
inline const T *xmlFixedSizeMap<T, MAXCOUNT>::GetItemFromKey(int nKey) const
{
	if(nKey < 0 || nKey >= (int)MAXCOUNT)
		return NULL;
	return m_xmlItemMap[nKey];
}

template <class T, size_t MAXCOUNT>
inline T *xmlFixedSizeMap<T, MAXCOUNT>::GetItemFromKey(int nKey)
{
	if(nKey < 0 || nKey >= MAXCOUNT)
		return NULL;
	return m_xmlItemMap[nKey];
}

template <class T, size_t MAXCOUNT>
inline void xmlFixedSizeMap<T, MAXCOUNT>::Store(XmlNode &rxParent)
{
	FileTime CurrentTime;

	if(!IsValid())
		return;

	XmlNode *pxNode = NULL;
	if(IsEmptyXmlName())
	{
		pxNode = &rxParent;
	}
	else
	{
		pxNode = rxParent.AddChild(GetXmlName());
	}

	if(pxNode != NULL)
	{
		for(index_t i = 0; i < (index_t)m_nCount; i++)
		{
			if(m_ItemList[i]->CheckAge(CurrentTime))
			{
				m_ItemList[i]->GetXmlItem().Store(*pxNode);
			}
		}
	}
}

template <class T, size_t MAXCOUNT>
inline bool xmlFixedSizeMap<T, MAXCOUNT>::Load(XmlNode &rxParent, index_t Index /*= 0*/)
{
	// clean is done somewhere else. Probably by the root object
	XmlNode *pxNode = NULL;
	if(IsEmptyXmlName())
	{
		assert(Index == 0);
		pxNode = &rxParent;
	}
	else
	{
		pxNode = rxParent.GetChild(GetXmlName(), Index);
	}
	if(pxNode == NULL)
		return false;
	// first build the tree
	size_t nItems = pxNode->GetChildCount(GetResourceString(m_ItemNameId));
	if(nItems == 0)
		return true;
	if(nItems > MAXCOUNT)
	{
		assert(0);
		return false;
	}
	for(index_t i = 0; i < (index_t)nItems; i++)
	{
		XmlNode *pxChild = pxNode->GetChild(GetResourceString(m_ItemNameId), i);
		if(pxChild != NULL)
		{
			int nKey;
			T *pNew = CreateFromXml(*pxChild, m_ItemNameId, nKey);
			if(pNew)
			{
				AddTItem(pNew, nKey);
			}
		}
	}

	// than load the children
	LoadChildren(rxParent, Index, true);
	return true;
}

template <class T, size_t MAXCOUNT>
inline bool xmlFixedSizeMap<T, MAXCOUNT>::Serialize(WritableBinStorage &rBinStorage)
{
	FileTime CurrentTime;
	int ItemCount = 0;

	// don't include items, which are out of date
	index_t i;
	for(i = 0; i < (index_t)m_nCount; i++)
	{
		if(m_ItemList[i]->CheckAge(CurrentTime))
		{
			ItemCount++;
		}
	}

	// serialize the count of relevant items
	if(!rBinStorage.Put((LPBYTE)&ItemCount, sizeof(ItemCount)))
		return false;

	// serialize each item
	for(index_t i = 0; i < (index_t)m_nCount; i++)
	{
		if(!m_ItemList[i]->IsOutOfDate()) // correlates with the previous call of CheckAge
		{
			if(!m_ItemList[i]->GetXmlItem().Serialize(rBinStorage))
				return false;
		}
	}
	return true;
}

template <class T, size_t MAXCOUNT>
inline bool xmlFixedSizeMap<T, MAXCOUNT>::Deserialize(ReadableBinStorage &rBinStorage)
{
	int Count = 0;
	if(!rBinStorage.Get((LPBYTE)&Count, sizeof(Count)))
		return false;

	if(Count > MAXCOUNT)
		return false;

	for(int i = 0; i < Count; i++)
	{
		int nKey;
		T *pNew = CreateFromBinStorage(rBinStorage, m_ItemNameId, nKey);
		if(pNew)
		{
			AddTItem(pNew, nKey);
		}
	}
	return true;
}

template <class T, size_t MAXCOUNT>
inline void xmlFixedSizeMap<T, MAXCOUNT>::Clean()
{
	for(int i = 0; i < MAXCOUNT; i++)
	{
		if(m_xmlItemMap[i] != NULL)
		{
			m_xmlItemMap[i]->Release();
		}
	}

	memset(m_xmlItemMap, 0, sizeof(m_xmlItemMap));
	m_nCount = 0;
}

template <class T, size_t MAXCOUNT>
inline bool xmlFixedSizeMap<T, MAXCOUNT>::IsValid() const
{
	return m_nCount > 0;
}

template <class T, size_t MAXCOUNT>
bool xmlFixedSizeMap<T, MAXCOUNT>::AddTItem(T *pItem, int nKey)
{
	if(!pItem)
		return false;
	if(nKey < 0 || nKey >= MAXCOUNT)
		return false;
	AddItem(*pItem);
	m_xmlItemMap[nKey] = pItem;
	return true;
}


#endif // _XML_STRUCTURE_HEADER_