#include "Pch.h"
#include "KeyboardSequenceEvent.h"
#include "StringConvert.h"
#include "SyncHelpers.h"
#include "KeyboardMapper.h"
#include "KeyboardDefinitions.h"
#include "FocusInfo.h"
#include "StringSizeLimits.h"
#include "Log.h"
#include "AquaLangStatistics.h"
#include "AsyncHookDispatcher.h"
#include "IFocusChangeListener.h"
#include "AquaLangWaterMark.h"
#include "DialogObject.h"

#define MAX_COMPOSED_KEYS_IN_ONE_EVENT 3 // in case of double click
#define MAX_BLOCKED_EVENTS_DURATION_MSEC 10000

KeyboardSequenceEvent::KeyboardSequenceEvent(
									UtilityWindow &rUtilityWindow,
									FocusInfo &rFocusInfo,
									const WindowItemDataBase &rWindowsDatabase,
									const LanguageDetector &rLangaugeDetector,
									AsyncHookDispatcher &rAsyncHookDispatcher
									)
	: m_rUtilityWindow(rUtilityWindow),
	m_rFocusInfo(rFocusInfo),
	m_rWindowsDatabase(rWindowsDatabase),
	m_rLangaugeDetector(rLangaugeDetector),
	m_rAsyncHookDispatcher(rAsyncHookDispatcher),
	m_CurrentSequence(),
	m_fBlockingHooks(false),
	m_nKeyboardEvents(0),
	m_nBlockedKeyboardEvents(0)
{
}

KeyboardSequenceEvent::~KeyboardSequenceEvent()
{
	CloseHandles();
	m_CurrentSequence.Close();
	m_StickyKeysEnabler.Close();
}

Result KeyboardSequenceEvent::Register(LPCTSTR Sequence, IKeyboardEvent &rListener, bool fBackground)
{
	bool fListWasEmpty = (m_SequenceLisenerList.GetCount() == 0);

	KeyboardSequenceClient *pClient = NULL;
	Result res = GetSequenceClient(pClient, Sequence, fBackground);
	if(res != Result_Success)
		return res;

	if(!pClient->AddListener(rListener))
	{
		if(pClient->IsEmpty())
		{
			m_SequenceLisenerList.Remove(*pClient);
			pClient->Release();
		}
		return Result_InvalidParameter;
	}

	if(fListWasEmpty)
	{
		res = OpenHandles();
		if(res != Result_Success)
			return res;
		if(!m_CurrentSequence.Open(KEY_SEQUENCE_MAX_COUNT, RECENT_TEXT_MAX_LENGTH))
			return Result_MemAllocFailed;
	}
	return Result_Success;
}

Result KeyboardSequenceEvent::Unregister(const IKeyboardEvent &rListener)
{
	// remove all associated objects
	PrioritizedListenerList<KeyboardSequenceClient> Temp;
	
	for(index_t i = 0; i < (index_t)m_SequenceLisenerList.GetCount(); i++)
	{
		KeyboardSequenceClient *pClient = NULL;
		Result res = m_SequenceLisenerList.Get(i, pClient);
		if(res == Result_Success)
		{
			pClient->RemoveListener(rListener);
			if(pClient->IsEmpty())
			{
				pClient->Release();
			}
			else
			{
				Temp.Add(*pClient);
			}
		}
	}

	m_SequenceLisenerList.Set(Temp);
	if(m_SequenceLisenerList.GetCount() == 0)
	{
		CloseHandles();
		m_CurrentSequence.Close();
		m_StickyKeysEnabler.Close();
	}
	return Result_Success;
}

Result KeyboardSequenceEvent::RegisterOnFocusChange(IFocusChangeListener &rListener)
{
	if(m_FocusListenerMap.find((int)&rListener) != m_FocusListenerMap.end())
		return Result_InvalidParameter;
	m_FocusListenerMap.insert(FocusListenerMap::value_type((int)&rListener, &rListener));
	return Result_Success;
}

