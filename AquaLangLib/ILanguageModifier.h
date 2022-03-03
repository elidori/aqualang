#ifndef _I_LANGUAGE_MODIFIER_HEADER_
#define _I_LANGUAGE_MODIFIER_HEADER_

class ILanguageModifier
{
protected:
	virtual ~ILanguageModifier() {}
public:
	virtual void ChangeLanguage(HWND hwnd, HKL hTargetKL) = 0;
	virtual void ClearTextHistory() = 0;
};

#endif // _I_LANGUAGE_MODIFIER_HEADER_