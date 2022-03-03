#include "Pch.h"
#include "KeyboardSequence.h"
#include "KeyboardMapper.h"
#include "SyncHelpers.h"
#include "StringConvert.h"
#include "KeyCode.h"
#include "KeyboardDefinitions.h"
#include "Log.h"

KeyboardSequence::KeyboardSequence(bool fReadOnly /*= false*/)
	: m_hRecentWindow(NULL),
	m_fReadOnly(fReadOnly),
	m_fHasWildcards(false),
	m_fInControlKeySequence(false)
{
}

KeyboardSequence::KeyboardSequence(const KeyboardSequence &r)
	: m_fReadOnly(r.m_fReadOnly),
	m_fHasWildcards(false),
	m_fInControlKeySequence(false)
{
	Set(r);
}

KeyboardSequence::~KeyboardSequence()
{
	Close();
}

bool KeyboardSequence::Set(const KeyboardSequence &r)
{
	bool fSuccess = m_KeySequence.Set(r.m_KeySequence);
	m_RecentText.Set(r.m_RecentText);

	m_PressedKeyList.erase(m_PressedKeyList.begin(), m_PressedKeyList.end());
	for(unsigned int i = 0; i < r.m_PressedKeyList.size(); i++)
	{
		m_PressedKeyList.push_back(r.m_PressedKeyList.at(i));
	}
	m_LatestKeyPressedTime.Set(r.m_LatestKeyPressedTime);

	m_fHasWildcards = r.m_fHasWildcards;
	
	m_fInControlKeySequence = r.m_fInControlKeySequence;

	return fSuccess;
}

bool KeyboardSequence::Open(int MaxKeySequenceSize, int MaxRecentTextSize, LPCTSTR Sequence /*= NULL*/, bool fBackground /*= false*/)
{
	if(!m_KeySequence.Open(MaxKeySequenceSize))
		return false;
	if(!m_RecentText.Open(MaxRecentTextSize))
		return false;

	if(!m_PrevKeySequence.Open(MaxKeySequenceSize))
		return false;

	if(Sequence != NULL)
	{
		if(!Init(Sequence, fBackground))
			return false;
	}
	return true;
}

void KeyboardSequence::Close()
{
	Term();
	m_KeySequence.Close();
}

bool KeyboardSequence::Init(LPCTSTR Sequence, bool fBackground)
{
	Term();

	int NextKey;
	int NextOperator;
	LPTSTR RemainingSequence = (LPTSTR)Sequence;

	while(GetNextKey(NextKey, NextOperator, RemainingSequence))
	{
		if(NextOperator == KEY_OPERATOR_MINUSMINUS)
		{
			size_t nPressedKeys = m_PressedKeyList.size();
			for(index_t i = 0; i < (index_t)nPressedKeys; i++)
			{
				AddKey(VK_SPACE, KEY_OPERATOR_MINUSMINUS, NULL, NULL, 0, NULL, NULL);
			}
			m_PressedKeyList.erase(m_PressedKeyList.begin(), m_PressedKeyList.end());
		}
		else
		{
			if(!fBackground)
			{
				if(m_PressedKeyList.size() == 0)
				{
					// make sure that the first key is function or control
					if(!KeyboardMapper::IsFunctionKey(NextKey) && !KeyboardMapper::IsControlKey(NextKey) && !KeyboardMapper::IsShiftKey(NextKey) && !KeyboardMapper::IsLockKey(NextKey))
						return false;
				}
				else
				{
					// if a key was pressed earlier, make sure that a control key is already pressed
					if(NextOperator != KEY_OPERATOR_MINUS)
					{
						if(!KeyboardMapper::IsControlKey(m_PressedKeyList.at(0)))
							return false;
					}
				}
			}
			if(!KeyEvent(NextKey, NextOperator, NULL, NULL, NULL, NULL))
			{
				Term();
				return false;
			}
		}
	}
	if(m_KeySequence.IsEmpty())
		return false;

	return true;
}

void KeyboardSequence::Term()
{
	Clear();
	m_PressedKeyList.erase(m_PressedKeyList.begin(), m_PressedKeyList.end());
	m_LatestKeyPressedTime.Restart();
	m_fInControlKeySequence = false;
}

