#ifndef _CLIPBOARD_ACCESS_HEADER_
#define _CLIPBOARD_ACCESS_HEADER_

struct ClipboardDataItem
{
	UINT Format;
	HGLOBAL hglbCopy;
};

class ClipboardData
{
public:
	ClipboardData();
	virtual ~ClipboardData();

	bool Push(UINT Format, HGLOBAL hglb);
	bool Pop(ClipboardDataItem &rItem);

	void Close();
private:
	std::vector<ClipboardDataItem> m_DataList;
};

class ClipboardAccess
{
public:
	static bool SetText(HWND hwnd, LPCWSTR wText, HKL hKL = NULL);
	static bool PeekMarkedText(HWND hwnd, std::wstring &wText);
	static bool CutAndToggelWithClipboard(HWND hwnd);

	static bool GetText(HWND &rhwnd, std::wstring &wText);
	static bool GetText(HWND &rhwnd, std::string &Text);

	static bool GetData(HWND &rhwnd, ClipboardData &rData);
	static bool SetData(HWND hwnd, ClipboardData &rData);

	static bool PasteText(HWND hwnd, LPCWSTR Text);
private:
	static LCID ConvertKeyboardLayoutToLocale(HKL hKL);
	static bool CanSendWindowMessage(HWND hwnd);
};

#endif //_CLIPBOARD_ACCESS_HEADER_