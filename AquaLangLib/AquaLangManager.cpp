#include "Pch.h"
#include "AquaLangManager.h"
#include "StringConvert.h"
#include "resource.h"
#include "RemoteWebBrowser.h"
#include "ComHelper.h"
#include "Log.h"
#include "WinHookListenerMap.h"
#include "HookDispatcher.h"
#include "KeyboardHookDispatcher.h"
#include "MouseHookDispatcher.h"

#define POLLING_INTERVAL_MSEC 10
#define CLOSE_TIMEOUT 10000

#define KEY_SEQUENCE_MAX_COUNT 128
#define RECENT_TEXT_MAX_LENGTH 128

#pragma warning (disable:4355)

AquaLangManager::AquaLangManager()
	: m_FocusChangeEvent(),
	m_UserInformationNotifier(),
	m_AsyncHookDispatcher(),
	m_LanguageHandler(m_LanguageStringTable, m_FocusChangeEvent.GetUtilityWindow(), m_FocusChangeEvent, m_UserInformationNotifier, *this),
	m_KeyboardSequenceEvent(m_AsyncHookDispatcher.GetUtilityWindow(), m_FocusInfo, m_LanguageHandler.GetDataBase(), m_LanguageDetector, m_AsyncHookDispatcher),
	m_TextConvertAdvisor(m_TipCoordinator, m_KeyboardSequenceEvent, m_AsyncHookDispatcher, *this),
	m_GuessLanguageHandler(m_TipCoordinator, m_AsyncHookDispatcher.GetUtilityWindow(), m_FocusInfo, m_KeyboardSequenceEvent, m_UserInformationNotifier, m_AsyncHookDispatcher),
	m_TextLanguageConverter(m_AsyncHookDispatcher.GetUtilityWindow(), m_KeyboardSequenceEvent, m_UserInformationNotifier, m_LanguageDetector, *this, m_TextConvertAdvisor, m_GuessLanguageHandler),
	m_WebSearchHandler(m_KeyboardSequenceEvent, m_LanguageStringTable, m_UserInformationNotifier),
	m_CalculatorHandler(m_TipCoordinator, m_AsyncHookDispatcher.GetUtilityWindow(), m_FocusInfo, m_KeyboardSequenceEvent, m_UserInformationNotifier),
	m_CalculatorPopupHandler(m_KeyboardSequenceEvent, m_CalculatorHandler),
	m_ClipboardSwapper(m_AsyncHookDispatcher.GetUtilityWindow(), m_KeyboardSequenceEvent),
	m_fRecovered(false)
{
}

AquaLangManager::~AquaLangManager()
{
	Close();
}

bool AquaLangManager::Open(LPCTSTR UserName, const AquaLangConfiguration &rConfig, INotificationAreaRequestsListener &rListener, bool fRecovery)
{
	CloseThread(CLOSE_TIMEOUT);

	if(!m_PsApiLoader.Open())
	{
		Log(_T("AquaLangManager::Open - Failed opening PsApi.dll\n"));
		MessageBox(
				NULL,
				_T("psapi.dll could not be loaded.\nPlease make sure this system library is well installed, in order to make AquaLang work correctly"),
				_T("AquaLang problem while opening"),
				MB_OK | MB_ICONEXCLAMATION
				);
	}

	m_Config = rConfig;

	if(UserName && _tcslen(UserName) > 0)
	{
		m_UserName = UserName;
	}
	if(m_LanguageStringTable.Open() != Result_Success)
		return false;

	m_GuessLanguageHandler.SetNotificationAreaHandler(rListener);

	m_fRecovered = fRecovery;

	if(!OpenThread())
	{
		Log(_T("AquaLangManager::Open - Failed opening thread\n"));
		return false;
	}
	return true;
}

void AquaLangManager::Close()
{
	CloseThread(CLOSE_TIMEOUT);
	m_UserName = _T("");

	assert(m_ValidatorMap.size() == 0);
	ValidatorMap::iterator Iterator = m_ValidatorMap.begin();
	for(; Iterator != m_ValidatorMap.end(); Iterator++)
	{
		delete (*Iterator).second;
	}
	m_ValidatorMap.erase(m_ValidatorMap.begin(), m_ValidatorMap.end());

	// just in case
	CloseHooks();
}

bool AquaLangManager::RunBrowser(LPCTSTR BrowserString)
{
	RemoteWebBrowser Browser(BrowserString);
	if(!Browser.Run())
		return false;

	return true;
}

bool AquaLangManager::TriggerCalculator()
{
	Log(_T("AquaLangManager::TriggerCalculator\n"));
	if(!m_CalculatorHandler.IsOpen())
		return false;
	m_CalculatorHandler.NotifyUserTriggersCalculator();
	return true;
}

bool AquaLangManager::GetStatistics(AquaLangStatistics &rStatistics)
{
	if(!m_UserInformationNotifier.FillStatistics(rStatistics))
		return false;
	if(!m_KeyboardSequenceEvent.FillStatistics(rStatistics))
		return false;
	return true;
}

