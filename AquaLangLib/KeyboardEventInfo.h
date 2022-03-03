#ifndef _KEYBOARD_EVENT_INFO_HEADER_
#define _KEYBOARD_EVENT_INFO_HEADER_

class FocusInfo;
class LanguageDetector;
class KeyboardSequenceEvent;
class WindowItemDataBase;
class WindowItem;

class KeyboardEventInfo
{
public:
	KeyboardEventInfo(HWND hCurrentWindow, const FocusInfo &rFocusInfo, const LanguageDetector &rLanguageDetector, const KeyboardSequenceEvent &rKeyboardSequenceEvent, const WindowItemDataBase &rWindowsDatabase);
	virtual ~KeyboardEventInfo();

	HWND GetCurrentWindow() const	{ return m_hCurrentWindow; }
	const WindowItem *GetWindowItem(const WindowItem** ppWindowRule) const;

	bool PeekAndLoadMarkedText();
	LPCWSTR PeekMarkedText(HKL &rhTextKL) const;

	bool GetMarkedOrRecentText(_tstring &rText, HKL &rhTextKL, bool fExcludeOlderText) const;
	bool GetRecentText(_tstring &rText, HKL &rhTextKL, bool fExcludeOlderText) const;

	bool QueryWorkingRect(RECT &rRect, int &rRectType) const;
private:
	const HWND m_hCurrentWindow;
	
	const FocusInfo &m_rFocusInfo;
	const LanguageDetector &m_rLanguageDetector;
	const KeyboardSequenceEvent &m_rKeyboardSequenceEvent;

	bool m_fClipboardWasPeeked;
	std::wstring m_ClipboardText;
	HKL m_hTextKL;

	const WindowItemDataBase &m_rWindowsDatabase;
};

#endif // _KEYBOARD_EVENT_INFO_HEADER_