bool KeyboardSequence::KeyEvent(int VirtualKey, int Operator, /*in*/int *pComposedKeyList, /*in, out*/int *pListSize, HKL hKL, HWND hCurrentWindow)
{
	int ListSizeLimit = 0;
	if(pListSize)
	{
		ListSizeLimit = *pListSize;
		*pListSize = 0;
	}

	// check validity
	if(VirtualKey == -1)
		return false;

	// act according to operator
	switch(Operator)
	{
	case KEY_OPERATOR_PLUS:
		{
			if(KeyboardMapper::IsControlKey(VirtualKey) || KeyboardMapper::IsWindowKey(VirtualKey))
			{
				m_fInControlKeySequence = true;
			}

			if(VirtualKey == MY_VK_PRINTABLE)
			{
				m_fHasWildcards = true;
			}
			bool fKeyRepeating = false;
			int nPressedKeyIndex = FindIndex(VirtualKey);
			// make sure that either the key was not pressed before, and if it was pressed, it is the last one pressed
			if(nPressedKeyIndex == -1)
			{
				m_PressedKeyList.push_back(VirtualKey);
			}
			else if(nPressedKeyIndex == (int)(m_PressedKeyList.size() - 1))
			{
				if(KeyboardMapper::ShouldIgnoreLongPress(VirtualKey))
					return true;
				AddKey(VirtualKey, KEY_OPERATOR_MINUS, pComposedKeyList, pListSize, ListSizeLimit, hKL, hCurrentWindow);
				fKeyRepeating = true;
				m_fInControlKeySequence = false;
			}
			else
			{
				return false;
			}

			if (!fKeyRepeating &&
				m_KeySequence.GetCount() >= 2 &&
				KeyCode::IsKeyDownAndUp(VirtualKey, m_KeySequence.Get(m_KeySequence.GetCount() - 2), m_KeySequence.Get(m_KeySequence.GetCount() - 1)) &&
				!KeyCode::IsSimulated(m_KeySequence.Get(m_KeySequence.GetCount() - 2)) &&
				m_LatestKeyPressedTime.GetDiff() < ::GetDoubleClickTime()
				)
			{
				AddKey(VirtualKey, KEY_OPERATOR_PLUSPLUS, pComposedKeyList, pListSize, ListSizeLimit, hKL, hCurrentWindow);
			}
			else
			{
				AddKey(VirtualKey, KEY_OPERATOR_PLUS, pComposedKeyList, pListSize, ListSizeLimit, hKL, hCurrentWindow);
			}

			m_LatestKeyPressedTime.Restart();
		}
		break;
	case KEY_OPERATOR_PLUSPLUS:
		if(FindIndex(VirtualKey) != -1)
			return false;
		AddKey(VirtualKey, KEY_OPERATOR_PLUS, pComposedKeyList, pListSize, ListSizeLimit, hKL, hCurrentWindow);
		AddKey(VirtualKey, KEY_OPERATOR_MINUS, pComposedKeyList, pListSize, ListSizeLimit, hKL, hCurrentWindow);
		AddKey(VirtualKey, KEY_OPERATOR_PLUSPLUS, pComposedKeyList, pListSize, ListSizeLimit, hKL, hCurrentWindow);
		m_PressedKeyList.push_back(VirtualKey);
		break;
		
	case KEY_OPERATOR_MINUS:
		{
			int nPressedKeyIndex = FindIndex(VirtualKey);
			if(nPressedKeyIndex == -1)
			{
				m_hRecentWindow = hCurrentWindow; // this is to make sure that if several keys are de-pressed in a raw, only the first of them gets the special treatment below
				return false;
			}

			// the following handle change of focus after the down pressing of this key. This is typical
			// for instance in excell, where the first click opens the cell for editing. This workaround enables
			// the including of the first character in the text language conversion
			bool fDummy;

			if((m_KeySequence.IsEmpty() || LastKeyIsNonPrintable() || (hCurrentWindow != m_hRecentWindow)) && CanPrintNewKey(VirtualKey, false, fDummy))
			{
				AddKey(VirtualKey, KEY_OPERATOR_PLUS, pComposedKeyList, pListSize, ListSizeLimit, hKL, hCurrentWindow, true/*fSimulated*/);
			}

			AddKey(VirtualKey, KEY_OPERATOR_MINUS, pComposedKeyList, pListSize, ListSizeLimit, hKL, hCurrentWindow);
			m_PressedKeyList.erase(m_PressedKeyList.begin() + nPressedKeyIndex);
			if(m_PressedKeyList.size() == 0)
			{
				m_fInControlKeySequence = false;
			}
		}
		break;
	default:
		return false;
	}
	return true;
}

