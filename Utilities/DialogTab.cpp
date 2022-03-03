#include "Pch.h"
#include "DialogTab.h"
#include "DialogObject.h"

DialogTab::DialogTab()
	: m_pActivePage(NULL),
	m_hTab(NULL)
{
}

DialogTab::~DialogTab()
{
	Close();
}

void DialogTab::Open(HWND hTab)
{
	m_hTab = hTab;
}

bool DialogTab::AddPage(DialogObject *pPage, LPCTSTR Name)
{
	if(m_hTab == NULL)
		return false;
	if(!pPage)
		return false;

	LRESULT nCount = ::SendMessage(m_hTab, TCM_GETITEMCOUNT, 0, 0);

	TCITEM TcItem;
	TcItem.mask = TCIF_TEXT;
	TcItem.pszText = (LPTSTR)Name;
	TcItem.cchTextMax = 0; 

	::SendMessage(m_hTab, TCM_INSERTITEM, nCount, (LPARAM)&TcItem);

	m_PageMap.insert(PageMap::value_type((int)nCount, pPage));
	if(nCount == 0)
	{
		pPage->Show(true);
		m_pActivePage = pPage;
	}
	return true;
}

void DialogTab::Close()
{
	m_PageMap.erase(m_PageMap.begin(), m_PageMap.end());
	m_pActivePage = NULL;
	m_hTab = NULL;
}

bool DialogTab::ActivatePage(DialogObject *pPage)
{
	if(m_hTab == NULL)
		return false;
	PageMap::iterator Iterator = m_PageMap.begin();
	for(; Iterator != m_PageMap.end(); Iterator++)
	{
		if((*Iterator).second == pPage)
		{
			if(::SendMessage(m_hTab, TCM_SETCURSEL, (*Iterator).first, 0) == -1)
				return false;
			DoActivatePage(pPage);
			return true;
		}
	}
	return false;
}

void DialogTab::OnNotifySelectChange(NMHDR &rInfo)
{
	if(rInfo.code == TCN_SELCHANGE)
	{
		LRESULT nSelected = ::SendMessage(rInfo.hwndFrom, TCM_GETCURSEL, 0, 0);
		PageMap::iterator Iterator = m_PageMap.find((int)nSelected);
		if(Iterator != m_PageMap.end())
		{
			DoActivatePage((*Iterator).second);
		}
	}
}

void DialogTab::DoActivatePage(DialogObject *pNextActivePage)
{
	if(m_pActivePage != pNextActivePage)
	{
		if(m_pActivePage)
		{
			m_pActivePage->Show(false);
		}
		m_pActivePage = pNextActivePage;
		if(m_pActivePage)
		{
			m_pActivePage->Show(true);
		}
	}
}