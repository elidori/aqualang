#ifndef _KEYBOARD_SEQUENCE_CLIENT_HEADER_
#define _KEYBOARD_SEQUENCE_CLIENT_HEADER_

#include "KeyboardSequence.h"
#include "IKeyboardEvent.h"
#include "WinHookResult.h"
#include "UtilityWindow.h"
#include "UtilityWindowParams.h"
#include "IKeyboardEventCallback.h"

class KeyboardSequenceEvent;
class FocusInfo;
class LanguageDetector;
class WindowItemDataBase;

using namespace WinHook;

class KeyboardSequenceClient :
	public IWindowMessageListener,
	public IKeyboardEventCallback
{
private:
	KeyboardSequenceClient(
					const KeyboardSequence &rSequence,
					UtilityWindow &rUtilityWindow,
					KeyboardSequenceEvent &rKeyboardSequenceEvent,
					FocusInfo &rFocusInfo,
					const WindowItemDataBase &rWindowsDatabase,
					const LanguageDetector &rLangaugeDetector,
					bool fBackgroundClient
					);
	~KeyboardSequenceClient();

	Result Open();
	void Close();
public:
	static Result Create(
					KeyboardSequenceClient* &rpClient,
					const KeyboardSequence &rSequence,
					UtilityWindow &rUtilityWindow,
					KeyboardSequenceEvent &rKeyboardSequenceEvent,
					FocusInfo &rFocusInfo,
					const WindowItemDataBase &rWindowsDatabase,
					const LanguageDetector &rLangaugeDetector,
					bool fBackgroundClient
					);

	bool AddListener(IKeyboardEvent &rListener);

	bool IsEmpty() const;

	bool IsBackgroundClient() const	{ return m_fBackgroundClient; }

	bool IsSimilar(const KeyboardSequence &rSequence, bool fBackground, bool &rfIsEqual);
	bool DispatchIfMatch(const KeyboardSequence &rCurrentSequence, bool &rfFoundMatch);

	bool RemoveListener(const IKeyboardEvent &rListener);

	void Release();
private:
	// IWindowMessageListener pure virtuals {
	virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// }

	// IKeyboardEventCallback pure virtuals {
	virtual void EventOperationEnded(bool fComplete);
	// }

	void DispatchClients(HWND hCurrentWindow, const FocusInfo &rFocusInfo, const LanguageDetector &rLangaugeDetector);
	void BuildDispatchList(std::vector<IKeyboardEvent *> &rDispatchList, KeyboardEventInfo &Info);
private:
	typedef std::map<int, IKeyboardEvent *> KbListenerMap;
	KbListenerMap m_ListenerMap;

	KeyboardSequence m_ReferenceSequence;

	UtilityWindow &m_rUtilityWindow;
	KeyboardSequenceEvent &m_rKeyboardSequenceEvent;
	FocusInfo &m_rFocusInfo;
	const WindowItemDataBase &m_rWindowsDatabase;
	const LanguageDetector &m_rLangaugeDetector;

	const bool m_fBackgroundClient;
};

#endif // _KEYBOARD_SEQUENCE_CLIENT_HEADER_