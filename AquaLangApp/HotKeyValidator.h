#ifndef _HOT_KEY_VALIDATOR_HEADER_
#define _HOT_KEY_VALIDATOR_HEADER_

class AquaLangLibLoader;
typedef void *HSEQ;

struct HotKeyInfo
{
	HSEQ Validator;
	bool fActive;
	_tstring DebugString;
};

class HotKeyValidator
{
public:
	HotKeyValidator(AquaLangLibLoader &rAquaLangLib);
	virtual ~HotKeyValidator();

	bool AddHotKey(LPCSTR HotKey, bool fActive, LPCTSTR DebugString, _tstring &rErrorMessage);
	void Close();
private:
	AquaLangLibLoader &m_rAquaLangLib;

	std::vector<HotKeyInfo *> m_HotKeyList;
};

#endif // _HOT_KEY_VALIDATOR_HEADER_