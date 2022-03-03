#ifndef _LANGUAGE_DETECTOR_HEADER_
#define _LANGUAGE_DETECTOR_HEADER_

#include "KeyboardMapper.h"

class GuessLanguageHandler;

class LanguageDetector
{
public:
	LanguageDetector();
	virtual ~LanguageDetector();

	bool Open(GuessLanguageHandler *pGuessLanguageHandler);
	void Close();
	
	bool Detect(LPCWSTR Text, HKL &rhKL) const;
	static void ConvertText(LPCSTR InputString, HKL hSourceKL, HKL hTargetKL, std::string &OutputString, int CodePage);
	static void ConvertText(LPCSTR InputString, HKL hSourceKL, HKL hTargetKL, std::wstring &wOutputString, int CodePage);
	static void ConvertText(LPCWSTR wInputString, HKL hSourceKL, HKL hTargetKL, std::string &OutputString, int CodePage);
	static void ConvertText(LPCWSTR wInputString, HKL hSourceKL, HKL hTargetKL, std::wstring &wOutputString, int CodePage);
	void ToggleStringLanguage(LPCWSTR InputString, std::wstring &OutputString) const;
private:
	static WCHAR ConvertCharacter(WCHAR InputChar, BYTE* pKeyState, HKL hSourceKL, HKL hTargetKL);
	WCHAR ToggleCharacterLanguage(WCHAR InputChar, BYTE* pKeyState, int &LatestCharKLIndex) const;
	static bool GetCharacterFromKey(WCHAR &Char, int VirtualKey, HKL hTargetKL, const BYTE* pKeyState);
	int FindDefaultKLIndex(LPCWSTR InputString) const;
private:
	GuessLanguageHandler *m_pGuessLanguageHandler;
	KeyboardMapper m_KeyboardMapper;
};

#endif // _LANGUAGE_DETECTOR_HEADER_