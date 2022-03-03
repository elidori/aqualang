#include "Pch.h"
#include "ScreenFitter.h"

bool ScreenFitter::PlaceInScreen(int Width, int Height, const RECT &PointedRect, RECT &rCalcRect, int PointerWidthOffset /*= 0*/)
{
	int x = PointedRect.right - PointerWidthOffset;
	int y = PointedRect.bottom;

	RECT rctDesktop;
	if(!::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rctDesktop, NULL))
		return false;

	if(x + Width > rctDesktop.right && rctDesktop.right >= Width)
	{
		x = rctDesktop.right - Width;
	}
	// the following is in remark. Always display the window below the pointed rect
/*	if(y + Height > rctDesktop.bottom && rctDesktop.top + Height <= PointedRect.top)
	{
		y = PointedRect.top - Height;
	}
*/	SetRect(&rCalcRect, x, y, x + Width, y + Height);
	return true;
}
