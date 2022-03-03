#include "Pch.h"
#include "KeyCode.h"
#include "KeyboardDefinitions.h"
#include "KeyboardMapper.h"

KeyCode::KeyCode(int nVirtualKey, int Operator, bool fCanPrint, bool fSimulated)
{
	m_nKey = 
		nVirtualKey |
		Operator |
		(fCanPrint ? 0 : KEY_NON_PRINTABLE) |
		(((::GetKeyState(VK_SHIFT) & 0x80) != 0) ? KEY_SHIFT_IS_DOWN : 0) |
		(fSimulated ? KEY_SIMULATED : 0);
}

KeyCode::KeyCode(int nKey /*= -1*/)
	: m_nKey(nKey)
{
}

KeyCode::~KeyCode()
{
}

void KeyCode::Set(int nKey)
{
	m_nKey = nKey;
}

void KeyCode::Clear()
{
	m_nKey = -1;
}

int KeyCode::Get() const
{
	return m_nKey;
}

bool KeyCode::IsValid() const
{
	return (m_nKey != -1);
}

int KeyCode::VirtualKey() const
{
	return (m_nKey & VIRTUAL_KEY_MASK);
}

int KeyCode::Operation() const
{
	return (m_nKey & KEY_OPERATOR_MASK);
}

bool KeyCode::IsKeyPressed() const
{
	if(!IsValid())
		return false;
	return ((m_nKey & KEY_OPERATOR_MASK) == KEY_OPERATOR_PLUS || (m_nKey & KEY_OPERATOR_MASK) == KEY_OPERATOR_PLUSPLUS);
}

bool KeyCode::IsClearAllKey() const
{
	if(!IsValid())
		return false;
	return ((m_nKey & KEY_OPERATOR_MASK) == KEY_OPERATOR_MINUSMINUS);
}

bool KeyCode::IsPrintable() const
{
	if(!IsValid())
		return false;
	return ((m_nKey & KEY_NON_PRINTABLE) == 0);
}

bool KeyCode::IsShiftDown() const
{
	if(!IsValid())
		return false;
	return ((m_nKey & KEY_SHIFT_IS_DOWN) != 0);
}

bool KeyCode::IsShiftKey() const
{
	if(!IsValid())
		return false;
	switch(VirtualKey())
	{
	case VK_SHIFT:
	case VK_LSHIFT:
	case VK_RSHIFT:
		return true;
	}
	return false;
}

bool KeyCode::IsSimulated(int Key)
{
	return ((Key & KEY_SIMULATED) != 0);
}

bool KeyCode::IsKeyDownAndUp(int VirtualKey, int Key1, int Key2) // static
{
	return 
		(
			(Key1 & VIRTUAL_KEY_MASK) == VirtualKey &&
			(Key1 & KEY_OPERATOR_MASK) == KEY_OPERATOR_PLUS &&
			(Key2 & VIRTUAL_KEY_MASK) == VirtualKey &&
			(Key2 & KEY_OPERATOR_MASK) == KEY_OPERATOR_MINUS
		);
}

bool KeyCode::IsMatching(int nRefKey) const
{
	if(!IsValid() || nRefKey == -1)
		return false;

	// verify case of '--'
	if((nRefKey & KEY_OPERATOR_MASK) == KEY_OPERATOR_MINUSMINUS)
	{
		if((m_nKey & KEY_OPERATOR_MASK) != KEY_OPERATOR_MINUS)
			return false;
		return true;
	}

	// verify operators match
	if ((m_nKey & KEY_OPERATOR_MASK) != (nRefKey & KEY_OPERATOR_MASK) &&
		!(((m_nKey & KEY_OPERATOR_MASK) == KEY_OPERATOR_PLUSPLUS) && ((nRefKey & KEY_OPERATOR_MASK) == KEY_OPERATOR_PLUS)))
	{
		return false;
	}

	// verify virtual key
	int RefVirtualKey = nRefKey & VIRTUAL_KEY_MASK;
	int VirtualKey = m_nKey & VIRTUAL_KEY_MASK;

	if(VirtualKey == RefVirtualKey || RefVirtualKey == MY_VK_PRINTABLE && IsPrintable() && !IsShiftKey())
		return true;

	// verify buttons who have logical values
	if(RefVirtualKey == VK_CONTROL && (VirtualKey == VK_LCONTROL || VirtualKey == VK_RCONTROL))
		return true;
	if(RefVirtualKey == VK_MENU && (VirtualKey == VK_LMENU || VirtualKey == VK_RMENU))
		return true;
	if(RefVirtualKey == VK_SHIFT && (VirtualKey == VK_LSHIFT || VirtualKey == VK_RSHIFT))
		return true;
	return false;
}
