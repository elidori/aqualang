#include "Pch.h"
#include "AquaLangLibLoader.h"
#include "ApplicationDataPath.h"

AquaLangLibLoader::AquaLangLibLoader()
	: m_hLib(NULL),
	FuncOpenAquaLangA(NULL),
	FuncOpenAquaLangW(NULL),
	FuncCloseAquaLang(NULL),
	FuncRunBrowserA(NULL),
	FuncRunBrowserW(NULL),
	FuncTriggerCalculator(NULL),
	FuncGetAquaLangStatistics(NULL),
	FuncPrintKey(NULL),
	FuncCreateKeySequenceValidatorA(NULL),
	FuncCreateKeySequenceValidatorW(NULL),
	FuncDeleteKeySequenceValidator(NULL),
	FuncAreSequencesEqual(NULL),
	FuncIsContainedBySequence(NULL)
{
}

AquaLangLibLoader::~AquaLangLibLoader()
{
	Close();
}

bool AquaLangLibLoader::Open()
{
	if(m_hLib != NULL)
		return true;

	_tstring ApplicationPath;
	ApplicationDataPath PathFinder(NULL);
	PathFinder.ComposeRelativePath(_T(""), ApplicationPath);
	ApplicationPath += _T("AquaLangLib.dll");

	m_hLib = ::LoadLibrary(ApplicationPath.c_str());
	if(m_hLib == NULL)
		return false;

	FuncOpenAquaLangA = (T_OpenAquaLangA)::GetProcAddress(m_hLib, "OpenAquaLangA");
	if(FuncOpenAquaLangA == NULL)
	{
		Close();
		return false;
	}
	FuncOpenAquaLangW = (T_OpenAquaLangW)::GetProcAddress(m_hLib, "OpenAquaLangW");
	if(FuncOpenAquaLangW == NULL)
	{
		Close();
		return false;
	}
	FuncCloseAquaLang = (T_CloseAquaLang)::GetProcAddress(m_hLib, "CloseAquaLang");
	if(FuncCloseAquaLang == NULL)
	{
		Close();
		return false;
	}
	FuncRunBrowserA = (T_RunBrowserA)::GetProcAddress(m_hLib, "RunBrowserA");
	if(FuncRunBrowserA == NULL)
	{
		Close();
		return false;
	}
	FuncRunBrowserW = (T_RunBrowserW)::GetProcAddress(m_hLib, "RunBrowserW");
	if(FuncRunBrowserW == NULL)
	{
		Close();
		return false;
	}
	FuncTriggerCalculator = (T_TriggerCalculator)::GetProcAddress(m_hLib, "TriggerCalculator");
	if(FuncTriggerCalculator == NULL)
	{
		Close();
		return false;
	}
	FuncGetAquaLangStatistics = (T_GetAquaLangStatistics)::GetProcAddress(m_hLib, "GetAquaLangStatistics");
	if(FuncGetAquaLangStatistics == NULL)
	{
		Close();
		return false;
	}
	FuncPrintKey = (T_PrintKey)::GetProcAddress(m_hLib, "PrintKey");
	if(FuncPrintKey == NULL)
	{
		Close();
		return false;
	}

	FuncCreateKeySequenceValidatorA = (T_CreateKeySequenceValidatorA)::GetProcAddress(m_hLib, "CreateKeySequenceValidatorA");
	if(FuncCreateKeySequenceValidatorA == NULL)
	{
		Close();
		return false;
	}
	FuncCreateKeySequenceValidatorW = (T_CreateKeySequenceValidatorW)::GetProcAddress(m_hLib, "CreateKeySequenceValidatorW");
	if(FuncCreateKeySequenceValidatorW == NULL)
	{
		Close();
		return false;
	}
	FuncDeleteKeySequenceValidator = (T_DeleteKeySequenceValidator)::GetProcAddress(m_hLib, "DeleteKeySequenceValidator");
	if(FuncDeleteKeySequenceValidator == NULL)
	{
		Close();
		return false;
	}
	FuncAreSequencesEqual = (T_AreSequencesEqual)::GetProcAddress(m_hLib, "AreSequencesEqual");
	if(FuncAreSequencesEqual == NULL)
	{
		Close();
		return false;
	}
	FuncIsContainedBySequence = (T_IsContainedBySequence)::GetProcAddress(m_hLib, "IsContainedBySequence");
	if(FuncIsContainedBySequence == NULL)
	{
		Close();
		return false;
	}
	return true;
}

