#ifndef _PS_API_LOADER_HEADER_
#define _PS_API_LOADER_HEADER_

class PsApiLoader
{
public:
	PsApiLoader();
	virtual ~PsApiLoader();

	bool Open();
	bool IsOpen() const;
	void Close();

	DWORD GetProcessImageFileName(
					HANDLE hProcess,
					LPWSTR lpImageFileName,
					DWORD nSize
					);
	DWORD GetProcessImageFileName(
					HANDLE hProcess,
					LPSTR lpImageFileName,
					DWORD nSize
					);
private:
	typedef DWORD (WINAPI *T_GetProcessImageFileNameW)(
					HANDLE hProcess,
					LPWSTR lpImageFileName,
					DWORD nSize
					);
	T_GetProcessImageFileNameW FuncGetProcessImageFileNameW;

	typedef DWORD (WINAPI *T_GetProcessImageFileNameA)(
					HANDLE hProcess,
					LPSTR lpImageFileName,
					DWORD nSize
					);
	T_GetProcessImageFileNameA FuncGetProcessImageFileNameA;

	HINSTANCE m_hLib;
};

PsApiLoader &GetPsApiLoader();

#endif //_PS_API_LOADER_HEADER_