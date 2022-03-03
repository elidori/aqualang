#ifndef _AQUALANG_API_HEADER_
#define _AQUALANG_API_HEADER_

struct AquaLangConfiguration;
struct AquaLangStatistics;
class INotificationAreaRequestsListener;
typedef void *HSEQ;

extern "C"
{
	bool __stdcall OpenAquaLangA(const char *UserName, const AquaLangConfiguration &rConfig, INotificationAreaRequestsListener &rListener, bool fRecovery);
	bool __stdcall OpenAquaLangW(const WCHAR *UserName, const AquaLangConfiguration &rConfig, INotificationAreaRequestsListener &rListener, bool fRecovery);
	void __stdcall CloseAquaLang();

	bool __stdcall RunBrowserA(const char *BrowserString);
	bool __stdcall RunBrowserW(const WCHAR *BrowserString);

	bool __stdcall TriggerCalculator();

	bool __stdcall GetAquaLangStatistics(AquaLangStatistics &rStatistics);

	// keyboard utilities
	void __stdcall PrintKey(int VirtualKey, bool fPress);

	bool __stdcall CreateKeySequenceValidatorA(LPCSTR Sequence, HSEQ &hSequence);
	bool __stdcall CreateKeySequenceValidatorW(LPCWSTR Sequence, HSEQ &hSequence);
	void __stdcall DeleteKeySequenceValidator(HSEQ hSequence);
	bool __stdcall AreSequencesEqual(HSEQ hSequence1, HSEQ hSequence2);
	bool __stdcall IsContainedBySequence(HSEQ hReferenceSequence, HSEQ hTestedSequence);
}

#endif // _AQUALANG_API_HEADER_