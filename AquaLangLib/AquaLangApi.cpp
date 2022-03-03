#include "Pch.h"
#include "AquaLangApi.h"
#include "AquaLangManager.h"
#include "StringConvert.h"

AquaLangManager g_AquaLangManager;

extern "C"
bool __stdcall OpenAquaLangA(const char *UserName, const AquaLangConfiguration &rConfig, INotificationAreaRequestsListener &rListener, bool fRecovery)
{
	_tstring UserNameStr;
	SetString(UserNameStr, UserName);

	return g_AquaLangManager.Open(UserNameStr.c_str(), rConfig, rListener, fRecovery);
}

extern "C"
bool __stdcall OpenAquaLangW(const WCHAR *UserName, const AquaLangConfiguration &rConfig, INotificationAreaRequestsListener &rListener, bool fRecovery)
{
	_tstring UserNameStr;
	SetString(UserNameStr, UserName);

	return g_AquaLangManager.Open(UserNameStr.c_str(), rConfig, rListener, fRecovery);
}

extern "C"
void __stdcall CloseAquaLang()
{
	g_AquaLangManager.Close();
}

extern "C"
bool __stdcall RunBrowserA(const char *BrowserString)
{
	_tstring tBrowserString;
	SetString(tBrowserString, BrowserString);

	return g_AquaLangManager.RunBrowser(tBrowserString.c_str());
}

extern "C"
bool __stdcall RunBrowserW(const WCHAR *BrowserString)
{
	_tstring tBrowserString;
	SetString(tBrowserString, BrowserString);

	return g_AquaLangManager.RunBrowser(tBrowserString.c_str());
}

extern "C"
bool __stdcall TriggerCalculator()
{
	return g_AquaLangManager.TriggerCalculator();
}

extern "C"
bool __stdcall GetAquaLangStatistics(AquaLangStatistics &rStatistics)
{
	return g_AquaLangManager.GetStatistics(rStatistics);
}

extern "C"
void __stdcall PrintKey(int VirtualKey, bool fPress)
{
	g_AquaLangManager.PrintKey(VirtualKey, fPress);
}

extern "C"
bool __stdcall CreateKeySequenceValidatorA(LPCSTR Sequence, HSEQ &hSequence)
{
	_tstring tSequenceString;
	SetString(tSequenceString, Sequence);

	return g_AquaLangManager.CreateKeySequenceValidator(tSequenceString.c_str(), hSequence);
}

extern "C"
bool __stdcall CreateKeySequenceValidatorW(LPCWSTR Sequence, HSEQ &hSequence)
{
	_tstring tSequenceString;
	SetString(tSequenceString, Sequence);

	return g_AquaLangManager.CreateKeySequenceValidator(tSequenceString.c_str(), hSequence);
}

extern "C"
void __stdcall DeleteKeySequenceValidator(HSEQ hSequence)
{
	g_AquaLangManager.DeleteKeySequenceValidator(hSequence);
}

extern "C"
bool __stdcall AreSequencesEqual(HSEQ hSequence1, HSEQ hSequence2)
{
	return g_AquaLangManager.AreSequencesEqual(hSequence1, hSequence2);
}

extern "C"
bool __stdcall IsContainedBySequence(HSEQ hReferenceSequence, HSEQ hTestedSequence)
{
	return g_AquaLangManager.IsContainedBySequence(hReferenceSequence, hTestedSequence);
}

PsApiLoader &GetPsApiLoader()	{ return g_AquaLangManager.GetPsApiLoader(); }
