#include "Pch.h"
#include "ApplicationDataPath.h"

#define AQUALANG_APPLICATION_DATA_FOLDER _T("AquaLang")

ApplicationDataPath::ApplicationDataPath(LPCTSTR UserName)
{
	if(UserName && _tcslen(UserName) > 0)
	{
		m_UserName = UserName;
	}
	else
	{
		TCHAR DetectedUserName[256];
		_tcscpy_s(DetectedUserName, sizeof(DetectedUserName) / sizeof(DetectedUserName[0]), _T(""));
		DWORD Size = sizeof(DetectedUserName) / sizeof(DetectedUserName[0]);
		if(::GetUserName(DetectedUserName, &Size))
		{
			m_UserName = DetectedUserName;
		}
	}
}

ApplicationDataPath::~ApplicationDataPath()
{
}

void ApplicationDataPath::ComposePath(LPCTSTR FileName, _tstring &rPath)
{
	BuildPath();
	if(m_Path.length() == 0)
	{
		rPath = m_ApplicationPath;
	}
	else
	{
		rPath = m_Path;
	}

	if(FileName != NULL && _tcslen(FileName) > 0)
	{
		AppendUserName(rPath);
		rPath += FileName;
	}
}

void ApplicationDataPath::ComposeRelativePath(LPCTSTR FileName, _tstring &rPath)
{
	BuildPath();
	rPath = m_ApplicationPath + FileName;
}

void ApplicationDataPath::GetApplicationName(_tstring &rName)
{
	BuildPath();
	rName = m_ApplicationFullPathName;
}

void ApplicationDataPath::GetApplicationPrivateName(_tstring &rName)
{
	BuildPath();
	rName = m_ApplicationPrivateName;
}

void ApplicationDataPath::DeleteDirectory(LPCTSTR Path, int DeleteDepth, LPCTSTR FilesFilter /*= _T("\\*")*/)
{
	// first delete all files in the directory
	_tstring SearchPath = Path;
	SearchPath += FilesFilter;

	WIN32_FIND_DATA FileData;
	HANDLE hFileSearch = ::FindFirstFile(SearchPath.c_str(), &FileData);
	if(hFileSearch != INVALID_HANDLE_VALUE)
	{
		do
		{
			_tstring DeletedFile = Path;
			DeletedFile += _T("\\");
			DeletedFile += FileData.cFileName;

			if((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				::DeleteFile(DeletedFile.c_str());
			}
			else if(DeleteDepth > 0)
			{
				if((_tcscmp(FileData.cFileName, _T(".")) != 0) && (_tcscmp(FileData.cFileName, _T("..")) != 0))
				{
					DeleteDirectory(DeletedFile.c_str(), DeleteDepth - 1, _T("\\*"));
				}
			}
		}
		while(::FindNextFile(hFileSearch, &FileData));

		::FindClose(hFileSearch);
	}

	// then remove the directory (only if not empty
	if(_tcscmp(FilesFilter, _T("\\*")) == 0)
	{
		::RemoveDirectory(Path);
	}
}

void ApplicationDataPath::BuildPath()
{
	if(m_ApplicationPath.length() == 0)
	{
		TCHAR FullPath[1024];
		TCHAR URL[1024];
		LPTSTR lpsNameOffset = NULL;

		::GetModuleFileName (
			NULL,
			FullPath,
			sizeof(FullPath)/sizeof(FullPath[0])
			);
		m_ApplicationFullPathName = FullPath;

		::GetFullPathName (
			FullPath,
			sizeof(URL)/sizeof(URL[0]),
			URL,
			&lpsNameOffset
			);

		m_ApplicationPrivateName = lpsNameOffset;
		TryApplicationDataPath(lpsNameOffset, m_Path);

		_tcscpy_s(lpsNameOffset, sizeof(URL)/sizeof(URL[0]) - (lpsNameOffset - URL), _T("\\"));
		m_ApplicationPath = URL;
	}
}

bool ApplicationDataPath::TryApplicationDataPath(LPCTSTR UNUSED(ExecutableName), _tstring &rPath)
{
	// find application data folder
	TCHAR szPath[MAX_PATH];
	HRESULT hr = SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath);
	if(hr != S_OK)
		return false;

	// build path
	rPath = szPath;
	rPath += _T("\\");
	rPath += AQUALANG_APPLICATION_DATA_FOLDER;
	/*
	old code - calculated the subfolder according to the application name. This is good only as long as the application file name 
	is not changed ...
	rPath += ExecutableName;
	// remove exe extension
	LPCTSTR Extension = _tcsrchr(ExecutableName, _T('.'));
	if(Extension)
	{
		int ReducedSize = ExecutableName + _tcslen(ExecutableName) - Extension;
		rPath = rPath.substr(0, rPath.length() - ReducedSize);
	}
	*/
	rPath += _T("\\");

	// make sure the path exists
	if(!::CreateDirectory(rPath.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
	{
		rPath = _T("");
		return false;
	}
	return true;
}

void ApplicationDataPath::AppendUserName(_tstring &rPath) const
{
	if(m_UserName.length() > 0)
	{
		rPath += m_UserName;
		rPath += _T("-");
	}
}
