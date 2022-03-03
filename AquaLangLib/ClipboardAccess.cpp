#include "Pch.h"
#include "ClipboardAccess.h"
#include "KeyboardSequence.h"

///////////////////////////////////////////////////////////////////////////////////////
// ClipboardData
///////////////////////////////////////////////////////////////////////////////////////
ClipboardData::ClipboardData()
{
}

ClipboardData::~ClipboardData()
{
	Close();
}

bool ClipboardData::Push(UINT Format, HGLOBAL hglb)
{
	size_t MemSize = ::GlobalSize(hglb);
	if(MemSize == 0)
		return false;

	LPVOID pMem = ::GlobalLock(hglb);
	if(pMem == NULL)
		return false;

	bool fSuccess = false;
	HGLOBAL hCopyGlb = ::GlobalAlloc(GMEM_MOVEABLE, MemSize);
	if(hCopyGlb != NULL)
	{
		LPVOID pDest = ::GlobalLock(hCopyGlb);
		if(pDest != NULL)
		{
			memcpy(pDest, pMem, MemSize);
			::GlobalUnlock(hCopyGlb);

			ClipboardDataItem NewItem;
			NewItem.Format = Format;
			NewItem.hglbCopy = hCopyGlb;
			m_DataList.push_back(NewItem);

			fSuccess = true;
		}
		else
		{
			::GlobalFree(hCopyGlb);
		}
	}
	::GlobalUnlock(hglb);

	return fSuccess;
}

bool ClipboardData::Pop(ClipboardDataItem &rItem)
{
	if(m_DataList.size() == 0)
		return false;
	rItem = m_DataList.at(0);
	m_DataList.erase(m_DataList.begin());
	return true;
}

void ClipboardData::Close()
{
	for(unsigned int i = 0; i < m_DataList.size(); i++)
	{
		::GlobalFree(m_DataList.at(i).hglbCopy);
	}
	m_DataList.erase(m_DataList.begin(), m_DataList.end());
}

///////////////////////////////////////////////////////////////////////////////////////
// ClipboardAccess
///////////////////////////////////////////////////////////////////////////////////////
bool ClipboardAccess::GetText(HWND &rhwnd, std::wstring &wText)
{
	rhwnd = NULL;
	wText = L"";

	rhwnd = ::GetClipboardOwner();
	if(rhwnd == NULL)
		return false;

	if(!::IsClipboardFormatAvailable(CF_UNICODETEXT))
		return false;

	bool fSuccess = false;
	if(::OpenClipboard(rhwnd))
	{
		HGLOBAL hglb = ::GetClipboardData(CF_UNICODETEXT);
		if(hglb != NULL)
		{
			LPWSTR lpwstr = (LPWSTR)::GlobalLock(hglb);
			if(lpwstr != NULL)
			{
				wText = lpwstr;
				::GlobalUnlock(hglb);
				fSuccess = true;
			}
		}
		::CloseClipboard();
	}
	return fSuccess;
}

bool ClipboardAccess::GetText(HWND &rhwnd, std::string &Text)
{
	rhwnd = NULL;
	Text = "";

	rhwnd = ::GetClipboardOwner();
	if(rhwnd == NULL)
		return false;

	if(!::IsClipboardFormatAvailable(CF_TEXT))
		return false;

	bool fSuccess = false;
	if(::OpenClipboard(rhwnd))
	{
		HGLOBAL hglb = ::GetClipboardData(CF_TEXT);
		if(hglb != NULL)
		{
			LPSTR lpstr = (LPSTR)::GlobalLock(hglb);
			if(lpstr != NULL)
			{
				Text = lpstr;
				::GlobalUnlock(hglb);
				fSuccess = true;
			}
		}
		::CloseClipboard();
	}
	return fSuccess;
}

