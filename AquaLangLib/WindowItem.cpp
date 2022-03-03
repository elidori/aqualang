#include "Pch.h"
#include "WindowItem.h"
#include "WindowItemProperty.h"
#include "Log.h"
#include "resource.h"
#include "StringTable.h"

#pragma warning (disable:4355)

#define WINDOW_ITEM_ALLOCATION_CHUNK 1000

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// WindowItemListXml
/////////////////////////////////////////////////////////////////////////////////////////////////////////
WindowItemListXml::WindowItemListXml(WindowItem &rOwner)
	: xmlItemArray<WindowItem>(IDS_STRING_WINDOWITEMLIST),
	m_rOwner(rOwner)
{
}

WindowItemListXml::~WindowItemListXml()
{
}

WindowItem *WindowItemListXml::CreateFromXml(
					XmlNode &rxItemNode,
					int NameId
					)
{
	return m_rOwner.CreateFromXml(rxItemNode, NameId);
}

WindowItem *WindowItemListXml::CreateFromBinStorage(
						ReadableBinStorage &rBinStorage,
						int NameId
						)
{
	return m_rOwner.CreateFromBinStorage(rBinStorage, NameId);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// WindowItem
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectPool<WindowItem> WindowItem::m_WindowItemPool(WINDOW_ITEM_ALLOCATION_CHUNK);

WindowItem::WindowItem()
	: m_pWrapper(NULL),
	m_PropertyList(IDS_STRING_PROPERTYLIST),
	m_SubWindowsList(*this),
	m_WindowItemXmlStructure(IDS_STRING_ITEM),
	m_LanguageXmlValue(IDS_STRING_LANGUAGE),

	m_XmlNameId(IDS_STRING_ITEM),
	m_Depth(-1),
	m_pParent(NULL),

	m_fMarked(false),
	m_MarkedTime(),

	m_fOutOfDate(false)
{
	m_SubWindowsList.SetItemNameId(IDS_STRING_ITEM);

	m_WindowItemXmlStructure.AddItem(m_PropertyList);
	m_WindowItemXmlStructure.AddItem(m_LanguageXmlValue);
	m_WindowItemXmlStructure.AddItem(m_SubWindowsList);
}

WindowItem::~WindowItem()
{
	Close();
}

void WindowItem::Construct(
			int Depth,
			const WindowItem *pParent,
			int NameId,
			ObjectWrapper<WindowItem> *pWrapper
			)
{
	m_WindowItemXmlStructure.SetNameId(NameId);
	m_SubWindowsList.SetItemNameId(NameId);
	m_XmlNameId = NameId;
	m_Depth = Depth;
	m_pParent = pParent;
	m_pWrapper = pWrapper;
}

WindowItem *WindowItem::Create(
			int Depth,
			const WindowItem *pParent,
			int NameId
			)
{
	ObjectWrapper<WindowItem> *pNewWrapper = m_WindowItemPool.Get();
	if(pNewWrapper == NULL)
		return NULL;

	WindowItem &rNew = pNewWrapper->Info();
	rNew.Construct(Depth, pParent, NameId, pNewWrapper);
	return &rNew;
}

void WindowItem::SetLanguageTable(const LanguageStringTable &rLanguageStringTable)
{
	xmlLanguageValue::SetLanguageTable(rLanguageStringTable);
}

bool WindowItem::Initialize(HWND hwnd)
{
	return m_PropertyList.Initialize(hwnd);
}

bool WindowItem::Initialize(LPCTSTR SubObjectId)
{
	return m_PropertyList.Initialize(SubObjectId);
}

void WindowItem::SetLanguage(HKL hKL)
{
	m_LanguageXmlValue.SetInt((int)hKL);
}

void WindowItem::GetLanguage(HKL &rhKL) const
{
	rhKL = (HKL)m_LanguageXmlValue.GetInt();
}

void WindowItem::Release()
{
	if(m_pWrapper != NULL)
	{
		Close();
		m_pWrapper->Release();
	}
	else
	{
		delete this;
	}
}

WindowItem *WindowItem::CreateFromXml(
					XmlNode & UNUSED(rxItemNode),
					int NameId
					)
{
	WindowItem *pNew = WindowItem::Create(m_Depth + 1, this, NameId);
	if(!pNew)
		return NULL;
	return pNew;
}

WindowItem *WindowItem::CreateFromBinStorage(
						ReadableBinStorage &rBinStorage,
						int NameId
						)
{
	WindowItem *pNew = WindowItem::Create(m_Depth + 1, this, NameId);
	if(!pNew)
		return NULL;

	if(!pNew->GetXmlItem().Deserialize(rBinStorage))
	{
		pNew->Release();
		return NULL;
	}

	return pNew;
}

WindowItem *WindowItem::GetWindowItem(const WindowItem &rItem, bool &rfCreated, bool fEmptyListMatches)
{
	rfCreated = false;
	
	// see if a child is matching
	for(index_t i = 0; i < (index_t)m_SubWindowsList.GetCount(); i++)
	{
		if(m_SubWindowsList.GetItemFromIndex(i)->CheckMatching(rItem, fEmptyListMatches))
			return m_SubWindowsList.GetItemFromIndex(i);
	}

	// creating a new child
	WindowItem *pChildItem = WindowItem::Create(m_Depth + 1, this, m_XmlNameId);
	if(!pChildItem)
		return NULL;
	pChildItem->m_PropertyList.Update(rItem.m_PropertyList);

	rfCreated = true;
	if(pChildItem != NULL)
	{
		m_SubWindowsList.AddItem(*pChildItem);
	}
	return pChildItem;
}

const WindowItem *WindowItem::GetMatchingItem(const WindowItem &rItem, bool fEmptyListMatches) const
{
	if(m_Depth > rItem.m_Depth - 1)
		return NULL;

	if(m_Depth == rItem.m_Depth - 1)
	{
		for(index_t i = 0; i < (index_t)m_SubWindowsList.GetCount(); i++)
		{
			if(m_SubWindowsList.GetItemFromIndex(i)->m_PropertyList.IsMatching(rItem.m_PropertyList, fEmptyListMatches))
				return m_SubWindowsList.GetItemFromIndex(i);
		}
	}
	else
	{
		if(rItem.m_pParent != NULL)
		{
			const WindowItem *pItem = GetMatchingItem(*rItem.m_pParent, fEmptyListMatches); // Recursive call !
			if(pItem)
			{
				return pItem->GetMatchingItem(rItem, fEmptyListMatches);
			}
		}
	}
	return NULL;

}

const xmlValue *WindowItem::GetPropertyValue(WindowItemPropertyType_t Type) const
{
	return m_PropertyList.GetPropertyValue(Type);
}

void WindowItem::GetDebugString(_tstring &rString) const
{
	m_PropertyList.GetDebugString(rString);
}

int WindowItem::GetDebugFullString(_tstring &rString) const
{
	int nParents = 0;
	if(m_pParent)
	{
		nParents = m_pParent->GetDebugFullString(rString);
		rString += _T("\n");
	}
	if(!m_PropertyList.IsEmpty())
	{
		TCHAR IndexStr[20];
		_stprintf_s(IndexStr, sizeof(IndexStr) / sizeof(IndexStr[0]), _T("%d. "), nParents);
		rString += IndexStr;
		GetDebugString(rString);
	}
	return nParents + 1;
}

bool WindowItem::GetLastMark(__int64 &SecondsFromLastMark) const
{
	SecondsFromLastMark = 0;
	
	if(!m_fMarked)
		return false;

	SecondsFromLastMark = m_MarkedTime.GetDiff() / 1000;
	return true;
}

void WindowItem::MarkNow()
{
	m_fMarked = true;
	m_MarkedTime.Restart();
}

bool WindowItem::CheckMatching(const WindowItem &rRef, bool fEmptyListMatches)
{
	if(!m_PropertyList.IsMatching(rRef.m_PropertyList, fEmptyListMatches))
		return false;
	m_PropertyList.Update(rRef.m_PropertyList);
	return true;
}

bool WindowItem::CheckAge(const FileTime &rCurrentTime)
{
	m_fOutOfDate = m_PropertyList.IsOutOfDate(rCurrentTime);
	return !m_fOutOfDate;
}

const WindowItem *WindowItem::QueryWindowItem(const WindowItem &rItem, bool fEmptyListMatches) const
{
	// see if a child is matching
	for(index_t i = 0; i < (index_t)m_SubWindowsList.GetCount(); i++)
	{
		if(m_SubWindowsList.GetItemFromIndex(i)->m_PropertyList.IsMatching(rItem.m_PropertyList, fEmptyListMatches))
			return m_SubWindowsList.GetItemFromIndex(i);
	}
	return NULL;
}

void WindowItem::Close()
{
	m_XmlNameId = IDS_STRING_ITEM;
	m_Depth = -1;
	m_pParent = NULL;
	m_PropertyList.Clean();
	m_LanguageXmlValue.Clean();
	m_SubWindowsList.Clean();
	m_SubWindowsList.SetItemNameId(IDS_STRING_ITEM);
//	m_WindowItemXmlStructure.Clean();
	m_fMarked = false;
	m_fOutOfDate = false;
}
