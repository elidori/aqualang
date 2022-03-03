#include "Pch.h"
#include "KeyboardSequenceClient.h"
#include "KeyboardSequenceEvent.h"
#include "KeyboardEventInfo.h"
#include "FocusInfo.h"
#include "Log.h"

KeyboardSequenceClient::KeyboardSequenceClient(
					const KeyboardSequence &rSequence,
					UtilityWindow &rUtilityWindow,
					KeyboardSequenceEvent &rKeyboardSequenceEvent,
					FocusInfo &rFocusInfo,
					const WindowItemDataBase &rWindowsDatabase,
					const LanguageDetector &rLangaugeDetector,
					bool fBackgroundClient
					)
	: m_rUtilityWindow(rUtilityWindow),
	m_rKeyboardSequenceEvent(rKeyboardSequenceEvent),
	m_rFocusInfo(rFocusInfo),
	m_rWindowsDatabase(rWindowsDatabase),
	m_rLangaugeDetector(rLangaugeDetector),
	m_ReferenceSequence(rSequence),
	m_fBackgroundClient(fBackgroundClient)
{
}

KeyboardSequenceClient::~KeyboardSequenceClient()
{
	Close();
}

Result KeyboardSequenceClient::Create(
					KeyboardSequenceClient* &rpClient,
					const KeyboardSequence &rSequence,
					UtilityWindow &rUtilityWindow,
					KeyboardSequenceEvent &rKeyboardSequenceEvent,
					FocusInfo &rFocusInfo,
					const WindowItemDataBase &rWindowsDatabase,
					const LanguageDetector &rLangaugeDetector,
					bool fBackgroundClient
					)
{
	rpClient = NULL;

	KeyboardSequenceClient *pNew = new KeyboardSequenceClient(rSequence, rUtilityWindow, rKeyboardSequenceEvent, rFocusInfo, rWindowsDatabase, rLangaugeDetector, fBackgroundClient);
	if(!pNew)
		return Result_MemAllocFailed;
	Result res = pNew->Open();
	if(res != Result_Success)
	{
		pNew->Release();
		return res;
	}
	rpClient = pNew;
	return Result_Success;
}

Result KeyboardSequenceClient::Open()
{
	if(!m_rUtilityWindow.Register(*this))
		return Result_InvalidParameter;
	return Result_Success;
}

void KeyboardSequenceClient::Close()
{
	m_rUtilityWindow.Unregister(*this);
}

bool KeyboardSequenceClient::AddListener(IKeyboardEvent &rListener)
{
	int Key = (int)(&rListener);
	if(m_ListenerMap.find(Key) != m_ListenerMap.end())
		return false;
	m_ListenerMap.insert(KbListenerMap::value_type(Key, &rListener));
	return true;
}

bool KeyboardSequenceClient::IsEmpty() const
{
	return (m_ListenerMap.size() == 0);
}

bool KeyboardSequenceClient::IsSimilar(const KeyboardSequence &rSequence, bool fBackground, bool &rfIsEqual)
{
	rfIsEqual = false;
	if(fBackground != m_fBackgroundClient)
		return false;

	rfIsEqual = m_ReferenceSequence.IsEqual(rSequence);
	if(rfIsEqual)
		return true;
	if(rSequence.IsContaining(m_ReferenceSequence))
		return true;
	if(m_ReferenceSequence.IsContaining(rSequence))
		return true;
	return false;
}

bool KeyboardSequenceClient::DispatchIfMatch(const KeyboardSequence &rCurrentSequence, bool &rfFoundMatch)
{
	if(!rCurrentSequence.IsMatching(m_ReferenceSequence))
		return false;
	m_rUtilityWindow.UserMessage(WPARAM_KEYBOARD_SEQUENCE_EVENT, (LPARAM)this);
	rfFoundMatch = true;
	return true;
}