void AquaLangLibLoader::Close()
{
	FuncOpenAquaLangA = NULL;
	FuncOpenAquaLangW = NULL;
	FuncCloseAquaLang = NULL;
	FuncRunBrowserA = NULL;
	FuncRunBrowserW = NULL;
	FuncTriggerCalculator = NULL;
	FuncGetAquaLangStatistics = NULL;
	FuncPrintKey = NULL;
	FuncCreateKeySequenceValidatorA = NULL;
	FuncCreateKeySequenceValidatorW = NULL;
	FuncDeleteKeySequenceValidator = NULL;
	FuncAreSequencesEqual = NULL;
	FuncIsContainedBySequence = NULL;
	if(m_hLib != NULL)
	{
		::FreeLibrary(m_hLib);
		m_hLib = NULL;
	}
}

bool AquaLangLibLoader::OpenAquaLang(const char *UserName, const AquaLangConfiguration &rConfig, INotificationAreaRequestsListener &rListener, bool fRecovery)
{
	if(!Open())
		return false;
	return (this->FuncOpenAquaLangA)(UserName, rConfig, rListener, fRecovery);
}

bool AquaLangLibLoader::OpenAquaLang(const WCHAR *UserName, const AquaLangConfiguration &rConfig, INotificationAreaRequestsListener &rListener, bool fRecovery)
{
	if(!Open())
		return false;
	return (this->FuncOpenAquaLangW)(UserName, rConfig, rListener, fRecovery);
}

void AquaLangLibLoader::CloseAquaLang()
{
	if(m_hLib != NULL)
	{
		(this->FuncCloseAquaLang)();
	}
}

bool AquaLangLibLoader::RunBrowser(const char *BrowserString)
{
	return (this->FuncRunBrowserA)(BrowserString);
}

bool AquaLangLibLoader::RunBrowser(const WCHAR *BrowserString)
{
	return (this->FuncRunBrowserW)(BrowserString);
}

bool AquaLangLibLoader::TriggerCalculator()
{
	return (this->FuncTriggerCalculator)();
}

bool AquaLangLibLoader::GetAquaLangStatistics(AquaLangStatistics &rStatistics)
{
	return (this->FuncGetAquaLangStatistics)(rStatistics);
}

void AquaLangLibLoader::PrintKey(int VirtualKey, bool fPress)
{
	(this->FuncPrintKey)(VirtualKey, fPress);
}

bool AquaLangLibLoader::CreateKeySequenceValidator(LPCSTR Sequence, HSEQ &hSequence)
{
	return (this->FuncCreateKeySequenceValidatorA)(Sequence, hSequence);
}

bool AquaLangLibLoader::CreateKeySequenceValidator(LPCWSTR Sequence, HSEQ &hSequence)
{
	return (this->FuncCreateKeySequenceValidatorW)(Sequence, hSequence);
}

void AquaLangLibLoader::DeleteKeySequenceValidator(HSEQ hSequence)
{
	(this->FuncDeleteKeySequenceValidator)(hSequence);
}

bool AquaLangLibLoader::AreSequencesEqual(HSEQ hSequence1, HSEQ hSequence2)
{
	return (this->FuncAreSequencesEqual)(hSequence1, hSequence2);
}

bool AquaLangLibLoader::IsContainedBySequence(HSEQ hReferenceSequence, HSEQ hTestedSequence)
{
	return (this->FuncIsContainedBySequence)(hReferenceSequence, hTestedSequence);
}
