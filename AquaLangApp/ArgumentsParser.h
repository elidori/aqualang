#ifndef _ARGUMENTS_PARSER_HEADER_
#define _ARGUMENTS_PARSER_HEADER_

enum ACTION_TYPE
{
	ACTION_TYPE_UNKNOWN,
	ACTION_TYPE_RUN,
	ACTION_TYPE_FIRSTRUN,
	ACTION_TYPE_STARTUP,
	ACTION_TYPE_CONFIG,
	ACTION_TYPE_STOP,
	ACTION_TYPE_INSTALL,
	ACTION_TYPE_UPGRADE,
	ACTION_TYPE_UNINSTALL,
	ACTION_TYPE_RUN_BROWSER,
	ACTION_TYPE_CALCULATOR,
	ACTION_TYPE_WATCHDOG,
	ACTION_TYPE_RUN_FROM_WATCHDOG,
};

class ArgumentsParser
{
	typedef std::vector<_tstring> ArgList_t;
public:
	ArgumentsParser();
	virtual ~ArgumentsParser();

	bool Parse(LPCTSTR CmdLine);

	int GetAction() const			{ return m_Action; }
	LPCTSTR GetParameter() const	{ return m_Parameter.c_str(); }
private:
	bool ParseArguments(const ArgList_t &rArgList);
private:
	int m_Action;
	_tstring m_Parameter;
};

#endif // _ARGUMENTS_PARSER_HEADER_