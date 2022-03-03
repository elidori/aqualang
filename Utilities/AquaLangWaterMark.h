#ifndef _AQUALANG_WATER_MARK_HEADER_
#define _AQUALANG_WATER_MARK_HEADER_

class AquaLangWaterMark
{
public:
	AquaLangWaterMark();
	virtual ~AquaLangWaterMark();

	bool Open(LPCTSTR UserName, HWND hMarkedWnd);
	void Close();

	bool FindMarkInWindow(HWND hwnd, _tstring &rUserName);
private:
	static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);
	bool CheckWindow(HWND hwnd);
private:
	HWND m_hWaterMark;
	
	// intermediate parameters
	bool m_fFoundWaterMark;
	_tstring *m_pUserName;
};

#endif // _AQUALANG_WATER_MARK_HEADER_