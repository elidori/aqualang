#ifndef _MSIMG32_LOADER_HEADER_
#define _MSIMG32_LOADER_HEADER_

class MsImg32Loader
{
public:
	MsImg32Loader();
	virtual ~MsImg32Loader();

	bool Open();
	bool IsOpen() const;
	void Close();

	BOOL TransparentBlt(
				HDC hdcDest,
				int xoriginDest,
				int yoriginDest,
				int wDest,
				int hDest,
				HDC hdcSrc,
				int xoriginSrc,
				int yoriginSrc,
				int wSrc,
				int hSrc,
				UINT crTransparent
				);
private:
	typedef BOOL (WINAPI *T_TransparentBlt)(
				HDC hdcDest,
				int xoriginDest,
				int yoriginDest,
				int wDest,
				int hDest,
				HDC hdcSrc,
				int xoriginSrc,
				int yoriginSrc,
				int wSrc,
				int hSrc,
				UINT crTransparent
				);
	T_TransparentBlt FuncTransparentBlt;

	HINSTANCE m_hLib;
};

#endif // _MSIMG32_LOADER_HEADER_