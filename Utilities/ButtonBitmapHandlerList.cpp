#include "Pch.h"
#include "ButtonBitmapHandler.h"
#include "ButtonBitmapHandlerList.h"
#include "Log.h"

ButtonBitmapHandlerList::ButtonBitmapHandlerList()
	: m_hDialog(NULL),
	m_hHighLightedControl(NULL)
{
}

ButtonBitmapHandlerList::~ButtonBitmapHandlerList()
{
	Close();
}

bool ButtonBitmapHandlerList::Add(
								int ControlId,
								LPCTSTR ResourceName,
								LPCTSTR Text /*= _T("")*/,
								COLORREF NormalText /*= RGB(0, 0, 0)*/,
								COLORREF PressedText /*= RGB(0, 0, 0)*/,
								COLORREF DisabledText /*= RGB(0, 0, 0)*/,
								COLORREF HighlightedText /*= RGB(0, 0, 0)*/,
								bool fUseManualState /*= false*/)
{
	HWND hControlWnd = ::GetDlgItem(m_hDialog, ControlId);
	if(hControlWnd == NULL)
		return false;
	ButtonBitmapHandler *pNew = new ButtonBitmapHandler(ResourceName, hControlWnd, fUseManualState);
	if(!pNew)
		return false;
	if(Text != NULL && _tcslen(Text) > 0)
	{
		pNew->SetText(Text, NormalText, PressedText, DisabledText, HighlightedText);
	}

	m_BitmapObjectIdMap.insert(BitmapObjectMap::value_type(ControlId, pNew));
	m_BitmapObjectHWNDMap.insert(BitmapObjectMap::value_type((int)hControlWnd, pNew));
	return true;
}

bool ButtonBitmapHandlerList::Check(int ControlId, bool fCheck)
{
	BitmapObjectMap::iterator Iterator = m_BitmapObjectIdMap.find(ControlId);
	if(Iterator == m_BitmapObjectIdMap.end())
		return false;
	(*Iterator).second->Check(fCheck);
	return true;
}

bool ButtonBitmapHandlerList::GetChecked(int ControlId) const
{
	BitmapObjectMap::const_iterator Iterator = m_BitmapObjectIdMap.find(ControlId);
	if(Iterator == m_BitmapObjectIdMap.end())
		return false;
	return (*Iterator).second->GetChecked();
}

bool ButtonBitmapHandlerList::Highlight(HWND hwnd)
{
	if(hwnd == m_hHighLightedControl)
		return true;

	// cancel current highlight
	BitmapObjectMap::iterator Iterator = m_BitmapObjectHWNDMap.find((int)m_hHighLightedControl);
	if(Iterator != m_BitmapObjectHWNDMap.end())
	{
		(*Iterator).second->HighLight(false);
	}
	m_hHighLightedControl = NULL;

	// update new highlight
	Iterator = m_BitmapObjectHWNDMap.find((int)hwnd);
	if(Iterator == m_BitmapObjectHWNDMap.end())
		return false;

	(*Iterator).second->HighLight(true);
	m_hHighLightedControl = hwnd;

	return true;
}

bool ButtonBitmapHandlerList::Draw(int ControlId, DRAWITEMSTRUCT *pDrawItemStruct)
{
	BitmapObjectMap::iterator Iterator = m_BitmapObjectIdMap.find(ControlId);
	if(Iterator != m_BitmapObjectIdMap.end() && pDrawItemStruct != NULL)
	{
		if((*Iterator).second->Draw(*pDrawItemStruct))
			return true;
	}
	return false;
}

void ButtonBitmapHandlerList::Close()
{
	BitmapObjectMap::iterator Iterator = m_BitmapObjectIdMap.begin();
	for(; Iterator != m_BitmapObjectIdMap.end(); Iterator++)
	{
		delete (*Iterator).second;
	}
	m_BitmapObjectIdMap.erase(m_BitmapObjectIdMap.begin(), m_BitmapObjectIdMap.end());

	m_BitmapObjectHWNDMap.erase(m_BitmapObjectHWNDMap.begin(), m_BitmapObjectHWNDMap.end());
}
