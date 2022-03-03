#include "Pch.h"
#include "StickyKeysEnabler.h"
#include "Log.h"

StickyKeysEnabler::StickyKeysEnabler()
	: m_fValid(false)
{
}

StickyKeysEnabler::~StickyKeysEnabler()
{
	Close();
}

bool StickyKeysEnabler::Open()
{
	if(!m_fValid)
	{
		STICKYKEYS StickyKeys;

		StickyKeys.cbSize = sizeof(StickyKeys);
		if(::SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(StickyKeys), &StickyKeys, 0))
		{
			m_dwOriginalFlags = StickyKeys.dwFlags;
			m_fValid = true;
		}
	}
	return m_fValid;
}

bool StickyKeysEnabler::Enable(bool fEnable)
{
	if(!m_fValid)
		return false;

	STICKYKEYS sk;
	sk.cbSize = sizeof(sk);
	sk.dwFlags = m_dwOriginalFlags;

	if(fEnable)
	{
		sk.dwFlags |= SKF_HOTKEYACTIVE;
		sk.dwFlags |= SKF_CONFIRMHOTKEY;
	}
	else
	{
		if((sk.dwFlags & SKF_STICKYKEYSON) != 0)
			return false; // the feature is already on, cannot disable it
		// Disable the hotkey and the confirmation
		sk.dwFlags &= ~SKF_HOTKEYACTIVE;
		sk.dwFlags &= ~SKF_CONFIRMHOTKEY;
	}
	if(!::SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0))
		return false;
	return true;
}

bool StickyKeysEnabler::Restore()
{
	if(!m_fValid)
		return false;

	STICKYKEYS sk;
	sk.cbSize = sizeof(sk);
	sk.dwFlags = m_dwOriginalFlags;

	if(!::SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0))
		return false;
	return true;
}

void StickyKeysEnabler::Close()
{
	Restore();
	m_fValid = false;
}
