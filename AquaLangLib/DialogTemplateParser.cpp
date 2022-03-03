#include "Pch.h"
#include "DialogTemplateParser.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogTemplateParser
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DialogTemplateParser::DialogTemplateParser()
{
}

DialogTemplateParser::~DialogTemplateParser()
{
	Close();
}

bool DialogTemplateParser::Open(LPVOID pTemplate)
{
	Close();

	const DLGTEMPLATE &Template = *(const DLGTEMPLATE *)((LPBYTE)pTemplate + sizeof(WORD)/*dlgVer*/ + sizeof(WORD)/*signature*/ + sizeof(DWORD)/*helpID*/);

	SetRect(&m_DialogRect, Template.x, Template.y, Template.x + Template.cx, Template.y + Template.cy);

	LPBYTE pItemTemplate = GetItemArrayPointer(Template);

	for(int i = 0; i < Template.cdit; i++)
	{
		DialogItemParser *pNew = new DialogItemParser;
		if(!pNew)
			return false;
		pNew->Parse(pItemTemplate);
		m_DialogItemList.push_back(pNew);
	}
	return true;
}

size_t DialogTemplateParser::GetItemCount()
{
	return m_DialogItemList.size();
}

bool DialogTemplateParser::GetItem(index_t nIndex, const DialogItemParser* &rpItem)
{
	rpItem = NULL;

	if(nIndex < 0 || nIndex >= (index_t)m_DialogItemList.size())
		return false;
	
	rpItem = m_DialogItemList.at(nIndex);
	return true;
}

void DialogTemplateParser::Close()
{
	for(unsigned int i = 0; i < m_DialogItemList.size(); i++)
	{
		delete m_DialogItemList.at(i);
	}
	m_DialogItemList.erase(m_DialogItemList.begin(), m_DialogItemList.end());
}

LPBYTE DialogTemplateParser::GetItemArrayPointer(const DLGTEMPLATE &Template)
{
	LPBYTE pPointer = (LPBYTE)&Template;
	pPointer += 4/*exStyle*/ + 4/*style*/ + 2/*cDlgItems*/ + 2/*x*/ + 2/*y*/ + 2/*cx*/ + 2/*cy*/;

	LPWORD pwPointer = (LPWORD)pPointer;

	// skip menu array
	if(*pwPointer == 0)
	{
		pwPointer++;
	}
	else if(*pwPointer == 0xFFFF)
	{
		pwPointer += 2;
	}
	else
	{
		pwPointer += wcslen((LPCWSTR)pwPointer) + 1;
	}

	// skip class array
	if(*pwPointer == 0)
	{
		pwPointer++;
	}
	else if(*pwPointer == 0xFFFF)
	{
		pwPointer += 2;
	}
	else
	{
		pwPointer += wcslen((LPCWSTR)pwPointer) + 1;
	}

	// skip title array
	pwPointer += wcslen((LPCWSTR)pwPointer) + 1;

	DWORD Style = Template.dwExtendedStyle; // this is because there is a swap in the position of style and extended style between DLGTEMPLATE and DLGTEMPLATEEX

	if((Style & (DS_SETFONT | DS_SHELLFONT)) != 0)
	{
		// pointsize
		pwPointer++;
		// weight
		pwPointer++;
		// Italic + Charset
		pwPointer++;
		// skip typface
		pwPointer += wcslen((LPCWSTR)pwPointer) + 1;
	}
	pPointer = (LPBYTE)pwPointer;
	// align to dword
	if(((int)pPointer % sizeof(DWORD)) != 0)
	{
		pPointer += sizeof(DWORD) - ((int)pPointer % sizeof(DWORD));
	}

	return pPointer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogItemParser
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DialogItemParser::DialogItemParser()
	: m_ItemId(-1)
{
}

DialogItemParser::~DialogItemParser()
{
}

void DialogItemParser::Parse(LPBYTE &pItemTemplate)
{
	const DLGITEMTEMPLATE &Template = *(const DLGITEMTEMPLATE *)(pItemTemplate + sizeof(DWORD)/*helpID*/);

	m_ItemId = Template.id;
	SetRect(&m_ItemRect, Template.x, Template.y, Template.x + Template.cx, Template.y + Template.cy);

	LPWORD pwPointer = (LPWORD)(pItemTemplate + 4/*helpID*/ + 4/*exStyle*/ + 4/*style*/ + 2/*x*/ + 2/*y*/ + 2/*cx*/ + 2/*cy*/ + 4/*id*/);

	// skip windowClass
	if(*pwPointer == 0)
	{
		pwPointer++;
	}
	else if(*pwPointer == 0xFFFF)
	{
		pwPointer += 2;
	}
	else
	{
		pwPointer += wcslen((LPCWSTR)pwPointer) + 1;
	}

	// skip title
	if(*pwPointer == 0)
	{
		pwPointer++;
	}
	else if(*pwPointer == 0xFFFF)
	{
		pwPointer += 2;
	}
	else
	{
		pwPointer += wcslen((LPCWSTR)pwPointer) + 1;
	}

	// skip extra count
	WORD extraCount = *pwPointer;
	pwPointer++;

	pItemTemplate = (LPBYTE)pwPointer + extraCount;

	// align to dword
	if(((int)pItemTemplate % sizeof(DWORD)) != 0)
	{
		pItemTemplate += sizeof(DWORD) - ((int)pItemTemplate % sizeof(DWORD));
	}
}