bool KeyboardSequence::UpdateLatestTextLanguage(HKL hKL, bool fStartAfterLastNonPrintKey)
{
	m_RecentText.Clear(fStartAfterLastNonPrintKey);
	for(int i = 0; i < m_KeySequence.GetCount(fStartAfterLastNonPrintKey); i++)
	{
		if(!m_RecentText.Update(m_KeySequence.Get(i, fStartAfterLastNonPrintKey), true, hKL))
			return false;
	}
	return true;
}

bool KeyboardSequence::HasOnlyShifts() const
{
	for(int i = 0; i < m_KeySequence.GetCount(); i++)
	{
		KeyCode _KeyCode(m_KeySequence.Get(i));
		if(!KeyboardMapper::IsShiftKey(_KeyCode.VirtualKey()))
		{
			if(!_KeyCode.IsClearAllKey()) // is '--'
				return false;
		}
	}
	return true;
}

bool KeyboardSequence::IsEqual(const KeyboardSequence &r) const
{
	// check state
	if(m_PressedKeyList.size() != r.m_PressedKeyList.size())
		return false;
	int i;
	for(i = 0; i < (int)m_PressedKeyList.size(); i++)
	{
		if(m_PressedKeyList.at(i) != r.m_PressedKeyList.at(i))
			return false;
	}

	// check sequence
	if(m_KeySequence.GetCount() != r.m_KeySequence.GetCount())
		return false;
	for(i = 0; i < r.m_KeySequence.GetCount(); i++)
	{
		if(m_KeySequence.Get(i) != r.m_KeySequence.Get(i))
			return false;
	}
	return true;
}

bool KeyboardSequence::IsMatching(const KeyboardSequence &r) const
{
	// check state
	if(!r.m_fHasWildcards)
	{
		if(m_PressedKeyList.size() != r.m_PressedKeyList.size())
			return false;

		int i;
		for(i = 0; i < (int)m_PressedKeyList.size(); i++)
		{
			int VirtualKey = m_PressedKeyList.at(i);

			if(r.m_PressedKeyList.at(i) == MY_VK_PRINTABLE)
			{
				bool fSpecialKey = false;
				if(!KeyboardMapper::CanPrintKey(VirtualKey, fSpecialKey, true) || KeyboardMapper::IsShiftKey(VirtualKey))
					return false;
			}
			else
			{
				if(VirtualKey != r.m_PressedKeyList.at(i))
				{
					switch(VirtualKey)
					{
					case VK_LCONTROL:
					case VK_RCONTROL:
						if(r.m_PressedKeyList.at(i) != VK_CONTROL)
							return false;
						break;
					case VK_LMENU:
					case VK_RMENU:
						if(r.m_PressedKeyList.at(i) != VK_MENU)
							return false;
						break;
					case VK_LSHIFT:
					case VK_RSHIFT:
						if(r.m_PressedKeyList.at(i) != VK_SHIFT)
							return false;
						break;
					}
				}
			}
		}
	}

	// check sequence
	if(m_KeySequence.GetCount() < r.m_KeySequence.GetCount())
		return false;
	if(m_KeySequence.GetCount() == 0 || r.m_KeySequence.GetCount() == 0)
		return false;

	int RefLastIndex = r.m_KeySequence.GetCount() - 1;
	int LastIndex = m_KeySequence.GetCount() - 1;

	for(int i = 0; i < r.m_KeySequence.GetCount(); i++)
	{
		KeyCode _Key(m_KeySequence.Get(LastIndex - i));
		if(!_Key.IsMatching(r.m_KeySequence.Get(RefLastIndex - i)))
			return false;
	}
	return true;
}

