#ifndef _BITMAP_IMAGE_HEADER_
#define _BITMAP_IMAGE_HEADER_

#include "TextResource.h"
#include "MsImg32Loader.h"

class BmPoint : public POINT
{
public:
	BmPoint() { x = -1; y = -1; }
};

class BmRect : public RECT
{
public:
	BmRect() { left = -1; top = -1; right = -1; bottom = -1; }
};

class BitmapImage
{
public:
	BitmapImage();
	virtual ~BitmapImage();

	void SetText(LPCTSTR Text, COLORREF Color, bool fBold, const POINT &OffsetInBitmap = BmPoint());

	bool Load(int BitmapResourceId);
	bool Load(LPCTSTR BitmapResourceName);
	bool IsLoaded() const;

	void UseColorKey(COLORREF ColorKey);

	bool GetBitmapSizeInfo(int &Width, int &Height) const;

	bool Draw(HDC hTargetDC, const RECT &rRectInTarget = BmRect(), const POINT &OffsetInBitmap = BmPoint());
	bool StretchDraw(HDC hTargetDC, const RECT &rRectInTarget, const RECT &rRectInBitmap = BmRect());
	bool StretchDrawKeepEdges(HDC hTargetDC, const RECT &rRectInTarget, const POINT &rTopLeftEdge, const POINT &rBottomRightEdge, const RECT &rRectInBitmap = BmRect());
private:
	bool DoStretchDraw(HDC hTargetDC, const RECT &rRectInTarget, HDC hBitmapDC, const RECT &rRectInBitmap);
private:
	MsImg32Loader m_MsImg32Loader;

	HBITMAP m_hBitmap;

	int m_BitmapWidth;
	int m_BitmapHeight;

	bool m_fUseColorKey;
	COLORREF m_ColorKey;

	TextResource m_TextOverlay;
};

#endif // _BITMAP_IMAGE_HEADER_