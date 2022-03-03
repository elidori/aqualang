#include "Pch.h"
#include "WindowItemPropertyTypes.h"
#include "WindowItemProperty.h"
#include "xmlStructure.h"
#include "resource.h"

WindowItemPropertyTypes::WindowItemPropertyTypes()
{
	// initialization of the property types used
	AddType(PropertyTypeHWND, _T("HWND"), WindowItemHWNDProperty::Create);
	AddType(PropertyTypeCaption, _T("caption"), WindowItemCaptionProperty::Create);
	AddType(PropertyTypeIndexInParent, _T("IndexInParent"), WindowItemIndexInParentProperty::Create);
	AddType(PropertyTypeProcessName, _T("ProcessName"), WindowItemProcessNameProperty::Create);
	AddType(PropertyTypeUsageTime, _T("UsageTime"), WindowItemUsageTimeProperty::Create);
	AddType(PropertyTypeRect, _T("Rect"), WindowItemRectProperty::Create);

	// in addition there are read only properties for the rules data base
	AddType(RuleTypeIsEditable, _T("IsEditable"), WindowItemIsEditableProperty::Create, true);
	AddType(RuleTypeClipboardEnabled, _T("ClipboardEnabled"), WindowItemClipboardEnabledProperty::Create, true);
}

WindowItemPropertyTypes::~WindowItemPropertyTypes()
{
	Close();
}

void WindowItemPropertyTypes::AddType(int Type, LPCTSTR TypeString, T_WindowItemPropertyCreator Creator, bool fIsReadOnly /*= false*/)
{
	CreatorInfo Info;
	Info.Function = Creator;
	Info.Type = Type;
	Info.TypeString = TypeString;
	Info.fIsReadOnly = fIsReadOnly;

	m_PropertyCreatorMap.insert(PropertyCreatorMap::value_type(Type, Info));
	m_PropertyStringCreatorMap.insert(PropertyStringCreatorMap::value_type(TypeString, Info));
	if(!fIsReadOnly)
	{
		m_PropertyTypeList.push_back(Type);
	}

	xmlPropertyTypeValue *pNewXmlString = new xmlPropertyTypeValue(IDS_STRING_TYPE, *this);
	if(pNewXmlString != NULL)
	{
		pNewXmlString->SetString(TypeString);
		m_xmlStringMap.insert(xmlStringMap::value_type(Type, pNewXmlString));
	}
}

bool WindowItemPropertyTypes::Find(int Type, _tstring &TypeString) const
{
	PropertyCreatorMap::const_iterator Iterator = m_PropertyCreatorMap.find(Type);
	if(Iterator == m_PropertyCreatorMap.end())
		return false;

	TypeString = (*Iterator).second.TypeString;
	return true;
}

bool WindowItemPropertyTypes::Find(int Type, T_WindowItemPropertyCreator &Creator, bool &rfIsReadOnly) const
{
	Creator = NULL;
	rfIsReadOnly = false;

	PropertyCreatorMap::const_iterator Iterator = m_PropertyCreatorMap.find(Type);
	if(Iterator == m_PropertyCreatorMap.end())
		return false;

	Creator = (*Iterator).second.Function;
	rfIsReadOnly = (*Iterator).second.fIsReadOnly;
	return true;
}

bool WindowItemPropertyTypes::Find(LPCTSTR TypeString, int &Type, T_WindowItemPropertyCreator &Creator, bool &rfIsReadOnly) const
{
	Type = -1;
	Creator = NULL;
	rfIsReadOnly = false;

	PropertyStringCreatorMap::const_iterator Iterator = m_PropertyStringCreatorMap.find(TypeString);
	if(Iterator == m_PropertyStringCreatorMap.end())
		return false;

	Type = (*Iterator).second.Type;
	Creator = (*Iterator).second.Function;
	rfIsReadOnly = (*Iterator).second.fIsReadOnly;
	return true;
}

xmlPropertyTypeValue *WindowItemPropertyTypes::FindXmlString(int Type) const
{
	xmlStringMap::const_iterator Iterator = m_xmlStringMap.find(Type);
	if(Iterator == m_xmlStringMap.end())
		return NULL;
	return (*Iterator).second;
}

size_t WindowItemPropertyTypes::GetCount() const
{
	return m_PropertyTypeList.size();
}

bool WindowItemPropertyTypes::GetType(int Index, int &Type, T_WindowItemPropertyCreator &Creator) const
{
	if(Index < 0 || Index >= (int)m_PropertyTypeList.size())
		return false;

	Type = m_PropertyTypeList.at(Index);
	bool fIsReadOnly = false;
	if(!Find(Type, Creator, fIsReadOnly))
		return false;
	assert(!fIsReadOnly);
	return true;
}

void WindowItemPropertyTypes::Close()
{
	xmlStringMap::iterator Iterator = m_xmlStringMap.begin();
	for(; Iterator != m_xmlStringMap.end(); Iterator++)
	{
		delete (*Iterator).second;
	}
	m_xmlStringMap.erase(m_xmlStringMap.begin(), m_xmlStringMap.end());
}