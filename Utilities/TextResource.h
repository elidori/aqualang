#ifndef _TEXT_RESOURCE_HEADER_
#define _TEXT_RESOURCE_HEADER_

class TextRect : public RECT
{
public:
	TextRect(int x, int y, int w, int h)
	{
		left = x;
		top = y;
		right = x + w;
		bottom = y + h;
	}
};

class TextResource
{
public:
	TextResource();
	virtual ~TextResource();

	void SetTextColor(COLORREF TextColor, bool fBold);
	void SetText(LPCTSTR Text);
	void SetTargetRect(const RECT &TextRect);

	bool DrawText(HDC hTargetDC, bool fUpdateTextSize);
private:
	_tstring m_Text;
	COLORREF m_TextColor;
	bool m_fBold;
	bool m_fRectInitialized;
	RECT m_TextRect;
};

#endif // _TEXT_RESOURCE_HEADER_