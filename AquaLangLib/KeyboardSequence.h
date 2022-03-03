#ifndef _KEYBOARD_SEQUENCE_HEADER_
#define _KEYBOARD_SEQUENCE_HEADER_

#include "Fifo.h"
#include "TimeHelpers.h"
#include "RecentText.h"

class KeyboardSequence
{
public:
	KeyboardSequence(bool fReadOnly = false);
	KeyboardSequence(const KeyboardSequence &r);
	virtual ~KeyboardSequence();

	bool Set(const KeyboardSequence &r);

	bool Open(int MaxKeySequenceSize, int MaxRecentTextSize, LPCTSTR Sequence = NULL, bool fBackground = false);
	void Close();

	bool KeyEvent(int VirtualKey, int Operator, /*in*/int *pComposedKeyList, /*in, out*/int *pListSize, HKL hKL, HWND hCurrentWindow);
	bool UpdateLatestTextLanguage(HKL hKL, bool fStartAfterLastNonPrintKey);

	bool HasOnlyShifts() const;

	bool IsEqual(const KeyboardSequence &r) const;
	bool IsMatching(const KeyboardSequence &r) const;
	bool IsContaining(const KeyboardSequence &r) const;

	void Clear();
	void FocusChanged(HWND hFocusWindow);
	void ClearTextHistory();

	void MarkCurrentPosition();

	void ClearReferenceSequence(const KeyboardSequence &r);
	int GetCharacterCount(int &rnBackspace, int &rnDeletionMaxDepth, bool fStartAfterLastNonPrintKey) const;

	int BackspaceSequence(HWND hwnd, HKL hKL, bool fStartAfterLastNonPrintKey);
	void ReplaySequence(int BackspacesToIgnore, HWND hwnd, HKL hKL, bool fStartAfterLastNonPrintKey);

	void SequenceKeysUp(HWND hwnd, HKL hKL) const;

	void RefreshPressedKeys();

	bool GetRecentText(std::string &Text, HKL &rhTextKL, bool fStartAfterLastNonPrintKey) const;
	bool GetRecentText(std::wstring &Text, HKL &rhTextKL, bool fStartAfterLastNonPrintKey) const;

	bool GetLatestKeyEvent(int &rLatestKeyEvent);

	bool GetPreviouslyErasedText(_tstring &Text, HKL hTargetKL, int AdditionallyErased = 0) const;
	bool CheckIfAllKeysAreBackspace(int &rnBackspace) const;

	void DebugPrint() const;

	static void SendKeyboardMessage(HWND hwnd, int VirtualKey, bool fPress);
	static void PrintKey(
					int VirtualKey, 
					bool fPress,
					bool fShiftPressed,
					HWND hwnd,
					HKL hKL,
					bool fNoWait = false
					);
private:
	bool Init(LPCTSTR Sequence, bool fBackground);
	void Term();

	bool LastKeyIsNonPrintable() const;

	bool GetNextOperator(int &rNextOperator, LPTSTR &rRemainingSequence) const;
	bool GetNextKey(int &rNextKey, int &rNextOperator, LPTSTR &rRemainingSequence) const;

	bool UpdateLatestText(int Key, HKL hKL);

	void AddKey(int VirtualKey, int Operator, int *pComposedKeyList, /*in, out*/int *pListFullness, int ListSizeLimit, HKL hKL, HWND hCurrentWindow, bool fSimulated = false);
	int FindIndex(int VirtualKey) const;

	bool CanPrintNewKey(int VirtualKey, bool fPressed, bool &rfShouldClearSequence) const;
private:
	const bool m_fReadOnly;
	bool m_fHasWildcards;

	Fifo<int> m_KeySequence;
	RecentText m_RecentText;

	Fifo<int> m_PrevKeySequence; // saves the latest sequence before control\alt were pressed

	std::vector<int> m_PressedKeyList;

	bool m_fInControlKeySequence;

	Timer m_LatestKeyPressedTime;
	HWND m_hRecentWindow;
};

#endif // _KEYBOARD_SEQUENCE_HEADER_
