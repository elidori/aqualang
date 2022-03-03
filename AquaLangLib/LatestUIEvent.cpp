#include "Pch.h"
#include "LatestUIEvent.h"
#include "KeyboardMapper.h"
#include "KeyCode.h"

LatestUIEvent::LatestUIEvent()
	: m_fValid(false),
	m_fInPrintMode(false)
{
}

LatestUIEvent::~LatestUIEvent()
{
}

void LatestUIEvent::Clear()
{
	m_fValid = false;
	m_fInPrintMode = false;
}

void LatestUIEvent::KeyboardEvent(int ComposedKeyList[], int ListSize)
{
	for(int i = 0; i < ListSize; i++)
	{
		KeyCode _Key(ComposedKeyList[i]);

		if(_Key.IsValid())
		{
			m_fValid = true;
			m_Time.Restart();
			m_uMsg = _Key.IsKeyPressed() ? WM_KEYDOWN : WM_KEYUP;
			m_KeyboardKey = _Key.Get();

			if(m_uMsg == WM_KEYDOWN)
			{
				if(_Key.IsPrintable())
				{
					if(!KeyboardMapper::IsShiftKey(_Key.VirtualKey()))
					{
						m_fInPrintMode = true;
						m_LatestPrintTime.Restart();
					}
				}
				else
				{
					m_fInPrintMode = false;
				}
			}
		}
	}
}

void LatestUIEvent::MouseEvent(UINT uMsg)
{
	m_fValid = true;
	m_Time.Restart();
	m_uMsg = uMsg;

	m_fInPrintMode = false;
}

bool LatestUIEvent::GetLatestEvent(__int64 &TimePassedSinceEvent, UINT &uMsg, int &KbKey) const
{
	if(!m_fValid)
		return false;

	TimePassedSinceEvent = m_Time.GetDiff();
	uMsg = m_uMsg;
	KbKey = m_KeyboardKey;
	return true;
}

bool LatestUIEvent::IsInPrintMode(__int64 &TimePassedSinceLastPrint) const
{
	if(!m_fInPrintMode)
		return false;
	TimePassedSinceLastPrint = m_LatestPrintTime.GetDiff();
	return true;
}
