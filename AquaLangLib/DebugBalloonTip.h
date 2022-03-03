#ifndef _DEBUG_BALOON_TIP_HEADER_
#define _DEBUG_BALOON_TIP_HEADER_

#include "DialogObject.h"

class DebugBalloonTip : public DialogObject
{
public:
	DebugBalloonTip();
	virtual ~DebugBalloonTip();

	bool Init(
			LPCTSTR UserName,
			LPCTSTR Text,
			const RECT *pRect,
			int DialogId
			);
	void Term();
};

#endif // _DEBUG_BALOON_TIP_HEADER_