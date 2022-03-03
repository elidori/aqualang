#include "Pch.h"
#include "KeyboardMapper.h"

KeyboardMapper::KeyboardMapper()
	: m_nLayouts(0)
{
	Open();

	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("CTRL"), VK_CONTROL));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("LCTRL"), VK_LCONTROL));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("RCTRL"), VK_RCONTROL));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("ALT"), VK_MENU));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("LALT"), VK_LMENU));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("RALT"), VK_RMENU));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("SHIFT"), VK_SHIFT));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("LSHIFT"), VK_LSHIFT));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("NEXT"), VK_NEXT));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("END"), VK_END));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("HOME"), VK_HOME));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("LEFT"), VK_LEFT));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("UP"), VK_UP));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("RIGHT"), VK_RIGHT));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("DOWN"), VK_DOWN));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("INSERT"), VK_INSERT));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("DELETE"), VK_DELETE));

	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("NUMLOCK"), VK_NUMLOCK));

	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("="), VK_OEM_PLUS ));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("+"), VK_OEM_PLUS ));

	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F1"), VK_F1));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F2"), VK_F2));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F3"), VK_F3));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F4"), VK_F4));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F5"), VK_F5));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F6"), VK_F6));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F7"), VK_F7));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F8"), VK_F8));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F9"), VK_F9));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F10"), VK_F10));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F11"), VK_F11));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F12"), VK_F12));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F13"), VK_F13));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F14"), VK_F14));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F15"), VK_F15));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F16"), VK_F16));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F17"), VK_F17));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F18"), VK_F18));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F19"), VK_F19));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F20"), VK_F20));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F21"), VK_F21));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F22"), VK_F22));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F23"), VK_F23));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F24"), VK_F24));

	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("0"), 0x30));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("1"), 0x31));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("2"), 0x32));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("3"), 0x33));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("4"), 0x34));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("5"), 0x35));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("6"), 0x36));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("7"), 0x37));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("8"), 0x38));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("9"), 0x39));

	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("A"), 0x41));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("B"), 0x42));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("C"), 0x43));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("D"), 0x44));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("E"), 0x45));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("F"), 0x46));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("G"), 0x47));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("H"), 0x48));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("I"), 0x49));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("J"), 0x4a));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("K"), 0x4b));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("L"), 0x4c));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("M"), 0x4d));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("N"), 0x4e));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("O"), 0x4f));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("P"), 0x50));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("Q"), 0x51));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("R"), 0x52));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("S"), 0x53));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("T"), 0x54));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("U"), 0x55));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("V"), 0x56));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("W"), 0x57));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("X"), 0x58));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("Y"), 0x59));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("Z"), 0x5a));
	m_VirtualKeyStringMap.insert(VirtualKeyStringMap::value_type(_T("PRINTABLE"), MY_VK_PRINTABLE));
}

KeyboardMapper::~KeyboardMapper()
{
}

bool KeyboardMapper::GetKey(LPCTSTR KeyString, size_t nStringSize, int &rKey) const
{
	rKey = -1;

	TCHAR String[128];
	if(nStringSize > sizeof(String) / sizeof(String[0]))
		return false;
	_tcsncpy_s(String, sizeof(String) / sizeof(String[0]), KeyString, nStringSize);

	VirtualKeyStringMap::const_iterator Iterator = m_VirtualKeyStringMap.find(String);
	if(Iterator == m_VirtualKeyStringMap.end())
		return false;
	rKey = (*Iterator).second;
	return true;
}

HKL KeyboardMapper::GetLayout(int nIndex) const
{
	if(nIndex < 0 || nIndex >= m_nLayouts)
		return NULL;
	return m_LayoutList[nIndex];
}