bool KeyboardSequence::IsContaining(const KeyboardSequence &r) const
{
	KeyboardSequence TestSequence(true);
	if(!TestSequence.Open(m_KeySequence.GetMaxCount(), m_RecentText.GetTextMaxLength()))
		return false;

	for(int i = 0; i < m_KeySequence.GetCount(); i++)
	{
		KeyCode _KeyCode(m_KeySequence.Get(i));

		TestSequence.KeyEvent(_KeyCode.VirtualKey(), _KeyCode.Operation(), NULL, NULL, NULL, NULL);
		if(TestSequence.IsMatching(r))
			return true;
	}

	return false;
}

void KeyboardSequence::Clear()
{
	if(!m_KeySequence.IsEmpty())
	{
		m_PrevKeySequence.Set(m_KeySequence);
	}

	m_KeySequence.Clear();
	m_RecentText.Clear();
	m_hRecentWindow = NULL;
}

void KeyboardSequence::FocusChanged(HWND UNUSED(hFocusWindow))
{
	m_PrevKeySequence.Clear();
}

void KeyboardSequence::ClearTextHistory()
{
	m_PrevKeySequence.Clear();
	m_KeySequence.Clear();
	m_RecentText.Clear();
	m_hRecentWindow = NULL;
}

void KeyboardSequence::MarkCurrentPosition()
{
	m_KeySequence.MarkCurrentPosition();
	m_RecentText.MarkCurrentPosition();
}

void KeyboardSequence::ClearReferenceSequence(const KeyboardSequence &r)
{
	assert(IsMatching(r));
	if(!IsMatching(r))
		return;

	// first remove the latest keys, which produces the reference sequence
	int nIgnored = r.m_KeySequence.GetCount();
	for(int i = 0; i < nIgnored; i++)
	{
		int Value;
		m_KeySequence.UnPush(Value);
	}
	m_PressedKeyList.erase(m_PressedKeyList.begin(), m_PressedKeyList.end());
	// tell the keyboard to lift the currently pressed keys
	r.SequenceKeysUp(NULL, NULL);

	// Now, if after clearing the hotkey there still is control keys, then clear
	if(LastKeyIsNonPrintable())
	{
		Clear();
	}
}

int KeyboardSequence::GetCharacterCount(int &rnBackspace, int &rnDeletionMaxDepth, bool fStartAfterLastNonPrintKey) const
{
	rnBackspace = 0;
	rnDeletionMaxDepth = 0;
	int CharCount = 0;

	for(int i = 0; i < m_KeySequence.GetCount(fStartAfterLastNonPrintKey); i++)
	{
		KeyCode _Key(m_KeySequence.Get(i, fStartAfterLastNonPrintKey));

		if(_Key.IsPrintable())
		{
			if(_Key.IsKeyPressed())
			{
				int Count = KeyboardMapper::GetNumOfCharacters(_Key.VirtualKey());
				if(Count > 0)
				{
					CharCount += Count;
				}
				else if(Count < 0)
				{
					rnBackspace -= Count;
				}
				// update max delete depth
				int CurrentDeleteDepth = rnBackspace - CharCount;
				if(CurrentDeleteDepth > rnDeletionMaxDepth)
				{
					rnDeletionMaxDepth = CurrentDeleteDepth;
				}
			}
		}
	}
	return CharCount;
}

// returns the number of backspaces to skip
int KeyboardSequence::BackspaceSequence(HWND hwnd, HKL hKL, bool fStartAfterLastNonPrintKey)
{
	if(m_KeySequence.IsEmpty(fStartAfterLastNonPrintKey))
		return 0;

	int DeletionMaxDepth = 0;
	int BackspaceCount = 0;
	const int CharCount = GetCharacterCount(BackspaceCount, DeletionMaxDepth, fStartAfterLastNonPrintKey);

	const int CharactersToDelete = CharCount - BackspaceCount + DeletionMaxDepth;
	const int DeletionsToSkipWhenRetyping = DeletionMaxDepth;

	for(int i = 0; i < CharactersToDelete; i++)
	{
		PrintKey(VK_BACK, true, false, hwnd, hKL, false);
		PrintKey(VK_BACK, false, false, hwnd, hKL, false);
	}
	// this is to let the target application perform the TeranslateMessage before getting additional requests
	Sleep(100);

	return ((DeletionsToSkipWhenRetyping > 0) ? DeletionsToSkipWhenRetyping : 0);
}