Result KeyboardSequenceEvent::UnregisterOnFocusChange(IFocusChangeListener &rListener)
{
	FocusListenerMap::iterator Iterator  = m_FocusListenerMap.find((int)&rListener);
	if(Iterator == m_FocusListenerMap.end())
		return Result_InvalidParameter;
	m_FocusListenerMap.erase(Iterator);
	return Result_Success;
}


bool KeyboardSequenceEvent::IsEmpty(bool fStartAfterLastNonPrintKey) const
{
	int Backspaces = 0;
	int nDeletionMaxDepth = 0;
	return (m_CurrentSequence.GetCharacterCount(Backspaces, nDeletionMaxDepth, fStartAfterLastNonPrintKey) == 0);
}

void KeyboardSequenceEvent::BlockHooks(bool fBlock)
{
	m_fBlockingHooks = fBlock;
	if(fBlock)
	{
		m_TimeOfEventsBlock.Restart();
		m_LatestUIEvent.Clear();
	}
}

void KeyboardSequenceEvent::ClearReferenceSequenceKeys(const KeyboardSequence &rReferenceSequence)
{
	m_CurrentSequence.ClearReferenceSequence(rReferenceSequence);
}

int KeyboardSequenceEvent::BackspaceSequence(HKL hKL, bool fStartAfterLastNonPrintKey)
{
	return m_CurrentSequence.BackspaceSequence(m_rFocusInfo.QueryCurrentFocusWindow(), hKL, fStartAfterLastNonPrintKey);
}

void KeyboardSequenceEvent::ReplayLatestKeys(int BackspacesToIgnore, HKL hKL, bool fStartAfterLastNonPrintKey)
{
	m_CurrentSequence.ReplaySequence(BackspacesToIgnore, m_rFocusInfo.QueryCurrentFocusWindow(), hKL, fStartAfterLastNonPrintKey);
}

void KeyboardSequenceEvent::DisableStickyKeysUntilNextStroke()
{
	m_StickyKeysEnabler.Enable(false);
}

bool KeyboardSequenceEvent::UpdateRecentTextLanguage(HKL hKL, bool fStartAfterLastNonPrintKey)
{
	return m_CurrentSequence.UpdateLatestTextLanguage(hKL, fStartAfterLastNonPrintKey);
}

void KeyboardSequenceEvent::ClearTextHistory()
{
	m_CurrentSequence.ClearTextHistory();
}

void KeyboardSequenceEvent::MarkCurrentPosition()
{
	m_CurrentSequence.MarkCurrentPosition();
}

bool KeyboardSequenceEvent::GetRecentText(std::string &Text, HKL &rhTextKL, bool fStartAfterLastNonPrintKey) const
{
	return m_CurrentSequence.GetRecentText(Text, rhTextKL, fStartAfterLastNonPrintKey);
}

bool KeyboardSequenceEvent::GetRecentText(std::wstring &Text, HKL &rhTextKL, bool fStartAfterLastNonPrintKey) const
{
	return m_CurrentSequence.GetRecentText(Text, rhTextKL, fStartAfterLastNonPrintKey);
}

bool KeyboardSequenceEvent::GetPreviouslyErasedText(_tstring &Text, HKL hTargetKL, int AdditionallyErased /*= 0*/) const
{
	return m_CurrentSequence.GetPreviouslyErasedText(Text, hTargetKL, AdditionallyErased);
}

bool KeyboardSequenceEvent::CheckIfAllKeysAreBackspace(int &rnBackspace) const
{
	return m_CurrentSequence.CheckIfAllKeysAreBackspace(rnBackspace);
}

bool KeyboardSequenceEvent::IsInPrintMode(__int64 &TimePassedSinceLastPrint) const
{
	return m_LatestUIEvent.IsInPrintMode(TimePassedSinceLastPrint);
}

bool KeyboardSequenceEvent::GetLatestEvent(__int64 &TimePassedSinceEvent, UINT &uMsg, int &KbKey) const
{
	return m_LatestUIEvent.GetLatestEvent(TimePassedSinceEvent, uMsg, KbKey);
}

