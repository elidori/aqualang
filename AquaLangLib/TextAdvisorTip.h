#ifndef _TEXT_ADVISOR_TIP_HEADER_
#define _TEXT_ADVISOR_TIP_HEADER_

#include "BasicCompactTip.h"

class ICompactTipResult;
class ButtonBitmapHandler;
class TipCoordinator;

class TextAdvisorTip : public BasicCompactTip
{
public:
	TextAdvisorTip(ICompactTipResult &rTipResult, TipCoordinator &rTipCoordinator);
	virtual ~TextAdvisorTip();

	bool HasWindow(HWND hWindow) const;

	bool Open(LPCTSTR UserName);

	void ShouldActive()		{ m_fActive = true; }
	bool IsActive() const	{ return m_fActive; }

	bool Init(const RECT &rCaretRect, LPCTSTR TypedText, LPCTSTR RemainingText);
	void Term();

	void Close();

	const _tstring &GetRemainingText() const	{ return m_RemainingText; }
private:
	virtual bool ModifyTextBox(LPCTSTR Text, int TextX, int &rTextWidth);
    virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault);
private:
	ICompactTipResult &m_rTipResult;
	bool m_fActive;

	_tstring m_TypedText;
	_tstring m_RemainingText;

	ButtonBitmapHandler *m_pCloseButton;
};

#endif // _TEXT_ADVISOR_TIP_HEADER_