HKL KeyboardMapper::ToggleLayout(HKL hKL) const
{
	if(m_nLayouts == 0)
		return NULL;

	for(int i = 0; i < m_nLayouts; i++)
	{
		if(hKL == m_LayoutList[i])
		{
			return (i == m_nLayouts - 1) ? m_LayoutList[0] : m_LayoutList[i + 1];
		}
	}
	return NULL;
}

bool KeyboardMapper::Open()
{
	if(m_nLayouts > 0)
		return true;

	m_nLayouts = ::GetKeyboardLayoutList(MAX_LAYOUTS, m_LayoutList);
	if(m_nLayouts == 0)
		return false;

	return true;
}

bool KeyboardMapper::IsShiftKey(int VirtualKey) // static
{
	switch(VirtualKey)
	{
	case VK_SHIFT:
	case VK_LSHIFT:
	case VK_RSHIFT:
		return true;
	}
	return false;
}

bool KeyboardMapper::IsFunctionKey(int VirtualKey) // static
{
	switch(VirtualKey)
	{
	case VK_F1:
	case VK_F2:
	case VK_F3:
	case VK_F4:
	case VK_F5:
	case VK_F6:
	case VK_F7:
	case VK_F8:
	case VK_F9:
	case VK_F10:
	case VK_F11:
	case VK_F12:
	case VK_F13:
	case VK_F14:
	case VK_F15:
	case VK_F16:
	case VK_F17:
	case VK_F18:
	case VK_F19:
	case VK_F20:
	case VK_F21:
	case VK_F22:
	case VK_F23:
	case VK_F24:
		return true;
	}
	return false;
}

bool KeyboardMapper::IsControlKey(int VirtualKey) // static
{
	switch(VirtualKey)
	{
	case VK_CONTROL:
	case VK_LCONTROL:
	case VK_RCONTROL:
	case VK_MENU:
	case VK_LMENU:
	case VK_RMENU:
		return true;
	}
	return false;
}

bool KeyboardMapper::IsWindowKey(int VirtualKey) // static
{
	switch(VirtualKey)
	{
	case VK_LWIN:
	case VK_RWIN:
		return true;
	}
	return false;
}

bool KeyboardMapper::IsExtendedKey(int VirtualKey) // static
{
	switch(VirtualKey)
	{
	case VK_RMENU:
	case VK_RCONTROL:
	case VK_INSERT:
	case VK_DELETE:
	case VK_HOME:
	case VK_END:
	case VK_PRIOR:
	case VK_NEXT:
	case VK_LEFT:
	case VK_UP:
	case VK_RIGHT:
	case VK_DOWN:
	case VK_NUMLOCK:
	case VK_CANCEL:
	case VK_SNAPSHOT:
	case VK_DIVIDE:
	case VK_RETURN:
		return true;
	}
	return false;
}

bool KeyboardMapper::IsLockKey(int VirtualKey)
{
	return (VirtualKey == VK_NUMLOCK);
}

bool KeyboardMapper::ShouldIgnoreLongPress(int VirtualKey) // static
{
	switch(VirtualKey)
	{
	case VK_ESCAPE:
	case VK_CONTROL:
	case VK_LCONTROL:
	case VK_RCONTROL:
	case VK_MENU:
	case VK_LMENU:
	case VK_RMENU:
	case VK_SHIFT:
	case VK_LSHIFT:
	case VK_RSHIFT:
	case VK_F1:
	case VK_F2:
	case VK_F3:
	case VK_F4:
	case VK_F5:
	case VK_F6:
	case VK_F7:
	case VK_F8:
	case VK_F9:
	case VK_F10:
	case VK_F11:
	case VK_F12:
	case VK_F13:
	case VK_F14:
	case VK_F15:
	case VK_F16:
	case VK_F17:
	case VK_F18:
	case VK_F19:
	case VK_F20:
	case VK_F21:
	case VK_F22:
	case VK_F23:
	case VK_F24:
	case VK_NUMLOCK:
		return true;
	}
	return false;
}

