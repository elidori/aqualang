#include "Pch.h"
#include "TipOfTheDayScheduler.h"
#include "UtilityWindowParams.h"
#include "TimeHelpers.h"

#define NEW_TIP_INTERVAL_MSEC (3 * 24 * 3600 * 1000) // three days

TipOfTheDayScheduler::TipOfTheDayScheduler(UtilityWindow &rUtilityWindow, ITipOfTheDayPresentor &rTipPresentor)
	: m_rUtilityWindow(rUtilityWindow),
	m_rTipPresentor(rTipPresentor),
	m_LatestTipIndex(-1),
	m_LastTipInPrevSession(-1),
	m_TipList(),
	m_fTipStatusOpened(false),
	m_TipStatus()
{
}

TipOfTheDayScheduler::~TipOfTheDayScheduler()
{
	Close();
}

bool TipOfTheDayScheduler::Open(LPCTSTR UserName)
{
	if(m_fTipStatusOpened)
		return true;

	m_LatestTipIndex = -1;
	m_LastTipInPrevSession = -1;
	m_TipList.LoadTipList(UserName);
	
	int nNewTipWaitTime = NEW_TIP_INTERVAL_MSEC;

	// load initial settings from first run
	m_TipStatus.Open(UserName);
	
	m_LatestTipIndex = (index_t)m_TipStatus.m_LatestIndexPresented.GetInt();
	if(m_LatestTipIndex >= (index_t)m_TipList.GetCount())
	{
		m_LatestTipIndex = -1;
	}
	m_LastTipInPrevSession = m_LatestTipIndex;

	FILETIME NextTime;
	if(m_TipStatus.m_NextTimeToPresentTip.GetTime(NextTime))
	{
		FileTime CurrentTime;
		__int64 Diff = CurrentTime.GetDiffMsec(NextTime);
		nNewTipWaitTime = (int)min(-Diff, (__int64)NEW_TIP_INTERVAL_MSEC);
		nNewTipWaitTime = max(nNewTipWaitTime, 100);
	}

	m_fTipStatusOpened = true;

	if(!m_rUtilityWindow.Register(*this))
		return false;
	if(!SetNextTimeToPresent(nNewTipWaitTime))
		return false;

	return true;
}

LPCTSTR TipOfTheDayScheduler::GetNextText(bool &rfMoreNext, bool &rfMorePrev)
{
	return GetText(1, rfMoreNext, rfMorePrev);
}

LPCTSTR TipOfTheDayScheduler::GetPrevText(bool &rfMoreNext, bool &rfMorePrev)
{
	return GetText(-1, rfMoreNext, rfMorePrev);
}

void TipOfTheDayScheduler::Close()
{
	if(m_fTipStatusOpened)
	{
		m_TipStatus.StoreState();
		m_fTipStatusOpened = false;
	}

	m_rUtilityWindow.Unregister(*this);

	m_LatestTipIndex = -1;
	m_LastTipInPrevSession = -1; 

	m_rUtilityWindow.StopTimer(TIMER_ID_TIP_OF_THE_DAY);
}

LPCTSTR TipOfTheDayScheduler::GetText(int nIndexStep, bool &rfMoreNext, bool &rfMorePrev)
{
	rfMoreNext = false;
	rfMorePrev = false;

	if(m_TipList.GetCount() == 0)
		return NULL;

	m_LatestTipIndex = (m_LatestTipIndex + nIndexStep + m_TipList.GetCount()) % m_TipList.GetCount();
	m_TipStatus.m_LatestIndexPresented.SetInt(m_LatestTipIndex);
	LPCTSTR Text = m_TipList.GetText(m_LatestTipIndex, m_TextContainer);
	if(Text == NULL)
		return NULL;
	rfMoreNext = !(m_LatestTipIndex == (index_t)((m_LastTipInPrevSession + m_TipList.GetCount()) % m_TipList.GetCount()));
	rfMorePrev = !(m_LatestTipIndex == (index_t)((m_LastTipInPrevSession + 1) % m_TipList.GetCount()));
	return Text;
}

void TipOfTheDayScheduler::Message(UINT uMsg, WPARAM wParam, LPARAM UNUSED(lParam))
{
	if(uMsg == WM_TIMER)
	{
		if(wParam == TIMER_ID_TIP_OF_THE_DAY)
		{
			m_LastTipInPrevSession = m_LatestTipIndex;

			if(m_rTipPresentor.CanPresentTipOfTheDay())
			{
				bool fMoreNext;
				bool fMorePrev;
				LPCTSTR Text = GetText(1, fMoreNext, fMorePrev);
				if(Text != NULL && _tcslen(Text) > 0)
				{
					m_rTipPresentor.PresentTipOfTheDay(Text, fMoreNext, fMorePrev);
				}
			}
			SetNextTimeToPresent(NEW_TIP_INTERVAL_MSEC); // reset the time, since the first time may be different then NEW_TIP_INTERVAL_MSEC
		}
	}
}

bool TipOfTheDayScheduler::SetNextTimeToPresent(int nNewTipWaitTime)
{
	if(!m_rUtilityWindow.StartTimer(nNewTipWaitTime, TIMER_ID_TIP_OF_THE_DAY))
		return false;

	FileTime NextTime;
	NextTime.Add(nNewTipWaitTime);

	m_TipStatus.m_NextTimeToPresentTip.SetTime(NextTime.GetFILETIME());
	return true;
}
