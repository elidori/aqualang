#include "Pch.h"
#include "AquaLangWaterMark.h"
#include "Log.h"

extern HMODULE g_hModule;

#define AQUALANG_WATERMARK_STRING _T("AquaLang Watermark")

AquaLangWaterMark::AquaLangWaterMark()
	: m_hWaterMark(NULL),
	m_fFoundWaterMark(false)
{
}

AquaLangWaterMark::~AquaLangWaterMark()
{
	Close();
}

bool AquaLangWaterMark::Open(LPCTSTR UserName, HWND hMarkedWnd)
{
	_tstring Title = AQUALANG_WATERMARK_STRING;
	Title += _T("@");
	Title += UserName;

	if(m_hWaterMark != NULL)
		return false;
	m_hWaterMark = CreateWindowEx(
								0,
								_T("BUTTON"),
								Title.c_str(),
								WS_CHILD,
								0,
								0,
								0,
								0,
								hMarkedWnd,
								NULL,
								g_hModule,
								NULL);
	if(!m_hWaterMark)
		return false;

	return true;
}

void AquaLangWaterMark::Close()
{
	if(m_hWaterMark != NULL)
	{
		::DestroyWindow(m_hWaterMark);
		m_hWaterMark = NULL;
	}
	m_fFoundWaterMark = false;
}

bool AquaLangWaterMark::FindMarkInWindow(HWND hwnd, _tstring &rUserName)
{
	if(hwnd == NULL)
		return false;

	m_fFoundWaterMark = false;
	m_pUserName = &rUserName;
	::EnumChildWindows(hwnd, EnumChildProc, (LPARAM)this);
	return m_fFoundWaterMark;
}

BOOL CALLBACK AquaLangWaterMark::EnumChildProc(HWND hwnd, LPARAM lParam)
{
	AquaLangWaterMark *pThis = (AquaLangWaterMark *)lParam;
	if(pThis == NULL)
		return FALSE;
	if(pThis->CheckWindow(hwnd))
		return FALSE; // found it, stop enumaration
	return TRUE;
}

bool AquaLangWaterMark::CheckWindow(HWND hwnd)
{
	TCHAR Text[256];
	if(::GetWindowText(hwnd, Text, sizeof(Text) / sizeof(Text[0])) != 0)
	{
		if(_tcsncmp(Text, AQUALANG_WATERMARK_STRING, _tcslen(AQUALANG_WATERMARK_STRING)) == 0)
		{
			m_fFoundWaterMark = true;
			assert(m_pUserName != NULL);
			*m_pUserName = &Text[_tcslen(AQUALANG_WATERMARK_STRING) + 1]; // 1 is for the '@'
			return true;
		}
	}
	return false;
}
