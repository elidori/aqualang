#include "Pch.h"
#include "ArgumentsParser.h"
#include "AquaLangImpl.h"

HMODULE g_hModule = NULL;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE UNUSED(hPrevInstance), LPTSTR lpCmdLine, int UNUSED(nCmdShow))
{
	g_hModule = hInstance;

	ArgumentsParser ArgParser;
	if(!ArgParser.Parse(lpCmdLine))
	{
		_tstring Message = _T("Wrong argument for the application: ");
		Message += lpCmdLine;
		MessageBox(NULL, Message.c_str(), _T("AquaLang Error"), MB_OK);
		return -1;
	}

	AquaLangImpl Implementor(ArgParser.GetParameter());

	bool fSuccess = true;

	switch(ArgParser.GetAction())
	{
	case ACTION_TYPE_RUN:
		fSuccess = Implementor.Run(false, false, false);
		break;
	case ACTION_TYPE_FIRSTRUN:
		fSuccess = Implementor.Run(true, false, false);
		break;
	case ACTION_TYPE_STARTUP:
		fSuccess = Implementor.Run(false, true, false);
		break;
	case ACTION_TYPE_STOP:
		fSuccess = Implementor.Stop(true);
		break;
	case ACTION_TYPE_CONFIG:
		Implementor.Config();
		break;
	case ACTION_TYPE_INSTALL:
		fSuccess = Implementor.Install();
		break;
	case ACTION_TYPE_UPGRADE:
		fSuccess = Implementor.Upgrade();
		break;
	case ACTION_TYPE_UNINSTALL:
		fSuccess = Implementor.Uninstall();
		break;
	case ACTION_TYPE_RUN_BROWSER:
		fSuccess = Implementor.RunBrowser();
		break;
	case ACTION_TYPE_CALCULATOR:
		fSuccess = Implementor.TriggerCalculator();
		break;
	case ACTION_TYPE_WATCHDOG:
		fSuccess = Implementor.RunWatchdog();
		break;
	case ACTION_TYPE_RUN_FROM_WATCHDOG:
		fSuccess = Implementor.Run(false, false, true);
		break;
	}
	return fSuccess ? 0 : -1;
}