void KeyboardSequence::ReplaySequence(int BackspacesToIgnore, HWND hwnd, HKL hKL, bool fStartAfterLastNonPrintKey)
{
	if(m_KeySequence.IsEmpty(fStartAfterLastNonPrintKey))
		return;

	int nBackspacesToSkip = BackspacesToIgnore;

	// restore shift if needed
	if(KeyCode(m_KeySequence.Get(0, fStartAfterLastNonPrintKey)).IsShiftDown())
	{
//		PrintKey(VK_LSHIFT, true);
	}

	int CharCount = m_KeySequence.GetCount(fStartAfterLastNonPrintKey);
	for(int i = 0; i < CharCount; i++)
	{
		KeyCode _Key(m_KeySequence.Get(i, fStartAfterLastNonPrintKey));
		if(_Key.IsPrintable())
		{
			int VirtualKey = _Key.VirtualKey();
			bool fPress = _Key.IsKeyPressed();
			bool fShiftPressed = _Key.IsShiftDown(); 
			
			if(VirtualKey == VK_BACK && fPress && nBackspacesToSkip > 0)
			{
				nBackspacesToSkip--;
			}
			else
			{
				PrintKey(VirtualKey, fPress, fShiftPressed, hwnd, hKL);
			}
		}
	}
}

void KeyboardSequence::SequenceKeysUp(HWND hwnd, HKL hKL) const
{
	if(m_PressedKeyList.size() > 0)
	{
		for(index_t i = (index_t)m_PressedKeyList.size() - 1; i >= 0; i--)
		{
			PrintKey(m_PressedKeyList.at(i), false, false, hwnd, hKL);
		}
	}
}

void KeyboardSequence::RefreshPressedKeys()
{
	unsigned int nIndex = 0;
	for(unsigned int i = 0; i < m_PressedKeyList.size(); i++)
	{
		int nKey = m_PressedKeyList.at(nIndex);
		if((::GetKeyState(nKey) & 0x80) != 0)
		{
			nIndex++;
		}
		else
		{
			Log(_T("KeyboardSequence::RefreshPressedKeys() - removing key %x\n"), nKey);
			m_PressedKeyList.erase(m_PressedKeyList.begin() + nIndex);
		}
	}
}

bool KeyboardSequence::GetRecentText(std::string &Text, HKL &rhTextKL, bool fStartAfterLastNonPrintKey) const
{
	return m_RecentText.GetText(Text, rhTextKL, fStartAfterLastNonPrintKey);
}

bool KeyboardSequence::GetRecentText(std::wstring &Text, HKL &rhTextKL, bool fStartAfterLastNonPrintKey) const
{
	return m_RecentText.GetText(Text, rhTextKL, fStartAfterLastNonPrintKey);
}

bool KeyboardSequence::GetLatestKeyEvent(int &rLatestKeyEvent)
{
	rLatestKeyEvent = 0;
	if(m_KeySequence.IsEmpty())
		return false;
	rLatestKeyEvent = m_KeySequence.Get(m_KeySequence.GetCount() - 1);
	return true;
}

bool KeyboardSequence::GetPreviouslyErasedText(_tstring &Text, HKL hTargetKL, int AdditionallyErased /*= 0*/) const
{
	// 1. count the number of continuous backspaces in the end of the sequence
	if(m_PrevKeySequence.GetCount() == 0)
		return false;
	int nBackSpaces = 0;
	int nKeyIndex;
	for(nKeyIndex = m_PrevKeySequence.GetCount() - 1; nKeyIndex >= 0; nKeyIndex--)
	{
		KeyCode _KeyCode(m_PrevKeySequence.Get(nKeyIndex));
		if(_KeyCode.IsPrintable() && !_KeyCode.IsShiftKey())
		{
			if(_KeyCode.VirtualKey() != VK_BACK)
				break;
			if(_KeyCode.IsKeyPressed())
			{
				nBackSpaces++;
			}
		}
	}
	nBackSpaces += AdditionallyErased;
	if(nBackSpaces == 0)
		return false;

	// 2. get the slice of the erased sequence
	int nCharacters = 0;
	int nSliceEndIndex = nKeyIndex;
	int nSliceStartIndex;
	for(nSliceStartIndex = nSliceEndIndex; nSliceStartIndex > 0; nSliceStartIndex--)
	{
		KeyCode _KeyCode(m_PrevKeySequence.Get(nSliceStartIndex));
		if(_KeyCode.IsKeyPressed())
		{
			if(_KeyCode.VirtualKey() == VK_BACK)
			{
				nCharacters--;
			}
			else if(!_KeyCode.IsShiftKey())
			{
				nCharacters++;
			}
		}
		if(nCharacters == nBackSpaces)
			break;
	}
	// 3. turn the slice to text
	RecentText SliceText;
	if(!SliceText.Open(m_RecentText.GetTextMaxLength()))
		return false;
	for(int i = nSliceStartIndex; i <= nSliceEndIndex; i++)
	{
		SliceText.Update(m_PrevKeySequence.Get(i), false, hTargetKL);
	}
	HKL hDummyKL;
	if(!SliceText.GetText(Text, hDummyKL))
		return false;

	return true;
}

