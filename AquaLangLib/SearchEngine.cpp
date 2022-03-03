#include "Pch.h"
#include "SearchEngine.h"
#include "HttpClient.h"
#include "AquaLangConfiguration.h"

SearchEngine::SearchEngine(LPCSTR UrlPrefix, int ScrollDownSize)
	: m_UrlPrefix(UrlPrefix),
	m_ScrollDownSize(ScrollDownSize)
{
}

SearchEngine::~SearchEngine()
{
}

SearchEngine *SearchEngine::Create(int OptionCode)
{
	switch(OptionCode)
	{
	case WEB_SEARCH_GOOGLE:
		return new SearchEngine("http://www.google.com/search?hl=iw&q=", 0);
	case WEB_SEARCH_YAHOO:
		return new SearchEngine("http://search.yahoo.com/search?p=", 0);
	case WEB_SEARCH_BING:
		return new SearchEngine("http://www.bing.com/search?q=", 0);
	case WEB_SEARCH_ASK:
		return new SearchEngine("http://www.ask.com/web?q=", 0);
	}
	return NULL;
}

bool SearchEngine::GetUrlForSearchString(LPCTSTR SearchString, std::string &rURL) const
{
	if(SearchString == NULL || _tcslen(SearchString) == 0)
		return false;

	std::string EncodedText;
	if(HttpClient::URLEncode(SearchString, EncodedText) != HTTP_NO_ERROR)
		return false;

	rURL = m_UrlPrefix;
	rURL += EncodedText;
	return true;
}

void SearchEngine::Release()
{
	delete this;
}
