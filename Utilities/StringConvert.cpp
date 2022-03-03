#include "Pch.h"
#include "StringConvert.h"

void SetString(std::string &rTargetString, LPCSTR SourceString, size_t SourceStringSize /*= -1*/, int UNUSED(CodePage) /*= CP_UTF8*/)
{
	if(SourceString == NULL || SourceStringSize == 0)
	{
		rTargetString = "";
		return;
	}
	size_t len = strlen(SourceString);
	if(len >= SourceStringSize && SourceStringSize != -1)
	{
		len = SourceStringSize;
	}
	rTargetString.reserve(len + 1);
	rTargetString.resize(len);
	
	if(len > 0)
	{
		strncpy_s(&rTargetString[0], len + 1, SourceString, len);
	}
}

void SetString(std::string &rTargetString, LPCWSTR SourceString, size_t SourceStringSize /*= -1*/, int CodePage /*= CP_UTF8*/)
{
	if(SourceString == NULL || SourceStringSize == 0)
	{
		rTargetString = "";
		return;
	}
	size_t len = wcslen(SourceString);
	if(len >= SourceStringSize && SourceStringSize != -1)
	{
		len = SourceStringSize;
	}

	size_t TargetLen = WideCharToMultiByte(CodePage, 0, SourceString, (int)len, NULL, 0, NULL, NULL);
	rTargetString.reserve(TargetLen + 1);
	rTargetString.resize(TargetLen);

	if(TargetLen > 0)
	{
		WideCharToMultiByte(CodePage, 0, SourceString, (int)len, &rTargetString[0], (int)TargetLen, NULL, NULL);
	}
}

void SetString(std::wstring &rTargetString, LPCSTR SourceString, size_t SourceStringSize /*= -1*/, int CodePage /*= CP_UTF8*/)
{
	if(SourceString == NULL || SourceStringSize == 0)
	{
		rTargetString = L"";
		return;
	}
	size_t len = strlen(SourceString);
	if(len >= SourceStringSize && SourceStringSize != -1)
	{
		len = SourceStringSize;
	}

	size_t TargetLen = MultiByteToWideChar(CodePage, 0, SourceString, (int)len, NULL, 0);
	rTargetString.reserve(TargetLen + 1);
	rTargetString.resize(TargetLen);
	
	if(TargetLen > 0)
	{
		MultiByteToWideChar(CodePage, 0, SourceString, (int)len, &rTargetString[0], (int)TargetLen);
	}
}

void SetString(std::wstring &rTargetString, LPCWSTR SourceString, size_t SourceStringSize /*= -1*/, int UNUSED(CodePage) /*= CP_UTF8*/)
{
	if(SourceString == NULL || SourceStringSize == 0)
	{
		rTargetString = L"";
		return;
	}
	size_t len = wcslen(SourceString);
	if(len >= SourceStringSize && SourceStringSize != -1)
	{
		len = SourceStringSize;
	}

	rTargetString.reserve(len + 1);
	rTargetString.resize(len);
	
	if(len > 0)
	{
		wcsncpy_s(&rTargetString[0], len + 1, SourceString, len);
	}
}

void SetString(char *TargetString, size_t TargetSize, LPCSTR SourceString, size_t SourceStringSize /*= -1*/, int UNUSED(CodePage) /*= CP_UTF8*/)
{
	if(SourceString == NULL || SourceStringSize == 0)
	{
		strcpy_s(TargetString, TargetSize, "");
		return;
	}
	size_t len = strlen(SourceString);
	if(len >= SourceStringSize && SourceStringSize != -1)
	{
		len = SourceStringSize;
	}
	if(len >= TargetSize)
	{
		strcpy_s(TargetString, TargetSize, "");
		return;
	}

	strncpy_s(TargetString, TargetSize, SourceString, len);
	TargetString[len] = '\0';
}

void SetString(char *TargetString, size_t TargetSize, LPCWSTR SourceString, size_t SourceStringSize /*= -1*/, int CodePage /*= CP_UTF8*/)
{
	if(SourceString == NULL || SourceStringSize == 0)
	{
		strcpy_s(TargetString, TargetSize, "");
		return;
	}
	size_t len = wcslen(SourceString);
	if(len >= SourceStringSize && SourceStringSize != -1)
	{
		len = SourceStringSize;
	}

	size_t TargetLen = WideCharToMultiByte(CodePage, 0, SourceString, (int)len, NULL, 0, NULL, NULL);
	if(TargetLen >= TargetSize)
	{
		strcpy_s(TargetString, TargetSize, "");
		return;
	}

	WideCharToMultiByte(CodePage, 0, SourceString, (int)len, TargetString, (int)TargetLen, NULL, NULL);
	TargetString[TargetLen] = '\0';
}

void SetString(WCHAR *TargetString, size_t TargetSize, LPCSTR SourceString, size_t SourceStringSize /*= -1*/, int CodePage /*= CP_UTF8*/)
{
	if(SourceString == NULL || SourceStringSize == 0)
	{
		wcscpy_s(TargetString, TargetSize, L"");
		return;
	}
	size_t len = strlen(SourceString);
	if(len >= SourceStringSize && SourceStringSize != -1)
	{
		len = SourceStringSize;
	}

	size_t TargetLen = MultiByteToWideChar(CodePage, 0, SourceString, (int)len, NULL, 0);
	if(TargetLen >= TargetSize)
	{
		wcscpy_s(TargetString, TargetSize, L"");
		return;
	}

	MultiByteToWideChar(CodePage, 0, SourceString, (int)len, TargetString, (int)TargetLen);
	TargetString[len] = L'\0';
}

void SetString(WCHAR *TargetString, size_t TargetSize, LPCWSTR SourceString, size_t SourceStringSize /*= -1*/, int UNUSED(CodePage) /*= CP_UTF8*/)
{
	if(SourceString == NULL || SourceStringSize == 0)
	{
		wcscpy_s(TargetString, TargetSize, L"");
		return;
	}
	size_t len = wcslen(SourceString);
	if(len >= SourceStringSize && SourceStringSize != -1)
	{
		len = SourceStringSize;
	}
	if(len >= TargetSize)
	{
		wcscpy_s(TargetString, TargetSize, L"");
		return;
	}

	wcsncpy_s(TargetString, len + 1, SourceString, len);
	TargetString[len] = L'\0';
}
