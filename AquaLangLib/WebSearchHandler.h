#ifndef _WEB_SEARCH_HANDLER_HEADER_
#define _WEB_SEARCH_HANDLER_HEADER_

#include "IKeyboardEvent.h"
#include "WebBrowseRequest.h"

class KeyboardSequenceEvent;
class UserInformationNotifier;
class SearchEngine;
class TranslateEngine;
class LanguageStringTable;
class IKeyboardEventCallback;

class WebSearchHandler :
	public IKeyboardEvent
{
public:
	WebSearchHandler(KeyboardSequenceEvent &rKeyboardSequenceEvent, const LanguageStringTable &rLanguageStringTable, UserInformationNotifier &rUserInformationNotifier);
	virtual ~WebSearchHandler();

	bool Open(LPCTSTR UserName, LPCTSTR KeyboardTrigger, int SearchOptionCode, int TranslateOptionCode);
	void Close();
private:
	// IKeyboardEvent pure virtuals {
	virtual bool CanOperate(KeyboardEventInfo &Info, bool &fExclusive);
	virtual void Operate(const KeyboardEventInfo &Info, IKeyboardEventCallback &rCallback);
	// }
private:
	KeyboardSequenceEvent &m_rKeyboardSequenceEvent;
	const LanguageStringTable &m_rLanguageStringTable;
	UserInformationNotifier &m_rUserInformationNotifier;
	
	SearchEngine *m_pSearchEngine;
	TranslateEngine *m_pTranslateEngine;
	WebBrowseRequest m_Browser;
};

#endif // _WEB_SEARCH_HANDLER_HEADER_