#include "Pch.h"
#include "WebSearchHandler.h"
#include "WinHookResult.h"
#include "UtilityWindowParams.h"
#include "KeyboardSequenceEvent.h"
#include "HttpClient.h"
#include "StringConvert.h"
#include "SearchEngine.h"
#include "TranslateEngine.h"
#include "UserInformationNotifier.h"
#include "KeyboardEventInfo.h"
#include "KeyboardMapper.h"
#include "StringSizeLimits.h"
#include "Log.h"

#define BROWSER_VERT_MARGIN_TO_TEXT 6

WebSearchHandler::WebSearchHandler(KeyboardSequenceEvent &rKeyboardSequenceEvent, const LanguageStringTable &rLanguageStringTable, UserInformationNotifier &rUserInformationNotifier)
	: m_rKeyboardSequenceEvent(rKeyboardSequenceEvent),
	m_rLanguageStringTable(rLanguageStringTable),
	m_rUserInformationNotifier(rUserInformationNotifier),
	m_pSearchEngine(NULL),
	m_pTranslateEngine(NULL),
	m_Browser()
{
}

WebSearchHandler::~WebSearchHandler()
{
	Close();
}

bool WebSearchHandler::Open(LPCTSTR UserName, LPCTSTR KeyboardTrigger, int SearchOptionCode, int TranslateOptionCode)
{
	Result res = m_rKeyboardSequenceEvent.Register(KeyboardTrigger, *this, false);
	if(res != Result_Success)
		return false;

	m_pSearchEngine = SearchEngine::Create(SearchOptionCode);
	if(!m_pSearchEngine)
		return false;

	m_pTranslateEngine = TranslateEngine::Create(m_rLanguageStringTable, TranslateOptionCode);
	if(!m_pTranslateEngine)
		return false;

	m_Browser.Open(UserName);

	return true;
}

void WebSearchHandler::Close()
{
	if(m_pSearchEngine)
	{
		m_pSearchEngine->Release();
		m_pSearchEngine = NULL;
	}
	if(m_pTranslateEngine)
	{
		m_pTranslateEngine->Release();
		m_pTranslateEngine = NULL;
	}
	m_Browser.Close();
	m_rKeyboardSequenceEvent.Unregister(*this);
}

// collect needed information before deciding
bool WebSearchHandler::CanOperate(KeyboardEventInfo &Info, bool &fExclusive)
{
	fExclusive = false;

	Info.PeekAndLoadMarkedText();
	return true;
}

void WebSearchHandler::Operate(const KeyboardEventInfo &Info, IKeyboardEventCallback &rCallback)
{
	static const bool fExcludeOlderText = true;

	bool fComplete = false;

	_tstring SearchText;
	HKL hTextKL;
	if(Info.GetMarkedOrRecentText(SearchText, hTextKL, fExcludeOlderText))
	{
		bool fCanContinue = true;

		if(SearchText.length() > MAX_WEB_STRING_SIZE)
		{
			SearchText = SearchText.substr(SearchText.length() - MAX_WEB_STRING_SIZE, MAX_WEB_STRING_SIZE);
		}

		std::string SearchUrl;
		if(!m_pSearchEngine || !m_pSearchEngine->GetUrlForSearchString(SearchText.c_str(), SearchUrl))
		{
			fCanContinue = false;
		}

		KeyboardMapper _KeyboardMapper;
		std::string TranslateUrl;
		if(!m_pTranslateEngine || !m_pTranslateEngine->GetUrlForTranslatedString(SearchText.c_str(), hTextKL, _KeyboardMapper.ToggleLayout(hTextKL), TranslateUrl))
		{
			fCanContinue = false;
		}

		if(fCanContinue)
		{
			RECT CaretRect;
			int RectType;
			if(Info.QueryWorkingRect(CaretRect, RectType))
			{
				CaretRect.top -= BROWSER_VERT_MARGIN_TO_TEXT;
				CaretRect.bottom += BROWSER_VERT_MARGIN_TO_TEXT;
				m_Browser.Browse(
						SearchUrl.c_str(),
						m_pSearchEngine->GetScrollDownSize(),
						TranslateUrl.c_str(),
						m_pTranslateEngine->GetScrollDownSize(),
						SearchText.c_str(),
						CaretRect
						);
			}
		}
		fComplete = true;
		m_rUserInformationNotifier.IncreaseWebSearchEvents();
	}
	rCallback.EventOperationEnded(fComplete);
}
