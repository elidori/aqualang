#ifndef _RECENT_TEXT_HEADER_
#define _RECENT_TEXT_HEADER_

#include "IndexMarker.h"
#include "Fifo.h"

class RecentText
{
public:
	RecentText();
	virtual ~RecentText();

	void Set(const RecentText &r);

	bool Open(int MaxCharacters);

	int GetTextMaxLength() const;

	void Clear(bool fAfterLatestNonPrintableKey = false);
	bool Update(int Key, bool fUpdateMark, HKL hKL);

	void MarkCurrentPosition();

	bool GetText(std::wstring &wText, HKL &rhTextKL, bool fAfterLatestNonPrintableKey = false) const;
	bool GetText(std::string &Text, HKL &rhTextKL, bool fAfterLatestNonPrintableKey = false) const;
private:
	Fifo<WCHAR> m_TextFifo;

	HKL m_hLatestKL;
};

#endif // _RECENT_TEXT_HEADER_