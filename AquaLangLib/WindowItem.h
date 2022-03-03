#ifndef _WINDOW_ITEM_HEADER_
#define _WINDOW_ITEM_HEADER_

#include "xmlStructure.h"
#include "xmlValueTypes.h"
#include "WindowItemPropertyList.h"
#include "TimeHelpers.h"
#include "ObjectPool.h"

#define WINDOW_ITEM_SUB_ITEMS 3 // m_PropertyList, m_LanguageXmlValue, m_SubWindowsList

class WindowItem;
class LanguageStringTable;

class WindowItemListXml : public xmlItemArray<WindowItem>
{
public:
	WindowItemListXml(WindowItem &rOwner);
	virtual ~WindowItemListXml();

	virtual WindowItem *CreateFromXml(
					XmlNode &rxItemNode,
					int NameId
					);
	virtual WindowItem *CreateFromBinStorage(
						ReadableBinStorage &rBinStorage,
						int NameId
						);
private:
	WindowItem &m_rOwner;
};

class WindowItem
{
public:
	// language
	void SetLanguage(HKL hKL);
	void GetLanguage(HKL &rhKL) const;
	// properties
	const xmlValue *GetPropertyValue(WindowItemPropertyType_t Type) const;

	void GetDebugString(_tstring &rString) const;
	int GetDebugFullString(_tstring &rString) const; // returns the number of parent windows
	xmlItem &GetChilrenListXmlItem() { return m_SubWindowsList; }

	bool GetLastMark(__int64 &SecondsFromLastMark) const;
	void MarkNow();

	void Release();

	virtual xmlItem &GetXmlItem()	{ return m_WindowItemXmlStructure; }
	bool CheckAge(const FileTime &rCurrentTime);
	bool IsOutOfDate() const		{ return m_fOutOfDate; }

	const WindowItem *QueryWindowItem(const WindowItem &rItem, bool fEmptyListMatches) const;
private:
	friend class WindowItemDataBase;
	friend class ObjectWrapper<WindowItem>;
	friend class ObjectPool<WindowItem>;

	WindowItem();
	virtual ~WindowItem();

	void Construct(
			int Depth,
			const WindowItem *pParent,
			int NameId,
			ObjectWrapper<WindowItem> *pWrapper
			);

	static WindowItem *Create(
			int Depth,
			const WindowItem *pParent,
			int NameId
			);

	static void SetLanguageTable(const LanguageStringTable &rLanguageStringTable);

	bool Initialize(HWND hwnd);
	bool Initialize(LPCTSTR SubObjectId);

	WindowItem *GetWindowItem(const WindowItem &rItem, bool &rfCreated, bool fEmptyListMatches);
	const WindowItem *GetMatchingItem(const WindowItem &rItem, bool fEmptyListMatches) const;

	friend class WindowItemListXml;
	WindowItem *CreateFromXml(
					XmlNode &rxItemNode,
					int NameId
					);
	WindowItem *CreateFromBinStorage(
					ReadableBinStorage &rBinStorage,
					int NameId
					);
private:
	bool CheckMatching(const WindowItem &rRef, bool fEmptyListMatches);
	void Close();
private:
	static ObjectPool<WindowItem> m_WindowItemPool;
	ObjectWrapper<WindowItem> *m_pWrapper;

	int m_XmlNameId;
	int m_Depth;
	const WindowItem *m_pParent;

	WindowItemPropertyList m_PropertyList;
	xmlLanguageValue m_LanguageXmlValue;
	WindowItemListXml m_SubWindowsList;
	xmlFixedSizeStructure<xmlItem, WINDOW_ITEM_SUB_ITEMS> m_WindowItemXmlStructure;

	bool m_fMarked;
	Timer m_MarkedTime;

	bool m_fOutOfDate;
};

#endif // _WINDOW_ITEM_HEADER_