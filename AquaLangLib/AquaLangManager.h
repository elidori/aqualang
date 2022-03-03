#ifndef _AQUA_LANG_MANAGER_HEADER_
#define _AQUA_LANG_MANAGER_HEADER_

#include "ThreadInstance.h"
#include "SyncHelpers.h"
#include "FocusChangeEvent.h"
#include "TextLanguageConverter.h"
#include "TextConvertAdvisor.h"
#include "LanguageHandler.h"
#include "AquaLangConfiguration.h"
#include "BalloonTip.h"
#include "UtilityWindow.h"
#include "KeyboardSequenceEvent.h"
#include "WebSearchHandler.h"
#include "UserInformationNotifier.h"
#include "CalculatorHandler.h"
#include "CalculatorPopupHandler.h"
#include "FocusInfo.h"
#include "LanguageStringTable.h"
#include "LanguageDetector.h"
#include "ClipboardSwapper.h"
#include "PsApiLoader.h"
#include "AsyncHookDispatcher.h"
#include "ILanguageModifier.h"
#include "GuessLanguageHandler.h"
#include "TipCoordinator.h"

class INotificationAreaRequestsListener;
typedef void *HSEQ;

class AquaLangManager :
	public ThreadInstance,
	public ILanguageModifier
{
public:
	AquaLangManager();
	virtual ~AquaLangManager();

	bool Open(LPCTSTR UserName, const AquaLangConfiguration &rConfig, INotificationAreaRequestsListener &rListener, bool fRecovery);
	void Close();

	bool RunBrowser(LPCTSTR BrowserString);
	bool TriggerCalculator();

	bool GetStatistics(AquaLangStatistics &rStatistics);

	void PrintKey(int VirtualKey, bool fPress);
	bool CreateKeySequenceValidator(LPCTSTR Sequence, HSEQ &hSequence);
	void DeleteKeySequenceValidator(HSEQ hSequence);
	bool AreSequencesEqual(HSEQ hSequence1, HSEQ hSequence2);
	bool IsContainedBySequence(HSEQ hReferenceSequence, HSEQ hTestedSequence);

	PsApiLoader &GetPsApiLoader()	{ return m_PsApiLoader; }
private:
	// ThreadInstance pure virtuals {
	virtual DWORD RunInThread();
	virtual void BreakThread();
	// }

	// ILanguageModifier pure virtuals {
	virtual void ChangeLanguage(HWND hwnd, HKL hTargetKL);
	virtual void ClearTextHistory();
	// }

	Result OpenHooks();
	void CloseHooks();
private:
	AquaLangConfiguration m_Config;
	_tstring m_UserName;

	MessageWindow m_MessageWindow; // used for getting a message before exiting

	FocusInfo m_FocusInfo;
	LanguageDetector m_LanguageDetector;

	AsyncHookDispatcher m_AsyncHookDispatcher;

	LanguageStringTable m_LanguageStringTable;
	FocusChangeEvent m_FocusChangeEvent;
	UserInformationNotifier m_UserInformationNotifier;
	LanguageHandler m_LanguageHandler;
	
	KeyboardSequenceEvent m_KeyboardSequenceEvent;
	TextConvertAdvisor m_TextConvertAdvisor;
	GuessLanguageHandler m_GuessLanguageHandler;
	TextLanguageConverter m_TextLanguageConverter;
	WebSearchHandler m_WebSearchHandler;
	CalculatorHandler m_CalculatorHandler;
	CalculatorPopupHandler m_CalculatorPopupHandler;
	ClipboardSwapper m_ClipboardSwapper;

	PsApiLoader m_PsApiLoader;

	typedef std::map<HSEQ, KeyboardSequence *> ValidatorMap;
	ValidatorMap m_ValidatorMap;

	TipCoordinator m_TipCoordinator;

	// intermediate
	bool m_fRecovered;
};

#endif // _AQUA_LANG_MANAGER_HEADER_