bool KeyboardSequence::CheckIfAllKeysAreBackspace(int &rnBackspace) const
{
	rnBackspace = 0;

	if(m_KeySequence.GetCount() == 0)
		return false;

	int nBack = 0;
	for(int nKeyIndex = 0; nKeyIndex < m_KeySequence.GetCount(); nKeyIndex++)
	{
		KeyCode _KeyCode(m_KeySequence.Get(nKeyIndex));
		if(_KeyCode.VirtualKey() != VK_BACK && !_KeyCode.IsShiftKey())
			return false;
		if(_KeyCode.VirtualKey() == VK_BACK && _KeyCode.IsKeyPressed())
		{
			nBack++;
		}
	}
	if(nBack == 0)
		return false;

	rnBackspace = nBack;
	return true;
}

void KeyboardSequence::DebugPrint() const
{
	TCHAR Text[1024];
	_stprintf_s(
			Text, sizeof(Text) / sizeof(Text[0]),
			_T("Pressed: ")
			);
	
	int i;
	for(i = 0; i < (int)m_PressedKeyList.size(); i++)
	{
		_stprintf_s(
				Text + _tcslen(Text), sizeof(Text) / sizeof(Text[0]) - _tcslen(Text),
				_T("%02X "),
				m_PressedKeyList.at(i)
				);
	}

	_stprintf_s(
			Text + _tcslen(Text), sizeof(Text) / sizeof(Text[0]) - _tcslen(Text),
			_T(".Sequence: ")
			);

	int FirstToPrint = (m_KeySequence.GetCount() > 10) ? (m_KeySequence.GetCount() - 10) : 0;
	for(i = FirstToPrint; i < m_KeySequence.GetCount(); i++)
	{
		_stprintf_s(
				Text + _tcslen(Text), sizeof(Text) / sizeof(Text[0]) - _tcslen(Text),
				_T("%04X "),
				m_KeySequence.Get(i)
				);
	}
	_stprintf_s(
			Text + _tcslen(Text), sizeof(Text) / sizeof(Text[0]) - _tcslen(Text),
			_T("\n")
			);
	Log(Text);
}

bool KeyboardSequence::LastKeyIsNonPrintable() const
{
	int Value = 0;
	if(!m_KeySequence.PeekLatest(Value))
		return false;
	if(KeyCode(Value).IsPrintable())
		return false;
	return true;
}

bool KeyboardSequence::GetNextOperator(int &rNextOperator, LPTSTR &rRemainingSequence) const
{
	rNextOperator = KEY_OPERATOR_NONE;
	if(!rRemainingSequence || _tcslen(rRemainingSequence) == 0)
		return false;

	if(_tcsncmp(rRemainingSequence, _T("++"), _tcslen(_T("++"))) == 0)
	{
		rRemainingSequence += 2;
		rNextOperator = KEY_OPERATOR_PLUSPLUS;
	}
	else if(_tcsncmp(rRemainingSequence, _T("--"), _tcslen(_T("--"))) == 0)
	{
		rRemainingSequence += 2;
		rNextOperator = KEY_OPERATOR_MINUSMINUS;
	}
	else if(_tcsncmp(rRemainingSequence, _T("+"), _tcslen(_T("+"))) == 0)
	{
		rRemainingSequence += 1;
		rNextOperator = KEY_OPERATOR_PLUS;
	}
	else if(_tcsncmp(rRemainingSequence, _T("-"), _tcslen(_T("-"))) == 0)
	{
		rRemainingSequence += 1;
		rNextOperator = KEY_OPERATOR_MINUS;
	}
	else
	{
		rNextOperator = KEY_OPERATOR_PLUS;
	}
	return true;
}