bool KeyboardMapper::CanPrintKey(int VirtualKey, bool &rfSpecialKey, bool fIgnoreKeyboardState /*= false*/) // static
{
	rfSpecialKey = false;

	if(!fIgnoreKeyboardState)
	{
		if((::GetKeyState(VK_CONTROL) & 0x80) != 0 || (::GetKeyState(VK_MENU) & 0x80) != 0)
			return false;
	}

	if(VirtualKey >= 0x30 && VirtualKey <= 0x5a)
		return true;

	switch(VirtualKey)
	{
	case VK_ESCAPE:
	case VK_CONTROL:
	case VK_LCONTROL:
	case VK_RCONTROL:
	case VK_MENU:
	case VK_LMENU:
	case VK_RMENU:
	case VK_F1:
	case VK_F2:
	case VK_F3:
	case VK_F4:
	case VK_F5:
	case VK_F6:
	case VK_F7:
	case VK_F8:
	case VK_F9:
	case VK_F10:
	case VK_F11:
	case VK_F12:
	case VK_F13:
	case VK_F14:
	case VK_F15:
	case VK_F16:
	case VK_F17:
	case VK_F18:
	case VK_F19:
	case VK_F20:
	case VK_F21:
	case VK_F22:
	case VK_F23:
	case VK_F24:
	case VK_NUMLOCK:
		return false;
	case VK_BACK:
	case VK_SHIFT:
	case VK_LSHIFT:
	case VK_RSHIFT:
	case VK_SPACE:
	case VK_NUMPAD0:
	case VK_NUMPAD1:
	case VK_NUMPAD2:
	case VK_NUMPAD3:
	case VK_NUMPAD4:
	case VK_NUMPAD5:
	case VK_NUMPAD6:
	case VK_NUMPAD7:
	case VK_NUMPAD8:
	case VK_NUMPAD9:
	case VK_MULTIPLY:
	case VK_ADD:
	case VK_SEPARATOR:
	case VK_SUBTRACT:
	case VK_DECIMAL:
	case VK_DIVIDE:
	case VK_OEM_1:
	case VK_OEM_PLUS:
	case VK_OEM_COMMA:
	case VK_OEM_MINUS:
	case VK_OEM_PERIOD:
	case VK_OEM_2:
	case VK_OEM_3:
	case VK_OEM_4:
	case VK_OEM_5:
	case VK_OEM_6:
	case VK_OEM_7:
	case VK_OEM_8:
	case MY_VK_PRINTABLE:
		return true;
	}
	rfSpecialKey = true;
	return false;
}

int KeyboardMapper::GetNumOfCharacters(int VirtualKey) // static
{
	if(VirtualKey >= 0x30 && VirtualKey <= 0x5a)
		return 1;

	switch(VirtualKey)
	{
	case VK_BACK:
		return -1;
	case VK_SHIFT:
	case VK_LSHIFT:
	case VK_RSHIFT:
		return 0;
	case VK_SPACE:
	case VK_NUMPAD0:
	case VK_NUMPAD1:
	case VK_NUMPAD2:
	case VK_NUMPAD3:
	case VK_NUMPAD4:
	case VK_NUMPAD5:
	case VK_NUMPAD6:
	case VK_NUMPAD7:
	case VK_NUMPAD8:
	case VK_NUMPAD9:
	case VK_MULTIPLY:
	case VK_ADD:
	case VK_SEPARATOR:
	case VK_SUBTRACT:
	case VK_DECIMAL:
	case VK_DIVIDE:
	case VK_OEM_1:
	case VK_OEM_PLUS:
	case VK_OEM_COMMA:
	case VK_OEM_MINUS:
	case VK_OEM_PERIOD:
	case VK_OEM_2:
	case VK_OEM_3:
	case VK_OEM_4:
	case VK_OEM_5:
	case VK_OEM_6:
	case VK_OEM_7:
	case VK_OEM_8:
		return 1;
	}
	return 0;
}

