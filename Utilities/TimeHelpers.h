#ifndef _TIME_HELPERS_HEADER_
#define _TIME_HELPERS_HEADER_

class Timer
{
public:
	Timer();
	virtual ~Timer() {}

	void Set(const Timer &r);
	void Restart();
	__int64 GetTime() const;
	__int64 GetDiff() const;
private:
    bool m_UsePerformanceCounter;
    LARGE_INTEGER m_PerformanceCounterResolution;
	__int64 m_StartTime;
};

class FileTime
{
public:
	FileTime(); // initializes to current system time
	virtual ~FileTime() {}

	void Init(const FILETIME &FileTime);
	bool Init(const SYSTEMTIME &SystemTime);
	void Init(__int64 FileTimeAsInt);

	const FILETIME &GetFILETIME() const	{ return m_Time; }
	__int64 GetFILETIMEasInt() const;
	bool GetSYSTEMTIME(SYSTEMTIME &rSysTime) const;


	void Add(__int64 TimeMsec);

	__int64 GetDiffMsec(const FileTime &r) const;
	__int64 GetDiffMsec(const FILETIME &r) const;
	bool GetDiffMsec(const SYSTEMTIME &r, __int64 &rDiff) const;
private:
	FILETIME m_Time;
};

#endif // _TIME_HELPERS_HEADER_