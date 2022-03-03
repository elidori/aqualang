#ifndef _KEYBOARD_MAPPER_HEADER_
#define _KEYBOARD_MAPPER_HEADER_

#define MAX_LAYOUTS 256

enum
{
	MY_VK_PRINTABLE = 0x100
};

class KeyboardMapper
{
public:
	KeyboardMapper();
	virtual ~KeyboardMapper();

	bool GetKey(LPCTSTR KeyString, size_t nStringSize, int &rKey) const;

	int GetLayoutCount() const	{ return m_nLayouts; }
	HKL GetLayout(int nIndex) const;
	HKL ToggleLayout(HKL hKL) const; 

	static bool IsShiftKey(int VirtualKey);
	static bool IsFunctionKey(int VirtualKey);
	static bool IsControlKey(int VirtualKey);
	static bool IsWindowKey(int VirtualKey);
	static bool IsExtendedKey(int VirtualKey);
	static bool IsLockKey(int VirtualKey);
	static bool ShouldIgnoreLongPress(int VirtualKey);
	static bool CanPrintKey(int VirtualKey, bool &rfSpecialKey, bool fIgnoreKeyboardState = false);
	static int GetNumOfCharacters(int VirtualKey);
private:
	bool Open();
private:
	int m_nLayouts;
	HKL m_LayoutList[MAX_LAYOUTS];

	typedef std::map<_tstring, int/*VirtualKey*/> VirtualKeyStringMap;
	VirtualKeyStringMap m_VirtualKeyStringMap;
};

#endif // _KEYBOARD_MAPPER_HEADER_