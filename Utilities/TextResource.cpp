#include "Pch.h"
#include "TextResource.h"

TextResource::TextResource()
	: m_TextColor(RGB(0, 0, 0)),
	m_fBold(false),
	m_fRectInitialized(false)
{
}

TextResource::~TextResource()
{
}

void TextResource::SetTextColor(COLORREF TextColor, bool fBold)
{
	m_TextColor = TextColor;
	m_fBold = fBold;
}

void TextResource::SetText(LPCTSTR Text)
{
	if(Text != NULL)
	{
		m_Text = Text;
	}
	else
	{
		m_Text = _T("");
	}
}

void TextResource::SetTargetRect(const RECT &TextRect)
{
	m_TextRect = TextRect;
	m_fRectInitialized = true;
}

bool TextResource::DrawText(HDC hTargetDC, bool fUpdateTextSize)
{
	if(m_Text == _T("") || !m_fRectInitialized)
		return true;

	::SetBkMode(hTargetDC, TRANSPARENT);
	::SetTextColor(hTargetDC, m_TextColor);

	// match font
	if(fUpdateTextSize)
	{
		int FontHeight = (m_TextRect.bottom - m_TextRect.top + 4) / 5;
		int FontWidth = FontHeight * 3 / 4;
		HFONT hFont = ::CreateFont(
			-FontHeight, -FontWidth, 0, 0, m_fBold ? FW_BOLD : 0, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
							CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("MS Sans Serif")
							);
		HFONT hOldFont = (HFONT)::SelectObject(hTargetDC, hFont);
		::DrawText(hTargetDC, m_Text.c_str(), -1, &m_TextRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		::SelectObject(hTargetDC, hOldFont);
		::DeleteObject(hFont);
	}
	else
	{
		// draw text
		::DrawText(hTargetDC, m_Text.c_str(), -1, &m_TextRect, DT_CENTER);
	}

	return true;
}
