#include "Pch.h"
#include "TextDialog.h"

TextDialog::TextDialog(int DialogId, int TextId)
	: DialogObject(-1, false),
	m_DialogId(DialogId),
	m_TextId(TextId),
	m_BkColor(RGB(255, 255, 255)),
	m_BackgroundBrush(NULL)
{
}

TextDialog::~TextDialog()
{
	Close();
}

bool TextDialog::Open(LPCTSTR UserName, LPCTSTR Text, HWND hParent)
{
	if(GetDialogHandle() != NULL)
	{
		::SetFocus(GetDialogHandle());
	}
	else
	{
		if(m_BackgroundBrush == NULL)
		{
			m_BackgroundBrush = ::CreateSolidBrush(m_BkColor);
		}

		if(OpenDialog(UserName, m_DialogId, hParent) == NULL)
			return false;
	}

	::SetWindowText(::GetDlgItem(GetDialogHandle(), m_TextId), Text);
	::ShowWindow(GetDialogHandle(), SW_SHOW);
	return true;
}

void TextDialog::Close()
{
	CloseDialog();
	if(m_BackgroundBrush != NULL)
	{
		::DeleteObject(m_BackgroundBrush);
		m_BackgroundBrush = NULL;
	}
}

HWND TextDialog::GetItem(int Id)
{
	return ::GetDlgItem(GetDialogHandle(), Id);
}

LRESULT TextDialog::WindowProc(HWND hwnd, UINT uMsg, WPARAM UNUSED(wParam), LPARAM lParam, bool &fSkipDefault)
{
	fSkipDefault = false;
	LRESULT lResult = 0;

	switch(uMsg)
	{
	case WM_COMMAND:
		if((HWND)lParam == ::GetDlgItem(hwnd, IDOK))
		{
			Close();
		}
		break;
	}
	return lResult;
}

LRESULT TextDialog::LateWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fOverrideDefault)
{
	fOverrideDefault = false;
	LRESULT lResult = 0;

	switch(uMsg)
	{
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		if((HWND)lParam == ::GetDlgItem(hwnd, m_TextId))
		{
			if(m_BackgroundBrush != NULL)
			{
				HDC hDC = (HDC)wParam;
				::SetBkColor(hDC, m_BkColor);
				fOverrideDefault = true;
				lResult = (LRESULT)m_BackgroundBrush;
			}
		}
		break;
	case WM_SETFOCUS:
		::SendMessage(::GetDlgItem(hwnd, m_TextId), WM_KILLFOCUS, 0, 0);
		break;
	}

	return lResult;
}
