#include "Pch.h"
#include "WindowItemDataBase.h"
#include "WindowItemProperty.h"
#include "ApplicationDataPath.h"
#include "LanguageStringTable.h"
#include "resource.h"

extern HMODULE g_hModule;

#define DATABASEDEFAULT _T("winhook.dbs")
#define RULESDEFAULT _T("rules.dbs")

#define SERIALIZE_DATABASE_INFO true

WindowItemDataBase::WindowItemDataBase(const LanguageStringTable &rLanguageStringTable)
	: m_RootItem(),
	m_RulesItem(),
	m_fOpen(false),
	m_fOpenRulesOnly(false),
	m_rLanguageStringTable(rLanguageStringTable),
	m_fUseSerialization(SERIALIZE_DATABASE_INFO)
{
	WindowItem::SetLanguageTable(rLanguageStringTable);
}

WindowItemDataBase::~WindowItemDataBase()
{
}

bool WindowItemDataBase::Open(LPCTSTR UserName)
{
	Close();

	_tstring DataBaseURL;
	ApplicationDataPath PathFinder(UserName);
	
	PathFinder.ComposePath(DATABASEDEFAULT, DataBaseURL);
	m_xmlTree.Initialize(m_RootItem.GetChilrenListXmlItem(), DataBaseURL.c_str(), IDS_STRING_DATABASEROOTFIELD);

	if(IsSerialized(DataBaseURL.c_str()))
	{
		m_xmlTree.Deserialize();
	}
	else
	{
		m_xmlTree.LoadXml();
	}

	PathFinder.ComposeRelativePath(RULESDEFAULT, DataBaseURL);
	m_xmlRulesTree.Initialize(m_RulesItem.GetChilrenListXmlItem(), DataBaseURL.c_str(), IDS_STRING_RULESROOTFIELD);

	m_xmlRulesTree.LoadXml();

	m_fOpen = true;
	m_fOpenRulesOnly = false;
	return true;
}

bool WindowItemDataBase::OpenRulesOnly(LPCTSTR UserName)
{
	Close();

	_tstring DataBaseURL;
	ApplicationDataPath PathFinder(UserName);
	
	PathFinder.ComposeRelativePath(RULESDEFAULT, DataBaseURL);
	m_xmlRulesTree.Initialize(m_RulesItem.GetChilrenListXmlItem(), DataBaseURL.c_str(), IDS_STRING_RULESROOTFIELD);

	m_xmlRulesTree.LoadXml();

	m_fOpen = true;
	m_fOpenRulesOnly = true;
	return true;
}


void WindowItemDataBase::Close()
{
	if(m_fOpen)
	{
		m_fOpen = false;
		if(!m_fOpenRulesOnly)
		{
			if(m_fUseSerialization)
			{
				m_xmlTree.Serialize();
			}
			else
			{
				m_xmlTree.StoreXml(ENCODING_TYPE_UTF_16);
			}
		}
	}
	m_xmlTree.Clean();
	m_xmlRulesTree.Clean();
}

WindowItem *WindowItemDataBase::GetWindowItem(HWND hwnd, LPCTSTR SubObjectId, bool &rfCreated, HKL *pDefaultLanguage, const WindowItem* &rpWindowRule)
{
	if(pDefaultLanguage)
	{
		*pDefaultLanguage = NULL;
	}
	rpWindowRule = NULL;

	// first find/create window item according to the hwnd
	WindowItem *pRequestedItem = FindWindowItem(hwnd, rfCreated, false);
	if(!pRequestedItem)
		return NULL;

	// then refer to SubObjectId if exists
	if(SubObjectId != NULL && _tcslen(SubObjectId) > 0)
	{
		WindowItem SubItem;
		if(!SubItem.Initialize(SubObjectId))
			return NULL;
		pRequestedItem = pRequestedItem->GetWindowItem(SubItem, rfCreated, false);
		if(!pRequestedItem)
			return NULL;
	}

	const WindowItem *pRefItem = m_RulesItem.GetMatchingItem(*pRequestedItem, true);
	if(pRefItem)
	{
		rpWindowRule = pRefItem;
		// after creation, try to find default langauage
		if(pDefaultLanguage)
		{
			pRefItem->GetLanguage(*pDefaultLanguage);
		}
	}

	return pRequestedItem;
}

bool WindowItemDataBase::GetLanguageString(HKL hKL, _tstring &rLanguageId, _tstring &rLanguageSpecific) const
{
	return m_rLanguageStringTable.GetLanguageString(hKL, rLanguageId, rLanguageSpecific);
}

const WindowItem *WindowItemDataBase::QueryWindowItem(HWND hwnd, const WindowItem** ppWindowRule) const
{
	if(ppWindowRule != NULL)
	{
		*ppWindowRule = NULL;
	}

	if(!::IsWindow(hwnd))
		return NULL;

	if(m_fOpenRulesOnly)
		return NULL;

	// create temp object
	WindowItem Item;
	if(!Item.Initialize(hwnd))
		return NULL;

	HWND hParentWindow = ::GetParent(hwnd);
	const WindowItem *pParentItem = NULL;

	// find in tree, and create if needed
	if(::IsWindow(hParentWindow))
	{
		// recursive call!
		pParentItem = QueryWindowItem(hParentWindow, NULL);
	}
	else
	{
		pParentItem = &m_RootItem;
	}

	if(!pParentItem)
		return NULL;

	const WindowItem *pRequestedItem = pParentItem->QueryWindowItem(Item, false);
	if(!pRequestedItem)
		return NULL;

	if(ppWindowRule != NULL)
	{
		*ppWindowRule = m_RulesItem.GetMatchingItem(*pRequestedItem, true);
	}

	return pRequestedItem;
}

WindowItem *WindowItemDataBase::FindWindowItem(HWND hwnd, bool &rfCreated, bool fEmptyListMatches)
{
	rfCreated = false;
	if(!::IsWindow(hwnd))
		return NULL;

	// create temp object
	WindowItem Item;
	if(!Item.Initialize(hwnd))
		return NULL;

	HWND hParentWindow = ::GetParent(hwnd);
	WindowItem *pParentItem = NULL;

	// find in tree, and create if needed
	if(::IsWindow(hParentWindow))
	{
		// recursive call!
		bool fCreated;
		pParentItem = FindWindowItem(hParentWindow, fCreated, fEmptyListMatches);
	}
	else
	{
		pParentItem = &m_RootItem;
	}
	if(!pParentItem)
		return NULL;

	WindowItem *pRequestedItem = pParentItem->GetWindowItem(Item, rfCreated, fEmptyListMatches);
	if(!pRequestedItem)
		return NULL;

	return pRequestedItem;
}

bool WindowItemDataBase::IsSerialized(LPCTSTR FileName) const
{
	return ReadableBinStorage::CheckFile(FileName);
}
