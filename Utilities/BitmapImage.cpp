#include "Pch.h"
#include "BitmapImage.h"

extern HMODULE g_hModule;

BitmapImage::BitmapImage()
	: m_hBitmap(NULL),
	m_BitmapWidth(0),
	m_BitmapHeight(0)
{
}

BitmapImage::~BitmapImage()
{
}

void BitmapImage::SetText(LPCTSTR Text, COLORREF Color, bool fBold, const POINT & UNUSED(OffsetInBitmap) /*= BmPoint()*/)
{
	m_TextOverlay.SetTextColor(Color, fBold);
	m_TextOverlay.SetText(Text);
}

bool BitmapImage::Load(int BitmapResourceId)
{
	return Load(MAKEINTRESOURCE(BitmapResourceId));
}

bool BitmapImage::Load(LPCTSTR BitmapResourceName)
{
	m_MsImg32Loader.Open();

	m_hBitmap = NULL;

	HRSRC hrsRC = ::FindResource (g_hModule, BitmapResourceName, RT_BITMAP);
	if(!hrsRC)
		return false;
	DWORD dwResourceSize = SizeofResource (g_hModule, hrsRC);
	if(dwResourceSize < sizeof(BITMAPINFOHEADER))
		return false;
	HGLOBAL hglbRC = ::LoadResource (g_hModule, hrsRC);
	if(!hglbRC)
		return false;
	LPVOID lpRsc = ::LockResource(hglbRC);
	if(!lpRsc)
		return false;
	BITMAPINFOHEADER *pHeader = (BITMAPINFOHEADER *)lpRsc;
	m_BitmapWidth = pHeader->biWidth;
	m_BitmapHeight = pHeader->biHeight;

	m_hBitmap = (HBITMAP)::LoadImage(g_hModule, BitmapResourceName, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
	if(m_hBitmap == NULL)
		return false;

	return true;
}

bool BitmapImage::IsLoaded() const
{
	return (m_hBitmap != NULL);
}

void BitmapImage::UseColorKey(COLORREF ColorKey)
{
	m_fUseColorKey = true;
	m_ColorKey = ColorKey;
}

bool BitmapImage::GetBitmapSizeInfo(int &Width, int &Height) const
{
	if(m_hBitmap == NULL)
		return false;
	Width = m_BitmapWidth;
	Height = m_BitmapHeight;
	return true;
}

bool BitmapImage::Draw(HDC hTargetDC, const RECT &rRectInTarget /*= BmRect()*/, const POINT &OffsetInBitmap /*= BmPoint()*/)
{
	if(m_hBitmap == NULL)
		return false;

	HDC hMemDC = ::CreateCompatibleDC(hTargetDC);
	if(hMemDC == NULL)
		return false;
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, m_hBitmap);

	BOOL fResult;

	RECT TargetRect = rRectInTarget;
	if(TargetRect.top == -1 && TargetRect.bottom == -1)
	{
		SetRect(&TargetRect, 0, 0, m_BitmapWidth, m_BitmapHeight);
	}

	POINT ActualOffsetInBitmap = OffsetInBitmap;
	if(ActualOffsetInBitmap.x == -1 || ActualOffsetInBitmap.y == -1)
	{
		ActualOffsetInBitmap.x = 0;
		ActualOffsetInBitmap.y = 0;
	}

	if(m_fUseColorKey && m_MsImg32Loader.IsOpen())
	{
		fResult = m_MsImg32Loader.TransparentBlt(
						hTargetDC,
						TargetRect.left, TargetRect.top, TargetRect.right - TargetRect.left, TargetRect.bottom - TargetRect.top,
						hMemDC,
						ActualOffsetInBitmap.x, ActualOffsetInBitmap.y, TargetRect.right - TargetRect.left, TargetRect.bottom - TargetRect.top,
						m_ColorKey
						);
	}
	else
	{
		fResult = ::BitBlt(
						hTargetDC,
						TargetRect.left, TargetRect.top, TargetRect.right - TargetRect.left, TargetRect.bottom - TargetRect.top,
						hMemDC,
						ActualOffsetInBitmap.x, ActualOffsetInBitmap.y,
						SRCCOPY
						);
	}

	m_TextOverlay.SetTargetRect(TargetRect);
	if(!m_TextOverlay.DrawText(hTargetDC, true))
	{
		fResult = FALSE;
	}

	::SelectObject(hMemDC, hOldBitmap);
	::DeleteDC(hMemDC);

	return (fResult != FALSE);
}

