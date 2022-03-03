#include "Pch.h"
#include "ArgumentsParser.h"
#include "Log.h"

ArgumentsParser::ArgumentsParser()
	: m_Action(ACTION_TYPE_UNKNOWN)
{
}

ArgumentsParser::~ArgumentsParser()
{
}

bool ArgumentsParser::Parse(LPCTSTR CmdLine)
{
	Log(_T("ArgumentsParser::Parse - CmdLine=%s\n"), CmdLine);

	if(CmdLine == NULL)
		return true;

	ArgList_t ArgList;

	LPTSTR ParsedCmdLine = _tcsdup(CmdLine);
	if(!ParsedCmdLine)
		return false;

	LPTSTR Context = NULL;
	LPTSTR Current = NULL;

	Current = _tcstok_s(ParsedCmdLine, _T(" "), &Context);

	bool fSuccess = true;
	while(Current != NULL)
	{
		if(Current[0] == _T('\"'))
		{
			// strip out the '"' and read until next '"'
			Current++;
			size_t nTokenLength = _tcslen(Current);
			if(nTokenLength > 0 && Current[nTokenLength - 1] == _T('\"'))
			{
				// the content inside the "" has no spaces
				Current[nTokenLength - 1] = _T('\0');
				ArgList.push_back(Current);
			}
			else
			{
				// the content inside the "" has spaces
				// insert the part of the token until the first space
				ArgList.push_back(Current);
				// restore the space
				_tstring &Str = ArgList.at(ArgList.size() - 1);
				Str += _T(" ");
				// get the second part of the token, until the "
				Current = _tcstok_s(NULL, _T("\""), &Context);
				if(Current)
				{
					Str += Current;
				}
				else
				{
					fSuccess = false;
					break;
				}
			}
		}
		else
		{
			ArgList.push_back(Current);
		}
		Current = _tcstok_s(NULL, _T(" "), &Context);
	}
	delete[] ParsedCmdLine;

	if(!fSuccess)
		return false;

	if(!ParseArguments(ArgList))
		return false;

	return true;
}

bool ArgumentsParser::ParseArguments(const ArgList_t &rArgList)
{
	if(rArgList.size() == 0)
	{
		m_Action = ACTION_TYPE_RUN;
		m_Parameter = _T("");
		return true;
	}

	if(rArgList.at(0) == _T("-firstrun"))
	{
		m_Action = ACTION_TYPE_FIRSTRUN;
	}
	else if(rArgList.at(0) == _T("-startup"))
	{
		m_Action = ACTION_TYPE_STARTUP;
	}
	else if(rArgList.at(0) == _T("-stop"))
	{
		m_Action = ACTION_TYPE_STOP;
	}
	else if(rArgList.at(0) == _T("-config"))
	{
		m_Action = ACTION_TYPE_CONFIG;
	}
	else if(rArgList.at(0) == _T("-install"))
	{
		m_Action = ACTION_TYPE_INSTALL;
	}
	else if(rArgList.at(0) == _T("-upgrade"))
	{
		m_Action = ACTION_TYPE_UPGRADE;
	}
	else if(rArgList.at(0) == _T("-uninstall"))
	{
		m_Action = ACTION_TYPE_UNINSTALL;
	}
	else if(rArgList.at(0) == _T("-runbrowser"))
	{
		m_Action = ACTION_TYPE_RUN_BROWSER;
	}
	else if(rArgList.at(0) == _T("-calculator"))
	{
		m_Action = ACTION_TYPE_CALCULATOR;
	}
	else if(rArgList.at(0) == _T("-watchdog"))
	{
		m_Action = ACTION_TYPE_WATCHDOG;
	}
	else if(rArgList.at(0) == _T("-runfromwatchdog"))
	{
		m_Action = ACTION_TYPE_RUN_FROM_WATCHDOG;
	}
	else
	{
		m_Action = ACTION_TYPE_UNKNOWN;
		return false;
	}
	
	if(rArgList.size() >= 2)
	{
		m_Parameter = rArgList.at(1);
	}
	return true;
}
