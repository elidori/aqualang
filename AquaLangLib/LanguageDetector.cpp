#include "Pch.h"
#include "LanguageDetector.h"
#include "ClipboardAccess.h"
#include "ComHelper.h"
#include "StringConvert.h"
#include "GuessLanguageHandler.h"

LanguageDetector::LanguageDetector()
	: m_pGuessLanguageHandler(NULL)
{
}

LanguageDetector::~LanguageDetector()
{
	Close();
}

bool LanguageDetector::Open(GuessLanguageHandler *pGuessLanguageHandler)
{
	m_pGuessLanguageHandler = pGuessLanguageHandler;
	return true;
}

void LanguageDetector::Close()
{
	m_pGuessLanguageHandler = NULL;
}

bool LanguageDetector::Detect(LPCWSTR Text, HKL &rhKL) const
{
	rhKL = NULL;

	if(Text == NULL)
		return false;
	if(m_KeyboardMapper.GetLayoutCount() == 0)
		return false;

	if(m_pGuessLanguageHandler != NULL)
	{
		if(m_pGuessLanguageHandler->GuessLanguage(Text, rhKL))
			return true;
	}

	// inialize array of counters, each corresponding to another language
	// each count refers to the number of detected characters in this language
	typedef std::map<HKL, int> LanguageCountMap;
	LanguageCountMap _LanguageCountMap;

	for(int LayoutIndex = 0; LayoutIndex < m_KeyboardMapper.GetLayoutCount(); LayoutIndex++)
	{
		_LanguageCountMap.insert(LanguageCountMap::value_type(m_KeyboardMapper.GetLayout(LayoutIndex), 0));
	}

	// go over the text characters and increment the matching languages counters
	size_t nLength = wcslen(Text);
	for(index_t i = 0; i < (index_t)nLength; i++)
	{
		LanguageCountMap::iterator Iterator = _LanguageCountMap.begin();
		for(; Iterator != _LanguageCountMap.end(); Iterator++)
		{
			HKL hKL = (*Iterator).first;
			int Key = ::VkKeyScanExW(Text[i], hKL);
			if(Key != -1)
			{
				((*Iterator).second)++;
			}
		}
	}

	// pick the language with the largest counter
	int LargestCounter = 0;
	LanguageCountMap::iterator Iterator = _LanguageCountMap.begin();
	for(; Iterator != _LanguageCountMap.end(); Iterator++)
	{
		if(rhKL == NULL || LargestCounter < (*Iterator).second)
		{
			LargestCounter = (*Iterator).second;
			rhKL = (*Iterator).first;
		}
	}
	return true;
}

int LanguageDetector::FindDefaultKLIndex(LPCWSTR InputString) const
{
	std::vector<int> LanguageCounterList;

	int i;
	for(i = 0; i < m_KeyboardMapper.GetLayoutCount(); i++)
	{
		LanguageCounterList.push_back(0);
	}

	size_t nLength = wcslen(InputString);
	for(index_t CharIndex = 0; CharIndex < (index_t)nLength; CharIndex++)
	{
		WCHAR InputChar = InputString[CharIndex];

		for(i = 0; i < m_KeyboardMapper.GetLayoutCount(); i++)
		{
			HKL hKL = m_KeyboardMapper.GetLayout(i);
			int Key = ::VkKeyScanExW(InputChar, hKL);
			if(Key != -1)
			{
				(LanguageCounterList.at(i))++;
			}
		}
	}

	int DefaultIndex = -1;
	int MaxCount = -1;
	for(i = 0; i < m_KeyboardMapper.GetLayoutCount(); i++)
	{
		if(LanguageCounterList.at(i) > MaxCount)
		{
			DefaultIndex = i;
			MaxCount = LanguageCounterList.at(i);
		}
	}
	return DefaultIndex;
}

void LanguageDetector::ConvertText(LPCSTR InputString, HKL hSourceKL, HKL hTargetKL, std::string &OutputString, int CodePage)
{
	std::wstring wInputString;
	SetString(wInputString, InputString, (size_t)-1, CodePage);

	std::wstring wOutputString;
	ConvertText(wInputString.c_str(), hSourceKL, hTargetKL, wOutputString, 0);
	SetString(OutputString, wOutputString.c_str(), wOutputString.length(), CodePage);
}

void LanguageDetector::ConvertText(LPCSTR InputString, HKL hSourceKL, HKL hTargetKL, std::wstring &wOutputString, int CodePage)
{
	std::wstring wInputString;
	SetString(wInputString, InputString, (size_t)-1, CodePage);

	ConvertText(wInputString.c_str(), hSourceKL, hTargetKL, wOutputString, CodePage);
}

void LanguageDetector::ConvertText(LPCWSTR wInputString, HKL hSourceKL, HKL hTargetKL, std::string &OutputString, int CodePage)
{
	std::wstring wOutputString;
	ConvertText(wInputString, hSourceKL, hTargetKL, wOutputString, CodePage);
	SetString(OutputString, wOutputString.c_str(), wOutputString.length(), CodePage);
}

void LanguageDetector::ConvertText(LPCWSTR wInputString, HKL hSourceKL, HKL hTargetKL, std::wstring &wOutputString, int UNUSED(CodePage))
{
	if(wInputString == NULL)
		return;

	BYTE KeyState[256];
	memset(KeyState, 0, sizeof(KeyState));

	size_t nLength = wcslen(wInputString);
	for(index_t i = 0; i < (index_t)nLength; i++)
	{
		WCHAR Char = ConvertCharacter(wInputString[i], KeyState, hSourceKL, hTargetKL);
		wOutputString += Char;
	}
}