void KeyboardSequenceEvent::IgnoreMouseEvents(const DialogObject &rDialog, bool fIgnore)
{
	if(fIgnore)
	{
		m_IgnoredMouseEventWindowMap[(int)&rDialog] = &rDialog;
	}
	else
	{
		IgnoredMouseEventWindowMap::iterator Iter = m_IgnoredMouseEventWindowMap.find((int)&rDialog);
		if(Iter != m_IgnoredMouseEventWindowMap.end())
		{
			m_IgnoredMouseEventWindowMap.erase(Iter);
		}
	}
}

bool KeyboardSequenceEvent::FillStatistics(AquaLangStatistics &rStatistics)
{
	rStatistics.KeyboardEvents = m_nKeyboardEvents;
	rStatistics.BlockedKeyboardEvents = m_nBlockedKeyboardEvents;
	return true;
}

Result KeyboardSequenceEvent::GetSequenceClient(KeyboardSequenceClient* &rpClient, LPCTSTR Sequence, bool fBackground)
{
	rpClient = NULL;

	KeyboardSequence NewSequence(true);
	if(!NewSequence.Open(KEY_SEQUENCE_MAX_COUNT, RECENT_TEXT_MAX_LENGTH, Sequence, fBackground))
		return Result_InvalidParameter;
	
	Result res = Result_Success;
	KeyboardSequenceClient *pClient = NULL;
	for(index_t i = 0; i < (index_t)m_SequenceLisenerList.GetCount(); i++)
	{
		res = m_SequenceLisenerList.Get(i, pClient);
		if(res == Result_Success)
		{
			bool fIsEqual = false;
			if(pClient->IsSimilar(NewSequence, fBackground, fIsEqual))
			{
				if(!fIsEqual)
					return Result_InvalidParameter;
				break; // found equal sequence. Use it instead of creating a new one
			}
		}
		pClient = NULL;
	}
	if(!pClient)
	{
		res = KeyboardSequenceClient::Create(pClient, NewSequence, m_rUtilityWindow, *this, m_rFocusInfo, m_rWindowsDatabase, m_rLangaugeDetector, fBackground);
		if(res != Result_Success)
			return res;

		if(fBackground)
		{
			res = m_SequenceLisenerList.AddLast(*pClient);
		}
		else
		{
			res = m_SequenceLisenerList.Add(*pClient);
		}
		if(res != Result_Success)
		{
			pClient->Release();
			return res;
		}

		if(NewSequence.HasOnlyShifts())
		{
			m_StickyKeysEnabler.Open();
		}
	}
	rpClient = pClient;
	return Result_Success;
}

LRESULT KeyboardSequenceEvent::OnKeyboardHookEvent(UINT uMsg, int Code)
{
	if(Code >= 0xFF)
	{
//		Log(_T("KeyboardSequenceEvent::OnKeyboardHookEvent - event was ignored Code=0x%x\n"), Code);
		return TRUE;
	}

	if(m_fBlockingHooks)
	{
//		Log(_T("KeyboardSequenceEvent::OnKeyboardHookEvent - event was blocked (total=%I64d)\n"), m_nBlockedKeyboardEvents);
		if(m_TimeOfEventsBlock.GetDiff() > MAX_BLOCKED_EVENTS_DURATION_MSEC)
		{
			Log(_T("KeyboardSequenceEvent::OnKeyboardHookEvent - someone may have forgotten to enable events (continuosly blocked for %I64d msec). Re-enabling\n"), m_TimeOfEventsBlock.GetDiff());
			m_fBlockingHooks = false;
		}
		else
		{
			m_nBlockedKeyboardEvents++;
			return TRUE;
		}
	}
	m_nKeyboardEvents++;

	if(uMsg == WM_KEYDOWN && !KeyboardMapper::IsShiftKey(Code))
	{
		m_StickyKeysEnabler.Restore();
	}

	// if focus changed or not on a window with caret, clear the list
	UpdateFocus();

	int ComposedKeyList[MAX_COMPOSED_KEYS_IN_ONE_EVENT];
	int ComposedKeyListSize = sizeof(ComposedKeyList) / sizeof(ComposedKeyList[0]);
	bool fHandleMessage = false;

	switch(uMsg)
	{
	case WM_KEYUP:
	case WM_SYSKEYUP:
		m_CurrentSequence.KeyEvent(Code, KEY_OPERATOR_MINUS, ComposedKeyList, &ComposedKeyListSize, m_rFocusInfo.QueryCurrentLanguage(), m_rFocusInfo.QueryCurrentFocusWindow());
		m_CurrentSequence.RefreshPressedKeys();
		fHandleMessage = true;
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		m_CurrentSequence.RefreshPressedKeys();
		m_CurrentSequence.KeyEvent(Code, KEY_OPERATOR_PLUS, ComposedKeyList, &ComposedKeyListSize, m_rFocusInfo.QueryCurrentLanguage(), m_rFocusInfo.QueryCurrentFocusWindow());
		fHandleMessage = true;
		break;
	}
	if(fHandleMessage)
	{
		m_LatestUIEvent.KeyboardEvent(ComposedKeyList, ComposedKeyListSize);

//		m_CurrentSequence.DebugPrint();

		if(m_rFocusInfo.QueryCurrentFocusWindow() != NULL)
		{
			Dispatch();
		}
	}
	return TRUE;
}

