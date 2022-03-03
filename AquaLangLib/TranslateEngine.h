#ifndef _TRANSLATE_ENGINE_HEADER_
#define _TRANSLATE_ENGINE_HEADER_

class LanguageStringTable;

class TranslateEngine
{
private:
	TranslateEngine(
				const LanguageStringTable &rLanguageStringTable,
				LPCSTR UrlPrefix,
				LPCSTR HostLanguageFormat,
				LPCSTR FromFormat,
				LPCSTR ToFormat,
				bool fFullLangName,
				int ScrollDownSize
				);
	virtual ~TranslateEngine();
public:
	static TranslateEngine *Create(const LanguageStringTable &rLanguageStringTable, int OptionCode);

	bool GetUrlForTranslatedString(LPCTSTR TranslatedString, HKL hLangFrom, HKL hLangTo, std::string &rURL) const;
	int GetScrollDownSize() const	{ return m_ScrollDownSize; }

	void Release();
private:
	const LanguageStringTable &m_rLanguageStringTable;

	std::string m_UrlPrefix;
	std::string m_HostLanguageFormat;
	std::string m_FromFormat;
	std::string m_ToFormat;
	const bool m_fFullLangName;

	const int m_ScrollDownSize;
};

#endif //_TRANSLATE_ENGINE_HEADER_
