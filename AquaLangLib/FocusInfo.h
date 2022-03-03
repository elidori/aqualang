#ifndef _FOCUS_INFO_HEADER_
#define _FOCUS_INFO_HEADER_

class OleAccLibrary
{
public:
	OleAccLibrary();
	virtual ~OleAccLibrary();

	bool Open();
	bool IsOpen() const;
	void Close();

	HRESULT ObjectFromLresult(LRESULT lResult, REFIID riid, WPARAM wParam, void** ppvObject);
private:
	HINSTANCE m_hLib;

	LPFNOBJECTFROMLRESULT FuncObjectFromLresult;
};


enum WORKING_RECT_TYPE
{
	WORKING_RECT_NONE,

	WORKING_RECT_CARET,
	WORKING_RECT_MOUSE,
	WORKING_RECT_TOP_LEFT_WINDOW
};

class FocusInfo
{
public:
	FocusInfo();
	virtual ~FocusInfo();

	bool EnableHTMLChecking();

	bool UpdateCurrentInfo(bool fInHook, bool &rfFocusChanged);
	void Clear();

	HWND QueryCurrentFocusWindow() const;
	HWND QueryCurrentCaretWindow() const;
	HWND QueryCurrentEditingWindow() const; // caret or focus

	LPCTSTR GetSubObjectId() const;

	bool QueryCaretGlobalRect(RECT &rRect) const;
	bool QueryWorkingRect(RECT &rRect, int &rRectType) const;

	HKL QueryCurrentLanguage() const;
private:
	bool IsCursorInWindow(const POINT &CursorPos, const RECT &WindowRect) const;
	bool CheckHTML(HWND hwnd, _tstring &rSubObjectId);
private:
	bool m_fDataIsValid;
	GUITHREADINFO m_GuiThreadInfo;
	HWND m_hFocusWindow;
	_tstring m_SubObjectId; // in case of IE HTML

	OleAccLibrary m_OleAccLibrary;
};

#endif // _FOCUS_INFO_HEADER_