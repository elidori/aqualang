#include "Pch.h"

void GetTime(LPSTR String, int StringSize)
{
	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	sprintf_s(
			String, StringSize,
			"%d\\%d\\%d %d:%02d:%02d.%03d - ",
			LocalTime.wDay,
			LocalTime.wMonth,
			LocalTime.wYear,
			LocalTime.wHour,
			LocalTime.wMinute,
			LocalTime.wSecond,
			LocalTime.wMilliseconds
			);
}

void GetTime(LPWSTR String, int StringSize)
{
	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	swprintf_s(
			String, StringSize,
			L"%d\\%d\\%d %d:%02d:%02d.%03d - ",
			LocalTime.wDay,
			LocalTime.wMonth,
			LocalTime.wYear,
			LocalTime.wHour,
			LocalTime.wMinute,
			LocalTime.wSecond,
			LocalTime.wMilliseconds
			);
}

void Log(LPCTSTR lpszFormat, ...)
{
    va_list args;
    va_start(args, lpszFormat);

    TCHAR szBuffer[1024];

	GetTime(szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]));
	size_t BufferLength = _tcslen(szBuffer);
    _vstprintf_s(szBuffer + BufferLength, sizeof(szBuffer)/sizeof(szBuffer[0]) - BufferLength, lpszFormat, args);

	OutputDebugString(szBuffer);

    va_end(args);
}

void LogA(LPCSTR lpszFormat, ...)
{
    va_list args;
    va_start(args, lpszFormat);

    CHAR szBuffer[1024];

	GetTime(szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]));
	size_t BufferLength = strlen(szBuffer);
    vsprintf_s(szBuffer + BufferLength, sizeof(szBuffer)/sizeof(szBuffer[0]) - BufferLength, lpszFormat, args);

	OutputDebugStringA(szBuffer);

    va_end(args);
}

void LogW(LPCWSTR lpszFormat, ...)
{
    va_list args;
    va_start(args, lpszFormat);

    WCHAR szBuffer[1024];

	GetTime(szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]));
	size_t BufferLength = wcslen(szBuffer);
    vswprintf_s(szBuffer + BufferLength, sizeof(szBuffer)/sizeof(szBuffer[0]) - BufferLength, lpszFormat, args);

	OutputDebugStringW(szBuffer);

    va_end(args);
}