bool ClipboardAccess::SetText(HWND hwnd, LPCWSTR wText, HKL hKL /*= NULL*/)
{
	bool fSuccess = false;
	if(::OpenClipboard(hwnd))
	{
		if(wText == NULL || wcslen(wText) == 0)
		{
			fSuccess = (::EmptyClipboard() != FALSE);
		}
		else
		{
			size_t len = wcslen(wText) + 1;
			HGLOBAL hglbCopy = ::GlobalAlloc(GMEM_MOVEABLE, len * sizeof(WCHAR)); 
			if(hglbCopy != NULL)
			{
				LPWSTR lpwstrCopy = (LPWSTR)::GlobalLock(hglbCopy);
				if(lpwstrCopy)
				{
					wcscpy_s(lpwstrCopy, len, wText);

					::GlobalUnlock(hglbCopy);

					::EmptyClipboard();
					if(::SetClipboardData(CF_UNICODETEXT, hglbCopy) != NULL)
					{
						fSuccess = true;
					}
				}
				if(!fSuccess)
				{
					::GlobalFree(hglbCopy);
				}
 			}
			// insert locale information if exists
			if(fSuccess && hKL != NULL)
			{
				bool lcidSuccess = false;
				hglbCopy = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(LCID)); 
				if(hglbCopy != NULL)
				{
					LCID *pLocaleId = (LCID *)::GlobalLock(hglbCopy);
					if(pLocaleId)
					{
						*pLocaleId = ConvertKeyboardLayoutToLocale(hKL);

						::GlobalUnlock(hglbCopy);

						if(::SetClipboardData(CF_LOCALE, hglbCopy) != NULL)
						{
							lcidSuccess = true;
						}
					}
					if(!lcidSuccess)
					{
						::GlobalFree(hglbCopy);
					}
 				}
			}
		}
		::CloseClipboard();
	}
	return fSuccess;
}

bool ClipboardAccess::PeekMarkedText(HWND hwnd, std::wstring &wText)
{
	bool fUseWindowMessage = CanSendWindowMessage(hwnd);

	// first get the current information in the clipboard
	HWND hOwnerWindow;
	ClipboardData OriginalData;
	ClipboardAccess::GetData(hOwnerWindow, OriginalData);

	// empty clipboard to see if the copy action brings anything
	ClipboardAccess::SetText(hwnd, NULL);

	// copy text to clipboard - either by sending WM or by pressing CTRL+C
	if(fUseWindowMessage)
	{
		::SendMessage(hwnd, WM_COPY, 0, 0);
	}
	else
	{
		KeyboardSequence::PrintKey(VK_LCONTROL, true, NULL, false, NULL);
		KeyboardSequence::PrintKey('C', true, NULL, false, NULL);
		KeyboardSequence::PrintKey('C', false, NULL, false, NULL);
		KeyboardSequence::PrintKey(VK_LCONTROL, false, NULL, false, NULL);
		Sleep(100);
	}

	bool fFound = false;
	// get text from clipboard
	if(ClipboardAccess::GetText(hwnd, wText))
	{
		if(wText.length() > 0)
		{
			fFound = true;
		}
	}

	// restore the original information
	ClipboardAccess::SetData(hOwnerWindow, OriginalData);
	return fFound;
}

bool ClipboardAccess::CutAndToggelWithClipboard(HWND hwnd)
{
	bool fUseWindowMessage = CanSendWindowMessage(hwnd);

	bool fSuccess = true;
	// first get the current information from the clipboard
	HWND hOwnerWindow;
	ClipboardData OriginalData;
	fSuccess &= ClipboardAccess::GetData(hOwnerWindow, OriginalData);

	// empty clipboard to see if the copy action brings anything
	fSuccess &= ClipboardAccess::SetText(hwnd, NULL);

	// cut the current information
	if(fUseWindowMessage)
	{
		::SendMessage(hwnd, WM_CUT, 0, 0);
	}
	else
	{
		KeyboardSequence::PrintKey(VK_LCONTROL, true, NULL, false, NULL);
		KeyboardSequence::PrintKey('X', true, NULL, false, NULL);
		KeyboardSequence::PrintKey('X', false, NULL, false, NULL);
		KeyboardSequence::PrintKey(VK_LCONTROL, false, NULL, false, NULL);
		Sleep(100);
	}

	// get the clipboard information and put in ClipboardData object #2
	HWND hNewOwnerWindow;
	ClipboardData NewData;
	fSuccess &= ClipboardAccess::GetData(hNewOwnerWindow, NewData);

	// put into the clipboard the first Clipboard data object
	fSuccess &= ClipboardAccess::SetData(hOwnerWindow, OriginalData);

	// paste the data from the clipboard
	if(fUseWindowMessage)
	{
		::SendMessage(hwnd, WM_PASTE, 0, 0);
	}
	else
	{
		KeyboardSequence::PrintKey(VK_LCONTROL, true, NULL, false, NULL);
		KeyboardSequence::PrintKey('V', true, NULL, false, NULL);
		KeyboardSequence::PrintKey('V', false, NULL, false, NULL);
		KeyboardSequence::PrintKey(VK_LCONTROL, false, NULL, false, NULL);
		Sleep(100);
	}

	// put into the clipboard the new Clipboard data object
	fSuccess &= ClipboardAccess::SetData(hNewOwnerWindow, NewData);
	return fSuccess;
}

