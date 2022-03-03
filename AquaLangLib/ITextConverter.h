#ifndef _I_TEXT_CONVERTER_HEADER_
#define _I_TEXT_CONVERTER_HEADER_

class ITextConverter
{
protected:
	virtual ~ITextConverter() {}
public:
	virtual bool ChangeTypedText(HKL hCurrentKL, HKL hTargetKL, HWND hCurrentWindow, IKeyboardEventCallback *pCallback) = 0;
};

#endif // _I_TEXT_CONVERTER_HEADER_