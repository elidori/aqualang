#ifndef _LATEST_UI_EVENT_HEADER_
#define _LATEST_UI_EVENT_HEADER_

#include "TimeHelpers.h"

class LatestUIEvent
{
public:
	LatestUIEvent();
	virtual ~LatestUIEvent();

	void Clear();

	void KeyboardEvent(int ComposedKeyList[], int ListSize);
	void MouseEvent(UINT uMsg);

	bool GetLatestEvent(__int64 &TimePassedSinceEvent, UINT &uMsg, int &KbKey) const;
	bool IsInPrintMode(__int64 &TimePassedSinceLastPrint) const;
private:
	bool m_fValid;
	Timer m_Time;

	UINT m_uMsg;
	int m_KeyboardKey;

	bool m_fInPrintMode;
	Timer m_LatestPrintTime;
};

#endif // _LATEST_UI_EVENT_HEADER_