LRESULT KeyboardSequenceEvent::OnMouseHookEvent(HWND UNUSED(hwnd), UINT uMsg, int x, int y)
{
	if(m_fBlockingHooks)
		return TRUE;

	if(ShouldIgnoreMouseEvent(x, y))
		return TRUE;

	m_CurrentSequence.Clear();
	UpdateFocus();

	m_LatestUIEvent.MouseEvent(uMsg);

	return TRUE;
}

Result KeyboardSequenceEvent::OpenHandles()
{
	// register all mouse events
	for(int i = WM_MOUSEFIRST; i < WM_MOUSELAST; i++)
	{
		if(i != WM_MOUSEMOVE && i != WM_MOUSEWHEEL)
		{
			Result res = m_rAsyncHookDispatcher.RegisterOnMouseEvent(i, *this);
			if(res != Result_Success)
				return res;
		}
	}

	// register all keyboard events
	Result res = m_rAsyncHookDispatcher.RegisterOnKeyboard(*this);
	if(res != Result_Success)
		return res;

	return Result_Success;
}

void KeyboardSequenceEvent::CloseHandles()
{
	m_rAsyncHookDispatcher.UnregisterOnKeyboard(*this);
	m_rAsyncHookDispatcher.UnregisterOnMouseEvents(*this);
}

bool KeyboardSequenceEvent::UpdateFocus() // return true when focus is not changed, and the window is editable
{
	bool fFocusChanged = false;
	if(!m_rFocusInfo.UpdateCurrentInfo(true, fFocusChanged))
		return false;

	if(fFocusChanged)
	{
		m_CurrentSequence.FocusChanged(m_rFocusInfo.QueryCurrentFocusWindow());

		NotifyOnFocusChange(m_rFocusInfo.QueryCurrentFocusWindow());
	}

	return !fFocusChanged;
}

void KeyboardSequenceEvent::NotifyOnFocusChange(HWND hFocusWindow)
{
	FocusListenerMap::iterator Iterator = m_FocusListenerMap.begin();
	for(; Iterator != m_FocusListenerMap.end(); Iterator++)
	{
		(*Iterator).second->FocusChanged(hFocusWindow);
	}
}

void KeyboardSequenceEvent::Dispatch()
{
	KeyboardSequenceClient *pClient = NULL;
	bool fFoundMatch = false;

	for(index_t i = 0; i < (index_t)m_SequenceLisenerList.GetCount(); i++)
	{
		Result res = m_SequenceLisenerList.Get(i, pClient);
		if(pClient != NULL && res == Result_Success)
		{
			pClient->DispatchIfMatch(m_CurrentSequence, fFoundMatch);
			if(fFoundMatch)
				break;
		}
	}
}

bool KeyboardSequenceEvent::ShouldIgnoreMouseEvent(int x, int y) const
{
	IgnoredMouseEventWindowMap::const_iterator Iter = m_IgnoredMouseEventWindowMap.begin();
	for(; Iter != m_IgnoredMouseEventWindowMap.end(); Iter++)
	{
		if(Iter->second->IsPointInWindow(x, y))
			return true;
	}
	return false;
}
