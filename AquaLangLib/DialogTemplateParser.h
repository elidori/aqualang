#ifndef _DIALOG_TEMPLATE_PARSER_HEADER_
#define _DIALOG_TEMPLATE_PARSER_HEADER_

class DialogItemParser;

class DialogTemplateParser
{
public:
	DialogTemplateParser();
	virtual ~DialogTemplateParser();

	bool Open(LPVOID pTemplate); // pTemplate points to DLGTEMPLATEEX

	const RECT &GetDialogRect() const	{ return m_DialogRect; } // in dialog units

	size_t GetItemCount();
	bool GetItem(index_t nIndex, const DialogItemParser* &rpItem);

	void Close();
private:
	LPBYTE GetItemArrayPointer(const DLGTEMPLATE &Template);
private:
	RECT m_DialogRect;

	typedef std::vector<DialogItemParser *> DialogItemList;
	DialogItemList m_DialogItemList;
};

class DialogItemParser
{
public:
	DialogItemParser();
	virtual ~DialogItemParser();

	void Parse(LPBYTE &pItemTemplate);

	int GetItemId() const			{ return m_ItemId; }
	const RECT &GetItemRect() const { return m_ItemRect; } // in dialog units
private:
	int m_ItemId;
	RECT m_ItemRect;
};

#endif // _DIALOG_TEMPLATE_PARSER_HEADER_