#ifndef _SCRREN_FITTER_HEADER_
#define _SCRREN_FITTER_HEADER_

class ScreenFitter
{
public:
	static bool PlaceInScreen(int Width, int Height, const RECT &PointedRect, RECT &rCalcRect, int PointerWidthOffset = 0);
};

#endif // _SCRREN_FITTER_HEADER_