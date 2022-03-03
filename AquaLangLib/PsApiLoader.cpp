#include "Pch.h"
#include "PsApiLoader.h"

PsApiLoader::PsApiLoader()
	: m_hLib(NULL),
	FuncGetProcessImageFileNameW(NULL),
	FuncGetProcessImageFileNameA(NULL)
{
}

PsApiLoader::~PsApiLoader()
{
	Close();
}

bool PsApiLoader::Open()
{
	if(m_hLib != NULL)
		return true;

	m_hLib = ::LoadLibrary(_T("psapi.dll"));
	if(m_hLib == NULL)
		return false;

	FuncGetProcessImageFileNameW = (T_GetProcessImageFileNameW)::GetProcAddress(m_hLib, "GetProcessImageFileNameW");
	if(FuncGetProcessImageFileNameW == NULL)
	{
		Close();
		return false;
	}
	FuncGetProcessImageFileNameA = (T_GetProcessImageFileNameA)::GetProcAddress(m_hLib, "GetProcessImageFileNameA");
	if(FuncGetProcessImageFileNameA == NULL)
	{
		Close();
		return false;
	}
	return true;
}

bool PsApiLoader::IsOpen() const
{
	return (m_hLib != NULL);
}

void PsApiLoader::Close()
{
	FuncGetProcessImageFileNameW = NULL;
	FuncGetProcessImageFileNameA = NULL;
	if(m_hLib != NULL)
	{
		::FreeLibrary(m_hLib);
		m_hLib = NULL;
	}
}

DWORD PsApiLoader::GetProcessImageFileName(
					HANDLE hProcess,
					LPWSTR lpImageFileName,
					DWORD nSize
					)

{
	return (this->FuncGetProcessImageFileNameW)(
							hProcess,
							lpImageFileName,
							nSize
							);
}

DWORD PsApiLoader::GetProcessImageFileName(
					HANDLE hProcess,
					LPSTR lpImageFileName,
					DWORD nSize
					)

{
	return (this->FuncGetProcessImageFileNameA)(
							hProcess,
							lpImageFileName,
							nSize
							);
}
