#include "Pch.h"
#include "TranslateEngine.h"
#include "HttpClient.h"
#include "AquaLangConfiguration.h"
#include "LanguageStringTable.h"
#include "StringConvert.h"

#define MAX_PAGE_NAME_LENGTH	1024	// Common practice

TranslateEngine::TranslateEngine(
						const LanguageStringTable &rLanguageStringTable,
						LPCSTR UrlPrefix,
						LPCSTR HostLanguageFormat,
						LPCSTR FromFormat,
						LPCSTR ToFormat,
						bool fFullLangName,
						int ScrollDownSize
						)
	: m_rLanguageStringTable(rLanguageStringTable),
	m_UrlPrefix(UrlPrefix),
	m_HostLanguageFormat(HostLanguageFormat),
	m_FromFormat(FromFormat),
	m_ToFormat(ToFormat),
	m_fFullLangName(fFullLangName),
	m_ScrollDownSize(ScrollDownSize)
{
}

TranslateEngine::~TranslateEngine()
{
}

TranslateEngine *TranslateEngine::Create(const LanguageStringTable &rLanguageStringTable, int OptionCode)
{
	switch(OptionCode)
	{
	case WEB_TRANSLATE_GOOGLE:
		return new TranslateEngine(rLanguageStringTable, "http://translate.google.com/?", "hl=%s&", "sl=auto&", "tl=%s#||", false, 160);
	case WEB_TRANSLATE_BABYLON:
		return new TranslateEngine(rLanguageStringTable, "http://translation.babylon.com/", "", "%s/", "to-%s/", true, 160);
	case WEB_TRANSLATE_BING:
		return new TranslateEngine(rLanguageStringTable, "http://www.microsofttranslator.com/?", "", "from=auto&", "to=%s&text=", false, 160);
	}
	return NULL;
}

bool TranslateEngine::GetUrlForTranslatedString(LPCTSTR TranslatedString, HKL hLangFrom, HKL hLangTo, std::string &rURL) const
{
	if(TranslatedString == NULL || _tcslen(TranslatedString) == 0)
		return false;

	// from
	_tstring FromStr;
	_tstring Dummy;
	if(!m_rLanguageStringTable.GetLanguageString(hLangFrom, Dummy, FromStr, m_fFullLangName ? LANGUAGE_FORMAT_FULL_NAME : LANGUAGE_FORMAT_SHORT_NAME))
		return false;
	std::string mbFromStr;
	SetString(mbFromStr, FromStr.c_str());
	char From[MAX_PAGE_NAME_LENGTH];
	sprintf_s(From, sizeof(From) / sizeof(From[0]), m_FromFormat.c_str(), mbFromStr.c_str());

	// to
	_tstring ToStr;
	if(!m_rLanguageStringTable.GetLanguageString(hLangTo, Dummy, ToStr, m_fFullLangName ? LANGUAGE_FORMAT_FULL_NAME : LANGUAGE_FORMAT_SHORT_NAME))
		return false;
	std::string mbToStr;
	SetString(mbToStr, ToStr.c_str());
	char To[MAX_PAGE_NAME_LENGTH];
	sprintf_s(To, sizeof(To) / sizeof(To[0]), m_ToFormat.c_str(), mbToStr.c_str());

	// host language
	char HostLanguage[MAX_PAGE_NAME_LENGTH];
	sprintf_s(HostLanguage, sizeof(HostLanguage) / sizeof(HostLanguage[0]), m_HostLanguageFormat.c_str(), mbToStr.c_str());

	// text
	std::string EncodedText;
	if(HttpClient::URLEncode(TranslatedString, EncodedText) != HTTP_NO_ERROR)
		return false;

	rURL = m_UrlPrefix + HostLanguage + From + To + EncodedText;
	return true;
}

void TranslateEngine::Release()
{
	delete this;
}
