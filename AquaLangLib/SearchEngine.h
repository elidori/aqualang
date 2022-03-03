#ifndef _SEARCH_ENGINE_HEADER_
#define _SEARCH_ENGINE_HEADER_

class SearchEngine
{
private:
	SearchEngine(LPCSTR UrlPrefix, int ScrollDownSize);
	virtual ~SearchEngine();
public:
	static SearchEngine *Create(int OptionCode);

	bool GetUrlForSearchString(LPCTSTR SearchString, std::string &rURL) const;
	int GetScrollDownSize() const	{ return m_ScrollDownSize; }

	void Release();
private:
	std::string m_UrlPrefix;
	const int m_ScrollDownSize;
};

#endif //_SEARCH_ENGINE_HEADER_