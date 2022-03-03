#ifndef _TOOL_TIP_HEADER_
#define _TOOL_TIP_HEADER_

class ToolTip
{
public:
	ToolTip();
	virtual ~ToolTip();

	bool Open(HWND hDialog, HWND hControlWindow, LPCTSTR Text);
	void Close();
private:
	HWND m_hToolTip;
};

#endif // _TOOL_TIP_HEADER_