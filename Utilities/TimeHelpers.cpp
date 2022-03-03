#include "Pch.h"
#include "TimeHelpers.h"

#define msec_to_100nsec 10000

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Timer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Timer::Timer()
{
	if(::QueryPerformanceFrequency(&m_PerformanceCounterResolution))
	{
		m_UsePerformanceCounter = true;
		m_PerformanceCounterResolution.QuadPart /= 1000;
	}
	else
	{
		assert(false);
		m_UsePerformanceCounter = false;
	}
	Restart();
}

void Timer::Set(const Timer &r)
{
	m_UsePerformanceCounter = r.m_UsePerformanceCounter;
	m_PerformanceCounterResolution = r.m_PerformanceCounterResolution;
	m_StartTime = r.m_StartTime;
}

void Timer::Restart()
{
	m_StartTime = GetTime();
}

__int64 Timer::GetTime() const
{
	__int64 currentTime;

	if(m_UsePerformanceCounter)
	{
		LARGE_INTEGER largeTemp;
		::QueryPerformanceCounter(&largeTemp);
		currentTime = (largeTemp.QuadPart / m_PerformanceCounterResolution.QuadPart);
		return currentTime;
	}
	else
		return ::GetTickCount();
}
__int64 Timer::GetDiff() const
{
	return (GetTime() - m_StartTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FileTime
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FileTime::FileTime()
{
	::GetSystemTimeAsFileTime(&m_Time);
}

void FileTime::Init(const FILETIME &FileTime)
{
	m_Time = FileTime;
}

bool FileTime::Init(const SYSTEMTIME &SystemTime)
{
	if(!::SystemTimeToFileTime(&SystemTime, &m_Time))
		return false;
	return true;
}

void FileTime::Init(__int64 FileTimeAsInt)
{
	ULARGE_INTEGER Time;
	Time.QuadPart = FileTimeAsInt;
	m_Time.dwHighDateTime = Time.HighPart;
	m_Time.dwLowDateTime = Time.LowPart;
}

__int64 FileTime::GetFILETIMEasInt() const
{
	ULARGE_INTEGER Time;
	Time.HighPart = m_Time.dwHighDateTime;
	Time.LowPart = m_Time.dwLowDateTime;
	return Time.QuadPart;
}

bool FileTime::GetSYSTEMTIME(SYSTEMTIME &rSysTime) const
{
	if(!::FileTimeToSystemTime(&m_Time, &rSysTime))
		return false;
	return true;
}

void FileTime::Add(__int64 TimeMsec)
{
	ULARGE_INTEGER Time;
	Time.HighPart = m_Time.dwHighDateTime;
	Time.LowPart = m_Time.dwLowDateTime;

	Time.QuadPart += (ULONGLONG)TimeMsec * msec_to_100nsec;

	m_Time.dwHighDateTime = Time.HighPart;
	m_Time.dwLowDateTime = Time.LowPart;
}

__int64 FileTime::GetDiffMsec(const FileTime &r) const
{
	return GetDiffMsec(r.m_Time);
}

__int64 FileTime::GetDiffMsec(const FILETIME &r) const
{
	LARGE_INTEGER MyTime;
	MyTime.HighPart = m_Time.dwHighDateTime;
	MyTime.LowPart = m_Time.dwLowDateTime;

	LARGE_INTEGER RefTime;
	RefTime.HighPart = r.dwHighDateTime;
	RefTime.LowPart = r.dwLowDateTime;

	return (MyTime.QuadPart - RefTime.QuadPart) / msec_to_100nsec;
}

bool FileTime::GetDiffMsec(const SYSTEMTIME &r, __int64 &rDiff) const
{
	FileTime RefTime;
	if(!RefTime.Init(r))
		return false;
	rDiff = GetDiffMsec(RefTime);
	return true;
}