bool ClipboardAccess::GetData(HWND &rhwnd, ClipboardData &rData)
{
	rData.Close();

	rhwnd = ::GetClipboardOwner();
	if(rhwnd == NULL)
		return false;

	bool fSuccess = false;
	if(::OpenClipboard(rhwnd))
	{
		UINT Format = 0;
		while((Format = ::EnumClipboardFormats(Format)) != 0)
		{
			HGLOBAL hglb = ::GetClipboardData(Format);
			if(hglb != NULL)
			{
				if(rData.Push(Format, hglb))
				{
					fSuccess = true;
				}
			}
		}
		::CloseClipboard();
	}
	return fSuccess;
}

bool ClipboardAccess::SetData(HWND hwnd, ClipboardData &rData)
{
	if(!::OpenClipboard(hwnd))
		return false;
	if(!::EmptyClipboard())
		return false;

	bool fSuccess = true;
	ClipboardDataItem DataItem;
	while(rData.Pop(DataItem))
	{
		if(::SetClipboardData(DataItem.Format, DataItem.hglbCopy) == NULL)
		{
			::GlobalFree(DataItem.hglbCopy);
			fSuccess = false;
		}
	}

	rData.Close();
	::CloseClipboard();
	return fSuccess;
}

bool ClipboardAccess::PasteText(HWND hwnd, LPCWSTR Text)
{
	bool fUseWindowMessage = CanSendWindowMessage(hwnd);

	bool fSuccess = true;
	// first get the current information from the clipboard
	HWND hOwnerWindow;
	ClipboardData OriginalData;
	fSuccess &= ClipboardAccess::GetData(hOwnerWindow, OriginalData);

	// Set the text for pasting
	fSuccess &= ClipboardAccess::SetText(hwnd, Text);

	// paste the text from the clipboard
	if(fUseWindowMessage)
	{
		::SendMessage(hwnd, WM_PASTE, 0, 0);
	}
	else
	{
		KeyboardSequence::PrintKey(VK_LCONTROL, true, false, NULL, NULL);
		KeyboardSequence::PrintKey('V', true, false, NULL, NULL);
		KeyboardSequence::PrintKey('V', false, false, NULL, NULL);
		KeyboardSequence::PrintKey(VK_LCONTROL, false, false, NULL, NULL);
		Sleep(100);
	}

	// put into the clipboard the original data object
	fSuccess &= ClipboardAccess::SetData(hOwnerWindow, OriginalData);

	return fSuccess;
}

LCID ClipboardAccess::ConvertKeyboardLayoutToLocale(HKL hKL)
{
	// Eli Dori - this is a patch. Need to find a standard way to do this
	return (LCID)((int)hKL & 0xFFFF);
}

bool ClipboardAccess::CanSendWindowMessage(HWND hwnd)
{
	TCHAR ClassName[256];
	if(::RealGetWindowClass(hwnd, ClassName, sizeof(ClassName) / sizeof(ClassName[0])) == 0)
		return false;

	if(_tcscmp(ClassName, WC_EDIT) == 0)
		return true;

	if(_tcscmp(ClassName, WC_COMBOBOX) == 0 || _tcscmp(ClassName, WC_COMBOBOXEX) == 0)
	{
		if((::GetWindowLong(hwnd, GWL_STYLE) & CBS_DROPDOWNLIST) == 0)
			return true;
	}
	return false;
}