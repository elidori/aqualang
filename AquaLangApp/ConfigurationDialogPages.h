#ifndef _CONFIGURATION_DIALOG_PAGES_HEADER_
#define _CONFIGURATION_DIALOG_PAGES_HEADER_

#include "AquaLangConfiguration.h"
#include "DialogObject.h"

class ConfigPage : public DialogObject
{
protected:
	ConfigPage();
	virtual ~ConfigPage();

	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &fSkipDefault);

	virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
public:
	virtual void Load() = 0;
	virtual void Store() = 0;

	void Release();
};

class WaterDropConfigPage : public ConfigPage
{
public:
	static ConfigPage *Create(HWND hParent, AquaLangBalloonTipSettings &rSettings);
private:
	WaterDropConfigPage(AquaLangBalloonTipSettings &rSettings);
	virtual ~WaterDropConfigPage();

	virtual void Load();
	virtual void Store();
private:
	virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	AquaLangBalloonTipSettings &m_rSettings;
};

class HotKeyConfigPage : public ConfigPage
{
public:
	static ConfigPage *Create(HWND hParent, AquaLangConfiguration &rSettings);
private:
	HotKeyConfigPage(AquaLangConfiguration &rSettings);
	virtual ~HotKeyConfigPage();

	virtual void Load();
	virtual void Store();
private:
	virtual void DialogInit(HWND hwndDlg);
	virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool AddComboItem(HWND hCombo, LPCTSTR Text, int Code);
private:
	AquaLangTextCoversionSettings &m_rTextConversionSettings;
	AquaLangWebBrowseSettings &m_rWebBrowseSettings;
	AquaLangSwapSettings &m_rClipboardSwapSettings;
};

class CalculatorConfigPage : public ConfigPage
{
public:
	static ConfigPage *Create(HWND hParent, AquaLangCalculatorSettings &rSettings);
private:
	CalculatorConfigPage(AquaLangCalculatorSettings &rSettings);
	virtual ~CalculatorConfigPage();

	virtual void Load();
	virtual void Store();
private:
	virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	AquaLangCalculatorSettings &m_rSettings;
};

class GuessLanguageConfigPage : public ConfigPage
{
public:
	static ConfigPage *Create(HWND hParent, AquaLangGuessLanguageSettings &rSettings);
private:
	GuessLanguageConfigPage(AquaLangGuessLanguageSettings &rSettings);
	virtual ~GuessLanguageConfigPage();

	virtual void Load();
	virtual void Store();
private:
	virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	AquaLangGuessLanguageSettings &m_rSettings;
};

class DebugConfigPage : public ConfigPage
{
public:
	static ConfigPage *Create(HWND hParent, AquaLangConfiguration &rSettings);
private:
	DebugConfigPage(AquaLangConfiguration &rSettings);
	virtual ~DebugConfigPage();

	virtual void Load();
	virtual void Store();
private:
	virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	AquaLangUserInfoNotification &m_rUserInfoSettings;
	AquaLangUpdate &m_rVersionUpdateSettings;
};

class VersionConfigPage : public ConfigPage
{
public:
	static ConfigPage *Create(HWND hParent, AquaLangUpdate &rVersionUpdateSettings, HWND hListener);
private:
	VersionConfigPage(AquaLangUpdate &rVersionUpdateSettings, HWND hListener);
	virtual ~VersionConfigPage();

	virtual void Load();
	virtual void Store();
private:
	virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	AquaLangUpdate &m_rVersionUpdateSettings;
	HWND m_hListener;
};

class SystemPage : public ConfigPage
{
public:
	static ConfigPage *Create(HWND hParent, AquaLangNotificationArea &rNotificationArea, bool &rfAutoStart);
private:
	SystemPage(AquaLangNotificationArea &rNotificationArea, bool &rfAutoStart);
	virtual ~SystemPage();

	virtual void Load();
	virtual void Store();
private:
	virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	AquaLangNotificationArea &m_rNotificationArea;
	bool &m_rfAutoStart;
};

#endif // _CONFIGURATION_DIALOG_PAGES_HEADER_