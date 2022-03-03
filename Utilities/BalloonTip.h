#ifndef _BALOON_TIP_HEADER_
#define _BALOON_TIP_HEADER_

#include "DialogObject.h"
#include "BitmapImage.h"
#include "TextResource.h"

class BalloonTip;

class ITipListener
{
protected:
	virtual ~ITipListener() {}
public:
	virtual void TipShouldClose(BalloonTip &rTip) = 0;
	virtual void TipWasClicked(BalloonTip &rTip, bool fLeftClick) = 0;
};

class BalloonTip : public DialogObject
{
public:
	BalloonTip(
			int TipFadeOutDuration,
			int TipFadeOutSteps,
			int UpLeftId,
			int UpRightId,
			int DownLeftId,
			int DownRightId,
			COLORREF ColorKey = RGB(0, 0, 255),
			COLORREF TextColor = RGB(0, 0, 0),
			const RECT *pTextRect = NULL
			);
	virtual ~BalloonTip();

	bool Open();
	bool Init(
			LPCTSTR UserName,
			LPCTSTR Title,
			LPCTSTR Text,
			const RECT *pSubjectRect,
			int DialogId,
			int TipInitialAlpha,
			int TipDurationMsec,
			bool fFollowMousePosition,
			ITipListener *pListener
			);
	void Term();
	void Close();
private:
	virtual void DialogInit(HWND hwndDlg);
    virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault);

	void CalculateRectAndType(const RECT &rSubjectRect, RECT &rWinRect, int &rBitmapDirection, int Width, int Height);
private:
	ITipListener *m_pListener;
	bool m_fExit;

	int m_nTimerCount;

	_tstring m_UserName;
	// directions
	const int m_UpLeftId;
	const int m_UpRightId;
	const int m_DownLeftId;
	const int m_DownRightId;

	const COLORREF m_ColorKey;

	// tip times
	int m_TipInitialAlpha;
	const int m_TipFadeOutDuration;
	const int m_TipFadeOutSteps;

	TextResource m_TextResource;
};

#endif // _BALOON_TIP_HEADER_