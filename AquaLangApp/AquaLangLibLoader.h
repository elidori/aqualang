#ifndef _AQUALANG_LIB_LOADER_HEADER_
#define _AQUALANG_LIB_LOADER_HEADER_

struct AquaLangConfiguration;
struct AquaLangStatistics;
class INotificationAreaRequestsListener;
typedef void *HSEQ;

class AquaLangLibLoader
{
public:
	AquaLangLibLoader();
	virtual ~AquaLangLibLoader();

	bool Open();
	void Close();

	bool OpenAquaLang(const char *UserName, const AquaLangConfiguration &rConfig, INotificationAreaRequestsListener &rListener, bool fRecovery);
	bool OpenAquaLang(const WCHAR *UserName, const AquaLangConfiguration &rConfig, INotificationAreaRequestsListener &rListener, bool fRecovery);
	void CloseAquaLang();
	bool RunBrowser(const char *BrowserString);
	bool RunBrowser(const WCHAR *BrowserString);
	bool TriggerCalculator();
	bool GetAquaLangStatistics(AquaLangStatistics &rStatistics);

	void PrintKey(int VirtualKey, bool fPress);

	bool CreateKeySequenceValidator(LPCSTR Sequence, HSEQ &hSequence);
	bool CreateKeySequenceValidator(LPCWSTR Sequence, HSEQ &hSequence);
	void DeleteKeySequenceValidator(HSEQ hSequence);
	bool AreSequencesEqual(HSEQ hSequence1, HSEQ hSequence2);
	bool IsContainedBySequence(HSEQ hReferenceSequence, HSEQ hTestedSequence);
private:
	typedef bool (__stdcall *T_OpenAquaLangA)(const char *UserName, const AquaLangConfiguration &rConfig, INotificationAreaRequestsListener &rListener, bool fRecovery);
	T_OpenAquaLangA FuncOpenAquaLangA;

	typedef bool (__stdcall *T_OpenAquaLangW)(const WCHAR *UserName, const AquaLangConfiguration &rConfig, INotificationAreaRequestsListener &rListener, bool fRecovery);
	T_OpenAquaLangW FuncOpenAquaLangW;

	typedef void (__stdcall *T_CloseAquaLang)();
	T_CloseAquaLang FuncCloseAquaLang;

	typedef bool (__stdcall *T_RunBrowserA)(const char *BrowserString);
	T_RunBrowserA FuncRunBrowserA;

	typedef bool (__stdcall *T_RunBrowserW)(const WCHAR *BrowserString);
	T_RunBrowserW FuncRunBrowserW;

	typedef bool (__stdcall *T_TriggerCalculator)();
	T_TriggerCalculator FuncTriggerCalculator;

	typedef bool (__stdcall *T_GetAquaLangStatistics)(AquaLangStatistics &rStatistics);
	T_GetAquaLangStatistics FuncGetAquaLangStatistics;

	typedef bool (__stdcall *T_RecoverAquaLang)();
	T_RecoverAquaLang FuncRecoverAquaLang;

	typedef bool (__stdcall *T_PrintKey)(int VirtualKey, bool fPress);
	T_PrintKey FuncPrintKey;

	typedef bool (__stdcall *T_CreateKeySequenceValidatorA)(LPCSTR Sequence, HSEQ &hSequence);
	T_CreateKeySequenceValidatorA FuncCreateKeySequenceValidatorA;

	typedef bool (__stdcall *T_CreateKeySequenceValidatorW)(LPCWSTR Sequence, HSEQ &hSequence);
	T_CreateKeySequenceValidatorW FuncCreateKeySequenceValidatorW;

	typedef void (__stdcall *T_DeleteKeySequenceValidator)(HSEQ hSequence);
	T_DeleteKeySequenceValidator FuncDeleteKeySequenceValidator;

	typedef bool (__stdcall *T_AreSequencesEqual)(HSEQ hSequence1, HSEQ hSequence2);
	T_AreSequencesEqual FuncAreSequencesEqual;

	typedef bool (__stdcall *T_IsContainedBySequence)(HSEQ hReferenceSequence, HSEQ hTestedSequence);
	T_IsContainedBySequence FuncIsContainedBySequence;
private:
	HINSTANCE m_hLib;
};

#endif // _AQUALANG_LIB_LOADER_HEADER_