bool KeyboardSequence::GetNextKey(int &rNextKey, int &rNextOperator, LPTSTR &rRemainingSequence) const
{
	rNextKey = -1;

	if(!GetNextOperator(rNextOperator, rRemainingSequence))
		return false;

	if(rNextOperator == KEY_OPERATOR_MINUSMINUS)
		return true;

	if(!rRemainingSequence || _tcslen(rRemainingSequence) == 0)
		return false;

	// find next delimiter
	LPTSTR NextDelimiter_Plus = _tcschr(rRemainingSequence, _T('+'));
	LPTSTR NextDelimiter_Minue = _tcschr(rRemainingSequence, _T('-'));

	LPTSTR NextDelimiter = NextDelimiter_Plus;
	if(!NextDelimiter || (NextDelimiter_Minue != NULL && NextDelimiter > NextDelimiter_Minue))
	{
		NextDelimiter = NextDelimiter_Minue;
	}

	// get key
	KeyboardMapper VkFinder;
	if(NextDelimiter)
	{
		int nSize = (int)(NextDelimiter - rRemainingSequence);
		VkFinder.GetKey(rRemainingSequence, nSize, rNextKey);
		rRemainingSequence = NextDelimiter;
	}
	else
	{
		size_t nSize = _tcslen(rRemainingSequence);
		VkFinder.GetKey(rRemainingSequence, nSize, rNextKey);
		rRemainingSequence = NULL;
	}
	return true;
}

void KeyboardSequence::AddKey(int VirtualKey, int Operator, int *pComposedKeyList, /*in, out*/int *pListFullness, int ListSizeLimit, HKL hKL, HWND hCurrentWindow, bool fSimulated /*= false*/)
{
	bool fShouldClearSequence;
 	bool fCanPrint = CanPrintNewKey(VirtualKey, Operator != KEY_OPERATOR_MINUS && Operator != KEY_OPERATOR_MINUSMINUS, fShouldClearSequence);

	KeyCode ComposedKey(VirtualKey, Operator, fCanPrint, fSimulated);

	if(fShouldClearSequence)
	{
		if(!m_fReadOnly)
		{
			Clear();
		}
	}
	else
	{
		if((hCurrentWindow != m_hRecentWindow) || fCanPrint && LastKeyIsNonPrintable())
		{
			if(!m_fReadOnly)
			{
				Clear();
			}
		}

		bool fIsPrintableChar = (fCanPrint && Operator != KEY_OPERATOR_MINUS && !KeyboardMapper::IsShiftKey(VirtualKey));
		m_KeySequence.Push(ComposedKey.Get(), !fIsPrintableChar, true);
		if(hKL != NULL)
		{
			m_RecentText.Update(ComposedKey.Get(), !fIsPrintableChar, hKL);
		}
	}
	m_hRecentWindow = hCurrentWindow;

	if(pListFullness != NULL && *pListFullness < ListSizeLimit)
	{
		assert(pComposedKeyList != NULL);
		pComposedKeyList[*pListFullness] = ComposedKey.Get();
		(*pListFullness)++;
	}
}

void KeyboardSequence::SendKeyboardMessage(HWND hwnd, int VirtualKey, bool fPress) // static
{
	HKL hKL = ::GetKeyboardLayout(GetWindowThreadProcessId(hwnd, NULL));

	int Counter = 1;
	int ScanCode = (::MapVirtualKeyEx(VirtualKey, MAPVK_VK_TO_VSC, hKL) & 0xFF);
	int ExtendedKey = KeyboardMapper::IsExtendedKey(VirtualKey) ? 1 : 0;
	int ContextCode = 0;
	int PrevState = fPress ? 0 : 1;
	int TransState = fPress ? 0 : 1;
	LPARAM lParam = 
			Counter +
			(ScanCode << 16) +
			(ExtendedKey << 24) +
			(ContextCode << 29) +
			(PrevState << 30) +
			(TransState << 31);

	::PostMessage(hwnd, fPress ? WM_KEYDOWN : WM_KEYUP, (WPARAM)VirtualKey, lParam);
}

