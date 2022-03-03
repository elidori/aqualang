#include "Pch.h"
#include "MsImg32Loader.h"

MsImg32Loader::MsImg32Loader()
	: m_hLib(NULL),
	FuncTransparentBlt(NULL)
{
}

MsImg32Loader::~MsImg32Loader()
{
	Close();
}

bool MsImg32Loader::Open()
{
	if(m_hLib != NULL)
		return true;

	m_hLib = ::LoadLibrary(_T("msimg32.dll"));
	if(m_hLib == NULL)
		return false;

	FuncTransparentBlt = (T_TransparentBlt)::GetProcAddress(m_hLib, "TransparentBlt");
	if(FuncTransparentBlt == NULL)
	{
		Close();
		return false;
	}
	return true;
}

bool MsImg32Loader::IsOpen() const
{
	return (m_hLib != NULL);
}

void MsImg32Loader::Close()
{
	FuncTransparentBlt = NULL;
	if(m_hLib != NULL)
	{
		::FreeLibrary(m_hLib);
		m_hLib = NULL;
	}
}

BOOL MsImg32Loader::TransparentBlt(
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
				)
{
	return (this->FuncTransparentBlt)(
				hdcDest,
				xoriginDest,
				yoriginDest,
				wDest,
				hDest,
				hdcSrc,
				xoriginSrc,
				yoriginSrc,
				wSrc,
				hSrc,
				crTransparent
				);
}