bool KeyboardSequenceClient::RemoveListener(const IKeyboardEvent &rListener)
{
	int Key = (int)(&rListener);
	KbListenerMap::iterator Iterator = m_ListenerMap.find(Key);
	if(Iterator == m_ListenerMap.end())
		return false;
	m_ListenerMap.erase(Iterator);
	return true;
}

void KeyboardSequenceClient::Release()
{
	delete this;
}

void KeyboardSequenceClient::Message(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_UTILITY_USER_MESSAGE:
		switch(wParam)
		{
		case WPARAM_KEYBOARD_SEQUENCE_EVENT:
			if(lParam == (LPARAM)this)
			{
				HWND hCurrentWindow = m_rFocusInfo.QueryCurrentFocusWindow();
				if(hCurrentWindow != NULL)
				{
					if(!m_fBackgroundClient)
					{
						m_rKeyboardSequenceEvent.BlockHooks(true);
						m_rKeyboardSequenceEvent.ClearReferenceSequenceKeys(m_ReferenceSequence);
					}
					DispatchClients(hCurrentWindow, m_rFocusInfo, m_rLangaugeDetector);
				}
			}
			break;
		case WPARAM_KEYBOARD_SEQUENCE_EVENT_COMPLETED:
		case WPARAM_KEYBOARD_SEQUENCE_EVENT_ABORTED:
			if(lParam == (LPARAM)this)
			{
				bool fComplete = (wParam == WPARAM_KEYBOARD_SEQUENCE_EVENT_COMPLETED);

				if(fComplete && m_ReferenceSequence.HasOnlyShifts())
				{
					m_rKeyboardSequenceEvent.DisableStickyKeysUntilNextStroke(); // shall be restored in the next non SHIFT key
				}

				if(!m_fBackgroundClient)
				{
					m_rKeyboardSequenceEvent.MarkCurrentPosition();
				}

				m_rKeyboardSequenceEvent.BlockHooks(false);
			}
			break;
		}
	}
}

void KeyboardSequenceClient::EventOperationEnded(bool fComplete)
{
	if(fComplete)
	{
		m_rUtilityWindow.UserMessage(WPARAM_KEYBOARD_SEQUENCE_EVENT_COMPLETED, (LPARAM)this);
	}
	else
	{
		m_rUtilityWindow.UserMessage(WPARAM_KEYBOARD_SEQUENCE_EVENT_ABORTED, (LPARAM)this);
	}
}

void KeyboardSequenceClient::DispatchClients(HWND hCurrentWindow, const FocusInfo &rFocusInfo, const LanguageDetector &rLangaugeDetector)
{
	KeyboardEventInfo Info(hCurrentWindow, rFocusInfo, rLangaugeDetector, m_rKeyboardSequenceEvent, m_rWindowsDatabase);

	std::vector<IKeyboardEvent *> DispatchList;
	BuildDispatchList(DispatchList, Info);

	bool fFoundClients = false;

	if(DispatchList.size() == 0)
	{
		// no candidates for this event
	}
	else if(DispatchList.size() == 1)
	{
		fFoundClients = true;
		DispatchList.at(0)->Operate(Info, *this);
	}
	else
	{
		// open a floating menu to pick the selected listner
	}

	if(!fFoundClients)
	{
		// the blocking must be release, as there is no client to release it
		EventOperationEnded(false);
	}
}

void KeyboardSequenceClient::BuildDispatchList(std::vector<IKeyboardEvent *> &rDispatchList, KeyboardEventInfo &Info)
{
	KbListenerMap::iterator Iterator = m_ListenerMap.begin();
	for(; Iterator != m_ListenerMap.end(); Iterator++)
	{
		IKeyboardEvent *pListener = (*Iterator).second;
		bool fExclusive = false;
		if(pListener->CanOperate(Info, fExclusive))
		{
			if(fExclusive)
			{
				rDispatchList.erase(rDispatchList.begin(), rDispatchList.end());
				rDispatchList.push_back(pListener);
				return;
			}
			else
			{
				rDispatchList.push_back(pListener);
			}
		}
	}
}