bool BitmapImage::StretchDraw(HDC hTargetDC, const RECT &rRectInTarget, const RECT &rRectInBitmap /*= BmRect()*/)
{
	if(m_hBitmap == NULL)
		return false;

	HDC hMemDC = ::CreateCompatibleDC(hTargetDC);
	if(hMemDC == NULL)
		return false;
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, m_hBitmap);

	RECT BitmapRect = rRectInBitmap;
	if(BitmapRect.top == -1 && BitmapRect.bottom == -1)
	{
		SetRect(&BitmapRect, 0, 0, m_BitmapWidth, m_BitmapHeight);
	}

	BOOL fResult = DoStretchDraw(hTargetDC, rRectInTarget, hMemDC, BitmapRect);
	
	m_TextOverlay.SetTargetRect(rRectInTarget);
	if(!m_TextOverlay.DrawText(hTargetDC, true))
	{
		fResult = FALSE;
	}

	::SelectObject(hMemDC, hOldBitmap);
	::DeleteDC(hMemDC);

	return (fResult != FALSE);
}

bool BitmapImage::StretchDrawKeepEdges(HDC hTargetDC, const RECT &rRectInTarget, const POINT &rTopLeftEdge, const POINT &rBottomRightEdge, const RECT &rRectInBitmap /*= BmRect()*/)
{
	if(m_hBitmap == NULL)
		return false;

	HDC hMemDC = ::CreateCompatibleDC(hTargetDC);
	if(hMemDC == NULL)
		return false;
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, m_hBitmap);

	RECT BitmapRect = rRectInBitmap;
	if(BitmapRect.top == -1 && BitmapRect.bottom == -1)
	{
		SetRect(&BitmapRect, 0, 0, m_BitmapWidth, m_BitmapHeight);
	}

	bool fResult = true;

	int TargetWidth = rRectInTarget.right - rRectInTarget.left;
	int TargetHeight = rRectInTarget.bottom - rRectInTarget.top;
	int BitmapWidth = BitmapRect.right - BitmapRect.left;
	int BitmapHeight = BitmapRect.bottom - BitmapRect.top;

	if(BitmapWidth <= rTopLeftEdge.x + rBottomRightEdge.x)
		return false;
	if(BitmapHeight <= rTopLeftEdge.y + rBottomRightEdge.y)
		return false;

	RECT TargetArea;
	RECT BitmapArea;

	if(rTopLeftEdge.y > 0)
	{
		// copy right top area
		if(rTopLeftEdge.x > 0)
		{
			SetRect(&TargetArea, rRectInTarget.left, rRectInTarget.top, rRectInTarget.left + rTopLeftEdge.x, rRectInTarget.top + rTopLeftEdge.y);
			SetRect(&BitmapArea, BitmapRect.left, BitmapRect.top, BitmapRect.left + rTopLeftEdge.x, BitmapRect.top + rTopLeftEdge.y);
			fResult &= DoStretchDraw(hTargetDC, TargetArea, hMemDC, BitmapArea);
		}
		// copy middle top area
		if(rTopLeftEdge.x + rBottomRightEdge.x < TargetWidth)
		{
			SetRect(&TargetArea, rRectInTarget.left + rTopLeftEdge.x, rRectInTarget.top, rRectInTarget.right - rBottomRightEdge.x, rRectInTarget.top + rTopLeftEdge.y);
			SetRect(&BitmapArea, BitmapRect.left + rTopLeftEdge.x, BitmapRect.top, BitmapRect.right - rBottomRightEdge.x, BitmapRect.top + rTopLeftEdge.y);
			fResult &= DoStretchDraw(hTargetDC, TargetArea, hMemDC, BitmapArea);
		}
		// copy left top area
		if(rBottomRightEdge.x > 0)
		{
			SetRect(&TargetArea, rRectInTarget.right - rBottomRightEdge.x, rRectInTarget.top, rRectInTarget.right, rRectInTarget.top + rTopLeftEdge.y);
			SetRect(&BitmapArea, BitmapRect.right - rBottomRightEdge.x, BitmapRect.top, BitmapRect.right, BitmapRect.top + rTopLeftEdge.y);
			fResult &= DoStretchDraw(hTargetDC, TargetArea, hMemDC, BitmapArea);
		}
	}

	if(rTopLeftEdge.y + rBottomRightEdge.y < TargetHeight)
	{
		// copy right top area
		if(rTopLeftEdge.x > 0)
		{
			SetRect(&TargetArea, rRectInTarget.left, rRectInTarget.top + rTopLeftEdge.y, rRectInTarget.left + rTopLeftEdge.x, rRectInTarget.bottom - rBottomRightEdge.y);
			SetRect(&BitmapArea, BitmapRect.left, BitmapRect.top + rTopLeftEdge.y, BitmapRect.left + rTopLeftEdge.x, BitmapRect.bottom - rBottomRightEdge.y);
			fResult &= DoStretchDraw(hTargetDC, TargetArea, hMemDC, BitmapArea);
		}
		// copy middle top area
		if(rTopLeftEdge.x + rBottomRightEdge.x < TargetWidth)
		{
			SetRect(&TargetArea, rRectInTarget.left + rTopLeftEdge.x, rRectInTarget.top + rTopLeftEdge.y, rRectInTarget.right - rBottomRightEdge.x, rRectInTarget.bottom - rBottomRightEdge.y);
			SetRect(&BitmapArea, BitmapRect.left + rTopLeftEdge.x, BitmapRect.top + rTopLeftEdge.y, BitmapRect.right - rBottomRightEdge.x, BitmapRect.bottom - rBottomRightEdge.y);
			fResult &= DoStretchDraw(hTargetDC, TargetArea, hMemDC, BitmapArea);
		}
		// copy left top area
		if(rBottomRightEdge.x > 0)
		{
			SetRect(&TargetArea, rRectInTarget.right - rBottomRightEdge.x, rRectInTarget.top + rTopLeftEdge.y, rRectInTarget.right, rRectInTarget.bottom - rBottomRightEdge.y);
			SetRect(&BitmapArea, BitmapRect.right - rBottomRightEdge.x, BitmapRect.top + rTopLeftEdge.y, BitmapRect.right, BitmapRect.bottom - rBottomRightEdge.y);
			fResult &= DoStretchDraw(hTargetDC, TargetArea, hMemDC, BitmapArea);
		}
	}

	if(rBottomRightEdge.y > 0)
	{
		// copy right top area
		if(rTopLeftEdge.x > 0)
		{
			SetRect(&TargetArea, rRectInTarget.left, rRectInTarget.bottom - rBottomRightEdge.y, rRectInTarget.left + rTopLeftEdge.x, rRectInTarget.bottom);
			SetRect(&BitmapArea, BitmapRect.left, BitmapRect.bottom - rBottomRightEdge.y, BitmapRect.left + rTopLeftEdge.x, BitmapRect.bottom);
			fResult &= DoStretchDraw(hTargetDC, TargetArea, hMemDC, BitmapArea);
		}
		// copy middle top area
		if(rTopLeftEdge.x + rBottomRightEdge.x < TargetWidth)
		{
			SetRect(&TargetArea, rRectInTarget.left + rTopLeftEdge.x, rRectInTarget.bottom - rBottomRightEdge.y, rRectInTarget.right - rBottomRightEdge.x, rRectInTarget.bottom);
			SetRect(&BitmapArea, BitmapRect.left + rTopLeftEdge.x, BitmapRect.bottom - rBottomRightEdge.y, BitmapRect.right - rBottomRightEdge.x, BitmapRect.bottom);
			fResult &= DoStretchDraw(hTargetDC, TargetArea, hMemDC, BitmapArea);
		}
		// copy left top area
		if(rBottomRightEdge.x > 0)
		{
			SetRect(&TargetArea, rRectInTarget.right - rBottomRightEdge.x, rRectInTarget.bottom - rBottomRightEdge.y, rRectInTarget.right, rRectInTarget.bottom);
			SetRect(&BitmapArea, BitmapRect.right - rBottomRightEdge.x, BitmapRect.bottom - rBottomRightEdge.y, BitmapRect.right, BitmapRect.bottom);
			fResult &= DoStretchDraw(hTargetDC, TargetArea, hMemDC, BitmapArea);
		}
	}

	m_TextOverlay.SetTargetRect(rRectInTarget);
	if(!m_TextOverlay.DrawText(hTargetDC, true))
	{
		fResult = FALSE;
	}

	::SelectObject(hMemDC, hOldBitmap);
	::DeleteDC(hMemDC);

	return (fResult != FALSE);
}

bool BitmapImage::DoStretchDraw(HDC hTargetDC, const RECT &rRectInTarget, HDC hBitmapDC, const RECT &rRectInBitmap)
{
	BOOL fResult;

	if(m_fUseColorKey && m_MsImg32Loader.IsOpen())
	{
		fResult = m_MsImg32Loader.TransparentBlt(
						hTargetDC,
						rRectInTarget.left, rRectInTarget.top, rRectInTarget.right - rRectInTarget.left, rRectInTarget.bottom - rRectInTarget.top,
						hBitmapDC,
						rRectInBitmap.left, rRectInBitmap.top, rRectInBitmap.right - rRectInBitmap.left, rRectInBitmap.bottom - rRectInBitmap.top,
						m_ColorKey
						);
	}
	else
	{
		fResult = ::StretchBlt(
						hTargetDC,
						rRectInTarget.left, rRectInTarget.top, rRectInTarget.right - rRectInTarget.left, rRectInTarget.bottom - rRectInTarget.top,
						hBitmapDC,
						rRectInBitmap.left, rRectInBitmap.top, rRectInBitmap.right - rRectInBitmap.left, rRectInBitmap.bottom - rRectInBitmap.top,
						SRCCOPY
						);
	}
	return (fResult != FALSE);
}
