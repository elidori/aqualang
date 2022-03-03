#ifndef _KEYBOARD_SEQUENCE_EVENT_HEADER_
#define _KEYBOARD_SEQUENCE_EVENT_HEADER_

#include "IKeyboardEvent.h"
#include "WinHookResult.h"
#include "WinHookListener.h"
#include "WinHookPrioritizedListenerList.h"
#include "KeyboardSequence.h"
#include "StickyKeysEnabler.h"
#include "KeyboardSequenceClient.h"
#include "LatestUIEvent.h"

class UtilityWindow;
class FocusInfo;
class LanguageDetector;
class AsyncHookDispatcher;
class IFocusChangeListener;
class WindowItemDataBase;
class DialogObject;
struct AquaLangStatistics;

using namespace WinHook;

class KeyboardSequenceEvent :
	public IKeyboardListener,
	public IMouseListener
{
public:
	KeyboardSequenceEvent(
				UtilityWindow &rUtilityWindow,
				FocusInfo &rFocusInfo,
				const WindowItemDataBase &rWindowsDatabase,
				const LanguageDetector &rLangaugeDetector,
				AsyncHookDispatcher &rAsyncHookDispatcher
				);
	virtual ~KeyboardSequenceEvent();

	Result Register(LPCTSTR Sequence, IKeyboardEvent &rListener, bool fBackground);
	Result Unregister(const IKeyboardEvent &rListener);

	Result RegisterOnFocusChange(IFocusChangeListener &rListener);
	Result UnregisterOnFocusChange(IFocusChangeListener &rListener);

	bool IsEmpty(bool fStartAfterLastNonPrintKey) const;

	void BlockHooks(bool fBlock);
	bool AreHooksBlocked() const	{ return m_fBlockingHooks; }
	void ClearReferenceSequenceKeys(const KeyboardSequence &rReferenceSequence);

	int BackspaceSequence(HKL hKL, bool fStartAfterLastNonPrintKey);
	void ReplayLatestKeys(int BackspacesToIgnore, HKL hKL, bool fStartAfterLastNonPrintKey);

	void DisableStickyKeysUntilNextStroke();

	bool UpdateRecentTextLanguage(HKL hKL, bool fStartAfterLastNonPrintKey);
 	bool GetRecentText(std::string &Text, HKL &rhTextKL, bool fStartAfterLastNonPrintKey) const;
	bool GetRecentText(std::wstring &Text, HKL &rhTextKL, bool fStartAfterLastNonPrintKey) const;

	bool GetPreviouslyErasedText(_tstring &Text, HKL hTargetKL, int AdditionallyErased = 0) const;
	bool CheckIfAllKeysAreBackspace(int &rnBackspace) const;

	void ClearTextHistory();
	void MarkCurrentPosition();

	bool IsInPrintMode(__int64 &TimePassedSinceLastPrint) const;
	bool GetLatestEvent(__int64 &TimePassedSinceEvent, UINT &uMsg, int &KbKey) const;

	void IgnoreMouseEvents(const DialogObject &rDialog, bool fIgnore);

	bool FillStatistics(AquaLangStatistics &rStatistics);
private:
	Result GetSequenceClient(KeyboardSequenceClient* &rpClient, LPCTSTR Sequence, bool fBackground);
	// IKeyboardListener pure virtuals {
	virtual LRESULT OnKeyboardHookEvent(UINT uMsg, int Code);
	// }

	// IMouseListener pure virtuals {
	virtual LRESULT OnMouseHookEvent(HWND hwnd, UINT uMsg, int x, int y);
	// }

	Result OpenHandles();
	void CloseHandles();

	bool UpdateFocus();
	void NotifyOnFocusChange(HWND hFocusWindow);

	void Dispatch();

	bool ShouldIgnoreMouseEvent(int x, int y) const;
private:
	UtilityWindow &m_rUtilityWindow;
	FocusInfo &m_rFocusInfo;
	const WindowItemDataBase &m_rWindowsDatabase;
	const LanguageDetector &m_rLangaugeDetector;
	AsyncHookDispatcher &m_rAsyncHookDispatcher;

	PrioritizedListenerList<KeyboardSequenceClient> m_SequenceLisenerList;

	KeyboardSequence m_CurrentSequence;

	bool m_fBlockingHooks;

	StickyKeysEnabler m_StickyKeysEnabler;

	typedef std::map<int, IFocusChangeListener *> FocusListenerMap;
	FocusListenerMap m_FocusListenerMap;

	typedef std::map<int, const DialogObject *> IgnoredMouseEventWindowMap;
	IgnoredMouseEventWindowMap m_IgnoredMouseEventWindowMap;

	LatestUIEvent m_LatestUIEvent;

	__int64 m_nKeyboardEvents;
	Timer m_TimeOfEventsBlock;
	__int64 m_nBlockedKeyboardEvents;
};

#endif // _KEYBOARD_SEQUENCE_EVENT_HEADER_
