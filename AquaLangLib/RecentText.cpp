#include "Pch.h"
#include "RecentText.h"
#include "StringConvert.h"
#include "KeyCode.h"
#include "KeyboardMapper.h"

RecentText::RecentText()
	: m_hLatestKL(NULL)
{
}

RecentText::~RecentText()
{
	m_TextFifo.Close();
}

void RecentText::Set(const RecentText &r)
{
	m_TextFifo.Set(r.m_TextFifo);
	m_hLatestKL = r.m_hLatestKL;
}

bool RecentText::Open(int MaxCharacters)
{
	return m_TextFifo.Open(MaxCharacters);
}

int RecentText::GetTextMaxLength() const
{
	return m_TextFifo.GetMaxCount();
}

void RecentText::Clear(bool fAfterLatestNonPrintableKey /*= false*/)
{
	if(fAfterLatestNonPrintableKey)
	{
		int Offset = m_TextFifo.GetMarkOffset();
		m_TextFifo.UnPushValueList(m_TextFifo.GetCount() - Offset);
	}
	else
	{
		m_TextFifo.Clear();
	}
}

bool RecentText::Update(int Key, bool fUpdateMark, HKL hKL)
{
	m_hLatestKL = hKL;

	KeyCode _Key(Key);
	if(_Key.IsKeyPressed())
	{
		int VirtualKey = _Key.VirtualKey();
		bool fPrintable = _Key.IsPrintable();
		bool fShiftIsPressed = _Key.IsShiftDown();

		int PrintChars = KeyboardMapper::GetNumOfCharacters(VirtualKey);
		if(PrintChars == -1) // backspace
		{
			WCHAR wChar;
			m_TextFifo.UnPush(wChar);
		}
		else if(!fPrintable)
		{
			// this may be one of two cases:
			// 1. A part of the Hot Key, which will be cleared out later, so don;t clear the recent text
			// 2. Non relevant control characters. They will probably cause the user of this class to clear the recent text. Let them do it.
		}
		else if(PrintChars == 1)
		{
			assert(fPrintable);
			BYTE KeyState[256];
			memset(KeyState, 0, sizeof(KeyState));
			KeyState[VK_LSHIFT] = (fShiftIsPressed ? 0x80 : 0x00);
			KeyState[VK_SHIFT] = (fShiftIsPressed ? 0x80 : 0x00);

			WCHAR CharText[10];
			int nChars = ::ToUnicodeEx(
							VirtualKey,
							::MapVirtualKeyEx(VirtualKey, MAPVK_VK_TO_VSC, hKL),
							KeyState,
							CharText,
							sizeof(CharText)/sizeof(CharText[0]),
							0,
							hKL
							);
			assert(nChars < 2);
			if(nChars == 1)
			{
				m_TextFifo.Push(CharText[0], fUpdateMark, true);
			}
		}
	}
	return true;
}

void RecentText::MarkCurrentPosition()
{
	m_TextFifo.MarkCurrentPosition();
}

bool RecentText::GetText(std::wstring &wText, HKL &rhTextKL, bool fAfterLatestNonPrintableKey /*= false*/) const
{
	rhTextKL = m_hLatestKL;

	int CharCount = m_TextFifo.GetCount(fAfterLatestNonPrintableKey);
	if(CharCount == 0)
		return false;

	wText.reserve(CharCount + 1);
	wText.resize(CharCount);

	if(!m_TextFifo.GetList(&wText[0], CharCount, 0, fAfterLatestNonPrintableKey))
		return false;

	return true;
}

bool RecentText::GetText(std::string &Text, HKL &rhTextKL, bool fAfterLatestNonPrintableKey /*= false*/) const
{
	std::wstring wText;
	if(!GetText(wText, rhTextKL, fAfterLatestNonPrintableKey))
		return false;
	SetString(Text, wText.c_str());
	return true;
}