void AquaLangManager::PrintKey(int VirtualKey, bool fPress)
{
	KeyboardSequence::PrintKey(VirtualKey, fPress, false, NULL, NULL);
}

bool AquaLangManager::CreateKeySequenceValidator(LPCTSTR Sequence, HSEQ &hSequence)
{
	if(Sequence == NULL)
		return false;
	size_t Len = _tcslen(Sequence);
	if(Len == 0)
		return false;

	KeyboardSequence *pNew = new KeyboardSequence(true);
	if(!pNew)
		return false;
	if(!pNew->Open(KEY_SEQUENCE_MAX_COUNT, RECENT_TEXT_MAX_LENGTH, Sequence, false))
	{
		delete pNew;
		return false;
	}

	hSequence = (HSEQ)pNew;
	m_ValidatorMap.insert(ValidatorMap::value_type(hSequence, pNew));
	return true;
}

void AquaLangManager::DeleteKeySequenceValidator(HSEQ hSequence)
{
	ValidatorMap::iterator Iterator = m_ValidatorMap.find(hSequence);
	if(Iterator == m_ValidatorMap.end())
	{
		Log(_T("AquaLangManager::DeleteKeySequenceValidator - handler was not found"));
		return;
	}
	delete (*Iterator).second;
	m_ValidatorMap.erase(Iterator);
}

bool AquaLangManager::AreSequencesEqual(HSEQ hSequence1, HSEQ hSequence2)
{
	ValidatorMap::iterator Iterator = m_ValidatorMap.find(hSequence1);
	if(Iterator == m_ValidatorMap.end())
		return false;
	KeyboardSequence *pSequence1 = (*Iterator).second;

	Iterator = m_ValidatorMap.find(hSequence2);
	if(Iterator == m_ValidatorMap.end())
		return false;
	KeyboardSequence *pSequence2 = (*Iterator).second;

	return pSequence1->IsEqual(*pSequence2);
}

bool AquaLangManager::IsContainedBySequence(HSEQ hReferenceSequence, HSEQ hTestedSequence)
{
	ValidatorMap::iterator Iterator = m_ValidatorMap.find(hReferenceSequence);
	if(Iterator == m_ValidatorMap.end())
		return false;
	KeyboardSequence *pReferenceSequence = (*Iterator).second;

	Iterator = m_ValidatorMap.find(hTestedSequence);
	if(Iterator == m_ValidatorMap.end())
		return false;
	KeyboardSequence *pTestedSequence = (*Iterator).second;

	return pReferenceSequence->IsContaining(*pTestedSequence);
}

