#ifndef _TIP_OF_THE_DAY_SCHEDULER_HEADER_
#define _TIP_OF_THE_DAY_SCHEDULER_HEADER_

#include "TipOfTheDayXml.h"
#include "UtilityWindow.h"

class ITipOfTheDayPresentor
{
protected:
	virtual ~ITipOfTheDayPresentor() {}
public:
	virtual bool CanPresentTipOfTheDay() = 0;
	virtual void PresentTipOfTheDay(LPCTSTR Text, bool fMoreNext, bool fMorePrev) = 0;
};

class TipOfTheDayScheduler : public IWindowMessageListener
{
public:
	TipOfTheDayScheduler(UtilityWindow &rUtilityWindow, ITipOfTheDayPresentor &rTipPresentor);
	virtual ~TipOfTheDayScheduler();

	bool Open(LPCTSTR UserName);

	// when tip is opened and user selected the next tip
	LPCTSTR GetNextText(bool &rfMoreNext, bool &rfMorePrev);
	// when tip is opened and user selected the previous tip
	LPCTSTR GetPrevText(bool &rfMoreNext, bool &rfMorePrev);

	void Close();
private:
	// when time has come to present a new tip
	LPCTSTR GetText(int nIndexStep, bool &rfMoreNext, bool &rfMorePrev);

	// IWindowMessageListener pure virtuals {
	virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// }

	bool SetNextTimeToPresent(int nNewTipWaitTime);
private:
	UtilityWindow &m_rUtilityWindow;
	ITipOfTheDayPresentor &m_rTipPresentor;

	index_t m_LatestTipIndex;
	index_t m_LastTipInPrevSession;

	TipOfTheDayListXml m_TipList;
	bool m_fTipStatusOpened;
	TipOfTheDayStatusXml m_TipStatus;

	_tstring m_TextContainer;
};

#endif // _TIP_OF_THE_DAY_SCHEDULER_HEADER_