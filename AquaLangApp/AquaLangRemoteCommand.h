#ifndef _AQUALANG_REMOTE_COMMAND_HEADER_
#define _AQUALANG_REMOTE_COMMAND_HEADER_

#include "ThreadInstance.h"
#include "ProcessUtils.h"

class UtilityWindow;

enum AppCommand
{
	AppCommand_None = 0,
	AppCommand_UpdateConfig = 1,
	AppCommand_Disable = 2,
	AppCommand_Enable = 3,
	AppCommand_Stop = 4,
	AppCommand_LogOff = 5,
	AppCommand_StopFromWatchdog = 6,
};

struct AppCommandSharedMemStruct
{
	int Command;
};


class AquaLangRemoteCommand : public ThreadInstance
{
public:
	AquaLangRemoteCommand(UtilityWindow &rUtilityWindow);
	virtual ~AquaLangRemoteCommand();

	bool Create(LPCTSTR UserName);
	bool Open(LPCTSTR UserName);

	bool SendCommand(AppCommand Command);

	void Close();
private:
	// ThreadInstance pure virtuals {
	virtual DWORD RunInThread();
	virtual void BreakThread();
	// }
private:
	UtilityWindow &m_rUtilityWindow; // for sending messages

	ProcessEvent m_CommandEvent;

	ProcessSharedMem m_CommandSharedMem;
	AppCommandSharedMemStruct *m_pCommandBuffer;
};

#endif // _AQUALANG_REMOTE_COMMAND_HEADER_