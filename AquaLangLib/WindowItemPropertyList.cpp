#include "Pch.h"
#include "WindowItemPropertyList.h"
#include "WindowItemProperty.h"
#include "resource.h"

WindowItemPropertyList::WindowItemPropertyList(int xmlNameId)
	: PropertyListXml(xmlNameId)
{
	SetItemNameId(IDS_STRING_PROPERTY);
}

WindowItemPropertyList::~WindowItemPropertyList()
{
	Clean();
}

const xmlValue *WindowItemPropertyList::GetPropertyValue(WindowItemPropertyType_t Type) const
{
	const WindowItemProperty *pProperty = GetItemFromKey(Type);
	if(!pProperty)
		return NULL;
	return &pProperty->GetValueNode();
}

bool WindowItemPropertyList::Initialize(HWND hwnd)
{
	if(!::IsWindow(hwnd))
		return false;

	int TypeIndex = 0;
	int nKey;
	WindowItemProperty *pNewProperty;

	while(WindowItemProperty::AddPropertyFromHWND(hwnd, TypeIndex, IDS_STRING_PROPERTY, nKey, pNewProperty))
	{
		if(pNewProperty)
		{
			AddTItem(pNewProperty, nKey);
		}
		TypeIndex++;
	}
	if(IsEmpty())
		return false;
	return true;
}

bool WindowItemPropertyList::Initialize(LPCTSTR SubObjectId)
{
	if(SubObjectId == NULL || _tcslen(SubObjectId) == 0)
		return false;

	int nKey;
	WindowItemProperty *pNewProperty;

	if(WindowItemProperty::AddPropertyFromId(SubObjectId, IDS_STRING_PROPERTY, nKey, pNewProperty))
	{
		if(pNewProperty)
		{
			AddTItem(pNewProperty, nKey);
		}
	}
	if(IsEmpty())
		return false;
	return true;
}

bool WindowItemPropertyList::Update(const WindowItemPropertyList &rRef)
{
	if(&rRef == this)
		return true;

	for(index_t i = 0; i < (index_t)rRef.GetCount(); i++)
	{
		const WindowItemProperty *pRefProperty = rRef.GetItemFromIndex(i);
		assert(pRefProperty);

		WindowItemProperty *pMyProperty = GetItemFromKey(pRefProperty->GetType());
		if(!pMyProperty)
		{
			WindowItemProperty *pNew = pRefProperty->Duplicate();
			if(!pNew)
				return false;
			AddTItem(pNew, pRefProperty->GetType());
		}
		else
		{
			pMyProperty->Update(*pRefProperty);
		}
	}
	return true;
}

bool WindowItemPropertyList::IsMatching(const WindowItemPropertyList &rRef, bool fEmptyListMatches) const
{
	if(IsEmpty() || rRef.IsEmpty())
		return fEmptyListMatches;

	bool fMatching = false;

	for(index_t i = 0; i < (index_t)GetCount(); i++)
	{
		const WindowItemProperty *pMyProperty = GetItemFromIndex(i);
		assert(pMyProperty);

		const WindowItemProperty *pRefProperty = rRef.GetItemFromKey(pMyProperty->GetType());
		if(pRefProperty)
		{
			if(pMyProperty->IsMatching(*pRefProperty))
			{
				fMatching = true;
			}
			else
			{
				if(pMyProperty->IsMatchingMust())
					return false;
			}
		}
	}
	return fMatching;
}

void WindowItemPropertyList::GetDebugString(_tstring &rString) const
{
	for(index_t i = 0; i < (index_t)GetCount(); i++)
	{
		const WindowItemProperty *pProperty = GetItemFromIndex(i);
		pProperty->GetDebugString(rString);
	}
}

bool WindowItemPropertyList::IsOutOfDate(const FileTime &rCurrentTime) const
{
	const WindowItemProperty *pTimeProperty = GetItemFromKey(PropertyTypeUsageTime);
	if(pTimeProperty == NULL)
		return false;
	
	return ((const WindowItemUsageTimeProperty *)pTimeProperty)->CheckTimeTooOld(rCurrentTime);
}

WindowItemProperty *WindowItemPropertyList::CreateFromXml(
									XmlNode &rxItemNode,
									int NameId,
									int &rnKey
									)
{
	return WindowItemProperty::CreateFromXml(rxItemNode, NameId, rnKey);
}

WindowItemProperty *WindowItemPropertyList::CreateFromBinStorage(
									ReadableBinStorage &rBinStorage,
									int NameId,
									int &rnKey
									)
{
	return WindowItemProperty::CreateFromBinStorage(rBinStorage, NameId, rnKey);
}
