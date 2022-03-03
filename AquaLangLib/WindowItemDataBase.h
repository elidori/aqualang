#ifndef _WINDOW_ITEM_DATABASE_HEADER_
#define _WINDOW_ITEM_DATABASE_HEADER_

#include "WindowItem.h"

class LanguageStringTable;

class WindowItemDataBase
{
public:
	WindowItemDataBase(const LanguageStringTable &rLanguageStringTable);
	virtual ~WindowItemDataBase();

	bool Open(LPCTSTR UserName);
	bool OpenRulesOnly(LPCTSTR UserName);
	void Close();

	WindowItem *GetWindowItem(HWND hwnd, LPCTSTR SubObjectId, bool &rfCreated, HKL *pDefaultLanguage, const WindowItem* &rpWindowRule);
	bool GetLanguageString(HKL hKL, _tstring &rLanguageId, _tstring &rLanguageSpecific) const;

	const WindowItem *QueryWindowItem(HWND hwnd, const WindowItem** ppWindowRule) const;
private:
	WindowItem *FindWindowItem(HWND hwnd, bool &rfCreated, bool fEmptyListMatches);
	bool IsSerialized(LPCTSTR FileName) const;
private:
	bool m_fOpen;
	bool m_fOpenRulesOnly;
	const bool m_fUseSerialization;

	WindowItem m_RootItem;
	xmlTree m_xmlTree;

	WindowItem m_RulesItem;
	xmlTree m_xmlRulesTree;

	const LanguageStringTable &m_rLanguageStringTable;
};

#endif //_WINDOW_ITEM_DATABASE_HEADER_