void KeyboardSequence::PrintKey(
							int VirtualKey,
							bool fPress,
							bool fShiftPressed,
							HWND hwnd,
							HKL hKL,
							bool fNoWait /*= false*/
							)
{
	if(hwnd == NULL)
	{
		INPUT KbInput;
		KbInput.type = INPUT_KEYBOARD;
		KbInput.ki.wVk = (WORD)VirtualKey;
		KbInput.ki.wScan = 0;
		KbInput.ki.dwFlags = fPress ? 0 : KEYEVENTF_KEYUP;
		KbInput.ki.time = 0;
		KbInput.ki.dwExtraInfo = NULL;
		::SendInput(1, &KbInput, sizeof(KbInput));
		Sleep(10); // give time for the editor to print the key. Otherwise, additional operations such as language change will happen before time
	}
	else
	{
		int Counter = 1;
		int ScanCode = (::MapVirtualKeyEx(VirtualKey, MAPVK_VK_TO_VSC, hKL) & 0xFF);
		int ExtendedKey = KeyboardMapper::IsExtendedKey(VirtualKey) ? 1 : 0;
		int ContextCode = 0; // Eli Dori - assuming not using ALT button
		int PrevState = fPress ? 0 : 1;
		int TransState = fPress ? 0 : 1;
		LPARAM lParam = 
				Counter +
				(ScanCode << 16) +
				(ExtendedKey << 24) +
				(ContextCode << 29) +
				(PrevState << 30) +
				(TransState << 31);

		if(VirtualKey == VK_BACK)
		{
			::PostMessage(hwnd, fPress ? WM_KEYDOWN : WM_KEYUP, (WPARAM)VirtualKey, lParam);
			if(fPress)
			{
				// this is to let the application perform the TranslateMessage. The reason why WM_CHAR is not sent
				// directly in this case, is that not every application uses it (Excel, VisualStudio), and WM_KEYDOWN is common to all application
				if(!fNoWait) // no wait is used when a batch of backspaces is transmitted, and only then the sleep is needed
				{
					Sleep(10);
				}
			}
		}
		else if(fPress)
		{
			BYTE KeyState[256];
			memset(KeyState, 0, sizeof(KeyState));
			KeyState[VK_LSHIFT] = (fShiftPressed ? 0x80 : 0x00);
			KeyState[VK_SHIFT] = (fShiftPressed ? 0x80 : 0x00);

			WCHAR CharText[10];
			int nChars = ::ToUnicodeEx(
							VirtualKey,
							::MapVirtualKeyEx(VirtualKey, MAPVK_VK_TO_VSC, hKL),
							KeyState,
							CharText,
							sizeof(CharText)/sizeof(CharText[0]),
							0,
							hKL
							);
			assert(nChars < 2);
			if(nChars == 1)
			{
				::PostMessageW(hwnd, WM_CHAR, (WPARAM)CharText[0], lParam);
			}
		}
	}
}

int KeyboardSequence::FindIndex(int VirtualKey) const
{
	for(unsigned int i = 0; i < m_PressedKeyList.size(); i++)
	{
		if(VirtualKey == m_PressedKeyList.at(i))
			return i;
	}
	return -1;
}

bool KeyboardSequence::CanPrintNewKey(int VirtualKey, bool fPressed, bool &rfShouldClearSequence) const
{
	rfShouldClearSequence = false;
	bool fCanPrint = KeyboardMapper::CanPrintKey(VirtualKey, rfShouldClearSequence) && !m_fInControlKeySequence;
	if(fCanPrint && !fPressed)
	{
		// try to see if the new key is unpressed after it was pressed with control keys - in such case it is not printable
		int PrevKey;
		if(m_KeySequence.PeekLatest(PrevKey))
		{
			KeyCode PrevKeyCode(PrevKey);
			if(!PrevKeyCode.IsPrintable())
			{
				fCanPrint = false;
			}
		}
	}
	return fCanPrint;
}
