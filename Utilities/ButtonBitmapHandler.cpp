#include "Pch.h"
#include "ButtonBitmapHandler.h"
#include "Log.h"

ButtonBitmapHandler::ButtonBitmapHandler(LPCTSTR ResourceName, HWND hWindow, bool fUseManualState /*= false*/)
	: m_fUseManualState(fUseManualState),
	m_hWindow(hWindow),
	m_fChecked(false),
	m_fHighLight(false),
	m_NormalBitmap(),
	m_PressedBitmap(),
	m_DisabledBitmap(),
	m_HighlightedBitmap()
{
	TCHAR BitmapResource[128];
	_tcscpy_s(BitmapResource, sizeof(BitmapResource) / sizeof(BitmapResource[0]), ResourceName);

	size_t nNameLength = _tcslen(ResourceName);

	_tcscpy_s(BitmapResource + nNameLength, sizeof(BitmapResource) / sizeof(BitmapResource[0]) - nNameLength, _T("U"));
	m_NormalBitmap.Load(BitmapResource);
	
	_tcscpy_s(BitmapResource + nNameLength, sizeof(BitmapResource) / sizeof(BitmapResource[0]) - nNameLength, _T("D"));
	m_PressedBitmap.Load(BitmapResource);

	_tcscpy_s(BitmapResource + nNameLength, sizeof(BitmapResource) / sizeof(BitmapResource[0]) - nNameLength, _T("X"));
	m_DisabledBitmap.Load(BitmapResource);
	
	_tcscpy_s(BitmapResource + nNameLength, sizeof(BitmapResource) / sizeof(BitmapResource[0]) - nNameLength, _T("F"));
	m_HighlightedBitmap.Load(BitmapResource);
}

ButtonBitmapHandler::~ButtonBitmapHandler()
{
}

void ButtonBitmapHandler::SetText(
				LPCTSTR Text,
				COLORREF NormalColor,
				COLORREF PressedColor,
				COLORREF DisabledColor,
				COLORREF HighlightedColor
				)
{
	m_NormalBitmap.SetText(Text, NormalColor, false);
	m_PressedBitmap.SetText(Text, PressedColor, true);
	m_DisabledBitmap.SetText(Text, DisabledColor, false);
	m_HighlightedBitmap.SetText(Text, HighlightedColor, true);
}

void ButtonBitmapHandler::Check(bool fCheck)
{
	if(m_fUseManualState)
	{
		m_fChecked = fCheck;
	}
}

bool ButtonBitmapHandler::GetChecked() const
{
	assert(m_fUseManualState);
	return m_fChecked;
}

void ButtonBitmapHandler::HighLight(bool fHighLight)
{
	m_fHighLight = fHighLight;
	::InvalidateRect(m_hWindow, NULL, FALSE);
}

bool ButtonBitmapHandler::Draw(const DRAWITEMSTRUCT &DrawItem)
{
	BitmapImage *pSelectedImage = NULL;
	if(::IsWindowEnabled(DrawItem.hwndItem))
	{
		bool fChecked;
		if(m_fUseManualState)
		{
			fChecked = m_fChecked;
		}
		else
		{
			LRESULT State = ::SendMessage(DrawItem.hwndItem, BM_GETSTATE, 0, 0);
			fChecked = ((State & (BST_CHECKED | BST_PUSHED)) != 0);
		}

		if(fChecked)
		{
			pSelectedImage = &m_PressedBitmap;
		}
		else
		{
			if(m_fHighLight)
			{
				pSelectedImage = &m_HighlightedBitmap;
			}
			else
			{
				pSelectedImage = &m_NormalBitmap;
			}
		}
	}
	else
	{
		pSelectedImage = &m_DisabledBitmap;
	}
	assert(pSelectedImage);
	return pSelectedImage->StretchDraw(DrawItem.hDC, DrawItem.rcItem);
}
