#ifndef _TEXT_LANGUAGE_CONVERTER_HEADER_
#define _TEXT_LANGUAGE_CONVERTER_HEADER_

#include "IKeyboardEvent.h"
#include "UtilityWindow.h"
#include "ITextConverter.h"

class KeyboardSequenceEvent;
class UserInformationNotifier;
class IKeyboardEventCallback;
class LanguageDetector;
class ILanguageModifier;
class ITextChangeClient;
class GuessLanguageHandler;

class TextLanguageConverter :
	public IKeyboardEvent,
	public IWindowMessageListener,
	public ITextConverter
{
public:
	TextLanguageConverter(
				UtilityWindow &rUtilityWindow,
				KeyboardSequenceEvent &rKeyboardSequenceEvent,
				UserInformationNotifier &rUserInformationNotifier,
				const LanguageDetector &rLanguageDetector,
				ILanguageModifier &rLanguageModifier,
				ITextChangeClient &rTextChangeClient,
				GuessLanguageHandler &rGuessLanguageHandler
				);
	virtual ~TextLanguageConverter();

	bool Open(LPCTSTR KeyboardTrigger);
	void Close();
private:
	// IKeyboardEvent pure virtuals {
	virtual bool CanOperate(KeyboardEventInfo &Info, bool &fExclusive);
	virtual void Operate(const KeyboardEventInfo &Info, IKeyboardEventCallback &rCallback);
	// }

	// ITextConverter pure virtuals {
	virtual bool ChangeTypedText(HKL hCurrentKL, HKL hTargetKL, HWND hCurrentWindow, IKeyboardEventCallback *pCallback);
	// }

	// IWindowMessageListener pure virtuals {
	virtual void Message(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// }
private:
	UtilityWindow &m_rUtilityWindow;
	KeyboardSequenceEvent &m_rKeyboardSequenceEvent;
	UserInformationNotifier &m_rUserInformationNotifier;
	const LanguageDetector &m_rLanguageDetector;
	ILanguageModifier &m_rLanguageModifier;
	ITextChangeClient &m_rTextChangeClient;
	GuessLanguageHandler &m_rGuessLanguageHandler;

	// intermediate params for the timer message
	HWND m_hCurrentWindow;
	HKL m_hTargetKL;
	int m_BackspacesToIgnore;
	int m_nRetries;
	IKeyboardEventCallback *m_pCallback;
};

#endif // _TEXT_LANGUAGE_CONVERTER_HEADER_