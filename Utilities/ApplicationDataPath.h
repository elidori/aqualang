#ifndef _APPLICATION_DATA_PATH_HEADER_
#define _APPLICATION_DATA_PATH_HEADER_

class ApplicationDataPath
{
public:
	ApplicationDataPath(LPCTSTR UserName);
	virtual ~ApplicationDataPath();

	void ComposePath(LPCTSTR FileName, _tstring &rPath);
	void ComposeRelativePath(LPCTSTR FileName, _tstring &rPath);
	void GetApplicationName(_tstring &rName);
	void GetApplicationPrivateName(_tstring &rName);

	static void DeleteDirectory(LPCTSTR Path, int DeleteDepth, LPCTSTR FilesFilter = _T("\\*"));
private:
	void BuildPath();
	bool TryApplicationDataPath(LPCTSTR ExecutableName, _tstring &rPath);
	void AppendUserName(_tstring &rPath) const;
private:
	_tstring m_UserName;
	_tstring m_Path;
	_tstring m_ApplicationPath;
	_tstring m_ApplicationFullPathName;
	_tstring m_ApplicationPrivateName;
};

#endif // _APPLICATION_DATA_PATH_HEADER_