DWORD AquaLangManager::RunInThread()
{
	Log(_T("AquaLangManager::RunInThread\n"));

	ComInitializer _ComInitializer;

	m_LanguageDetector.Open(&m_GuessLanguageHandler);

	if(m_Config.CalcSettings.fActive)
	{
		_tstring HotKey;
		SetString(HotKey, m_Config.CalcSettings.HotKey);
		if(m_CalculatorHandler.Configure(m_UserName.c_str(), m_Config.CalcSettings.ApearanceDurationIn100msec, HotKey.c_str(), m_Config.CalcSettings.KbPauseBeforePopupIn100msec))
		{
			m_AsyncHookDispatcher.AddThreadReleatedObject(m_CalculatorHandler);
		}
		else
		{
			Log(_T("AquaLangManager::RunInThread - failed configuring calculator handler\n"));
			// continue anyway
//			return -1;
		}
	}

	if(m_GuessLanguageHandler.Configure(
						m_UserName.c_str(), 
						m_Config.GuessLanguageSettings.ApearanceDurationIn100msec, 
						m_Config.GuessLanguageSettings.KbPauseBeforePopupIn100msec,
						m_Config.GuessLanguageSettings.MinimalNumberOfCharacters,
						m_Config.GuessLanguageSettings.MaximalNumberOfCharacters,
						m_LanguageStringTable
						))
	{
		if(m_Config.GuessLanguageSettings.fActive)
		{
			m_AsyncHookDispatcher.AddThreadReleatedObject(m_GuessLanguageHandler);
		}
	}
	else
	{
		Log(_T("AquaLangManager::RunInThread - failed initializing guess language\n"));
	}

	if(!m_AsyncHookDispatcher.Open())
	{
		Log(_T("AquaLangManager::RunInThread() - failed to open async hook object\n"));
		return (DWORD)-1;
	}

	if(m_Config.CalcSettings.fActive)
	{
		if(!m_CalculatorHandler.RegisterOnHooks())
		{
			Log(_T("AquaLangManager::RunInThread - failed registering calculator handler\n"));
			// continue anyway
//			return -1;
		}

		_tstring HotKey;
		SetString(HotKey, m_Config.CalcSettings.PopupHotKey);
		if(!m_CalculatorPopupHandler.Open(HotKey.c_str()))
		{
			Log(_T("AquaLangManager::RunInThread - failed opening calculator popup handler\n"));
			// continue anyway
//			return -1;
		}
	}

	Result res = m_UserInformationNotifier.Open(m_UserName.c_str(), m_Config.UserInfoSettings);
	if(res != Result_Success)
	{
		if (res == Result_InvalidParameter && strlen(m_Config.UserInfoSettings.ServerUrl) == 0)
		{

		}
		else
		{
			Log(_T("AquaLangManager::RunInThread() - failed to open user info notifier (res=%d)\n"), res);
			return (DWORD)-1;
		}
	}
	if(m_fRecovered)
	{
		m_UserInformationNotifier.NotifyWatchdogRecovery();
		m_fRecovered = false;
	}

#ifndef SLIM_AQUALANG
	Timer DbTimer;
	if(m_LanguageHandler.Open(m_UserName.c_str(), m_Config) != Result_Success)
	{
		Log(_T("AquaLangManager::RunInThread - failed opening Language Handler\n"));
		return (DWORD)-1;
	}
	Log(_T("AquaLangManager::RunInThread - data base loaded in %I64d msec\n"), DbTimer.GetDiff());
#endif
	if(m_Config.TextConversionSettings.fActive)
	{
		_tstring HotKey;
		SetString(HotKey, m_Config.TextConversionSettings.HotKey);

		if(!m_TextLanguageConverter.Open(HotKey.c_str()))
		{
			Log(_T("AquaLangManager::RunInThread - failed opening text language converter\n"));
			// continue anyway
//			return -1;
		}

		if(!m_TextConvertAdvisor.Open(m_UserName.c_str()))
		{
			Log(_T("AquaLangManager::RunInThread - failed opening text language convert advisor\n"));
			// continue anyway
//			return -1;
		}
	}

	if(m_Config.WebBrowseSettings.fActive)
	{
		_tstring HotKey;
		SetString(HotKey, m_Config.WebBrowseSettings.HotKey);

		if(!m_WebSearchHandler.Open(m_UserName.c_str(), HotKey.c_str(), m_Config.WebBrowseSettings.nWebSearchOptionCode, m_Config.WebBrowseSettings.nWebTranslateOptionCode))
		{
			Log(_T("AquaLangManager::RunInThread - failed opening web search handler\n"));
			// continue anyway
//			return -1;
		}
	}

	if(m_Config.SwapSettings.fActive)
	{
		_tstring HotKey;
		SetString(HotKey, m_Config.SwapSettings.HotKey);

		if(!m_ClipboardSwapper.Open(HotKey.c_str()))
		{
			Log(_T("AquaLangManager::RunInThread - failed opening clipboard swapper\n"));
			// continue anyway
//			return -1;
		}
	}

	if(!m_MessageWindow.OpenMessageWindow())
	{
		Log(_T("AquaLangManager::RunInThread - failed opening message window\n"));
		return (DWORD)-1;
	}

	res = OpenHooks();
	if(res != Result_Success)
	{
		Log(_T("AquaLangManager::RunInThread - failed opening hooks\n"));
		return (DWORD)-1;
	}

	MSG msg;
	while(!IsBreaking())
	{
		if(::GetMessage(&msg, NULL, NULL, NULL))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
	CloseHooks();

	m_MessageWindow.CloseMessageWindow();

#ifndef SLIM_AQUALANG
	DbTimer.Restart();
	m_LanguageHandler.Close();
	Log(_T("AquaLangManager::RunInThread - Databased saved in %I64d msec\n"), DbTimer.GetDiff());
#endif
	m_TextLanguageConverter.Close();
	m_TextConvertAdvisor.Close();
	m_WebSearchHandler.Close();
	m_CalculatorPopupHandler.Close();
	m_CalculatorHandler.Close();
	m_GuessLanguageHandler.Close();
	m_ClipboardSwapper.Close();
	m_UserInformationNotifier.Close();
	m_FocusInfo.Clear();
	m_AsyncHookDispatcher.Close();
	m_LanguageDetector.Close();
	return 0;
}

void AquaLangManager::BreakThread()
{
	m_MessageWindow.PostMessage(WM_USER, 0, 0); // triggering a message to exit ::GetMessage
}

void AquaLangManager::ChangeLanguage(HWND hwnd, HKL hTargetKL)
{
	::PostMessage(hwnd, WM_INPUTLANGCHANGEREQUEST, INPUTLANGCHANGE_SYSCHARSET, (LPARAM)hTargetKL);
	m_TextConvertAdvisor.ResetLanguageHistory();
}

void AquaLangManager::ClearTextHistory()
{
	m_KeyboardSequenceEvent.ClearTextHistory();
}

Result AquaLangManager::OpenHooks()
{
	Result res = g_MouseHookDispatcher.OpenHook();
	if(res != Result_Success)
		return res;
	res = g_KeyboardHookDispatcher.OpenHook();
	if(res != Result_Success)
	{
		CloseHooks();
		return res;
	}
	return Result_Success;
}

void AquaLangManager::CloseHooks()
{
	g_MouseHookDispatcher.CloseHook();
	g_KeyboardHookDispatcher.CloseHook();
}

