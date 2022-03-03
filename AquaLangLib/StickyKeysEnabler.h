#ifndef _STICKY_KEYS_ENABLER_HEADER_
#define _STICKY_KEYS_ENABLER_HEADER_

class StickyKeysEnabler
{
public:
	StickyKeysEnabler();
	virtual ~StickyKeysEnabler();

	bool Open();

	bool Enable(bool fEnable);
	bool Restore();

	void Close();
private:
	bool m_fValid;
	DWORD m_dwOriginalFlags;
};

#endif // _STICKY_KEYS_ENABLER_HEADER_