WCHAR LanguageDetector::ConvertCharacter(WCHAR InputChar, BYTE* pKeyState, HKL hSourceKL, HKL hTargetKL)
{
	int Key = ::VkKeyScanExW(InputChar, hSourceKL);
	if(Key == -1)
		return InputChar;

	int VirtualKey = LOBYTE(Key);
	bool fShiftIsPressed = ((HIBYTE(Key) & 0x01) != 0);
	bool fControlIsPressed = ((HIBYTE(Key) & 0x02) != 0);
	bool fAltIsPressed = ((HIBYTE(Key) & 0x04) != 0);

	bool fIgnoreShifts = (VirtualKey >= 'A' && VirtualKey <= 'Z');
	pKeyState[VK_LSHIFT] = (fShiftIsPressed && !fIgnoreShifts ? 0x80 : 0x00);
	pKeyState[VK_SHIFT] = (fShiftIsPressed && !fIgnoreShifts ? 0x80 : 0x00);

	pKeyState[VK_LCONTROL] = (fControlIsPressed ? 0x80 : 0x00);
	pKeyState[VK_CONTROL] = (fControlIsPressed ? 0x80 : 0x00);

	pKeyState[VK_LMENU] = (fAltIsPressed ? 0x80 : 0x00);
	pKeyState[VK_MENU] = (fAltIsPressed ? 0x80 : 0x00);

	WCHAR OutputChar;
	if(!GetCharacterFromKey(OutputChar, VirtualKey, hTargetKL, pKeyState))
	{
		return InputChar;
	}
	return OutputChar;
}

void LanguageDetector::ToggleStringLanguage(LPCWSTR InputString, std::wstring &OutputString) const
{
	if(InputString == NULL)
		return;
	if(m_KeyboardMapper.GetLayoutCount() == 0)
		return;

	int LatestCharKLIndex = FindDefaultKLIndex(InputString);

	BYTE KeyState[256];
	memset(KeyState, 0, sizeof(KeyState));

	size_t nLength = wcslen(InputString);
	for(index_t i = 0; i < (index_t)nLength; i++)
	{
		WCHAR Char = ToggleCharacterLanguage(InputString[i], KeyState, LatestCharKLIndex);
		OutputString += Char;
	}
}

WCHAR LanguageDetector::ToggleCharacterLanguage(WCHAR InputChar, BYTE* pKeyState, int &LatestCharKLIndex) const
{
	// first guess the character language and get the virtual key
	int CharKLIndex = -1;
	int VirtualKey = -1;
	bool fShiftIsPressed = false;
	bool fControlIsPressed = false;
	bool fAltIsPressed = false;

	// search to see the first language where the character can be present in it. Start from the language detected in the previous char
	for(int i = 0; i < m_KeyboardMapper.GetLayoutCount(); i++)
	{
		int KLIndex = (LatestCharKLIndex + i) % m_KeyboardMapper.GetLayoutCount();

		HKL hKL = m_KeyboardMapper.GetLayout(KLIndex);
		int Key = ::VkKeyScanExW(InputChar, hKL);
		if(Key != -1)
		{
			CharKLIndex = KLIndex;
			VirtualKey = LOBYTE(Key);
			fShiftIsPressed = ((HIBYTE(Key) & 0x01) != 0);
			fControlIsPressed = ((HIBYTE(Key) & 0x02) != 0);
			fAltIsPressed = ((HIBYTE(Key) & 0x04) != 0);

			LatestCharKLIndex = KLIndex;
			break;
		}
	}
	if(CharKLIndex == -1)
	{
		// character is not a part of the installed languages in the system - don't translate
		// Eli Dori: Optionally, it can be decided to translate to the first language in the list
		return InputChar;
	}

	// create a translated character
	HKL hTargetKL = m_KeyboardMapper.GetLayout((CharKLIndex + 1) % m_KeyboardMapper.GetLayoutCount());

	bool fIgnoreShifts = (VirtualKey >= 'A' && VirtualKey <= 'Z');
	pKeyState[VK_LSHIFT] = (fShiftIsPressed && !fIgnoreShifts ? 0x80 : 0x00);
	pKeyState[VK_SHIFT] = (fShiftIsPressed && !fIgnoreShifts ? 0x80 : 0x00);

	pKeyState[VK_LCONTROL] = (fControlIsPressed ? 0x80 : 0x00);
	pKeyState[VK_CONTROL] = (fControlIsPressed ? 0x80 : 0x00);

	pKeyState[VK_LMENU] = (fAltIsPressed ? 0x80 : 0x00);
	pKeyState[VK_MENU] = (fAltIsPressed ? 0x80 : 0x00);

	WCHAR OutputChar;
	if(!GetCharacterFromKey(OutputChar, VirtualKey, hTargetKL, pKeyState))
	{
		return InputChar;
	}
	return OutputChar;
}

bool LanguageDetector::GetCharacterFromKey(WCHAR &Char, int VirtualKey, HKL hTargetKL, const BYTE* pKeyState)
{
	WCHAR CharText[10];
	int nChars = ::ToUnicodeEx(
					VirtualKey,
					::MapVirtualKeyEx(VirtualKey, MAPVK_VK_TO_VSC, hTargetKL),
					pKeyState,
					CharText,
					sizeof(CharText)/sizeof(CharText[0]),
					0,
					hTargetKL
					);
	assert(nChars < 2);
	if(nChars != 1)
		return false;
	Char = CharText[0];
	return true;
}
