#include "Pch.h"
#include "VersionUpdater.h"
#include "FileDownloader.h"
#include "VersionInfo.h"
#include "ApplicationDataPath.h"
#include "AquaLangImpl.h"
#include "StringConvert.h"
#include "MD5Calculator.h"
#include "UserNameIndicator.h"

#define INSTALLATION_SUBDIR _T("Updates")
#define INSTALLATION_FILENAME _T("Update.msi")
#define INSTALLER_APP_NAME _T("AquaLangUpdater.exe")

#define MAX_FILES_TO_LIST 100

#define VERSION_UPDATER_SINGLETON_NAME _T("AquaLang updater singletone")
#define DOWNLOAD_CHUNK_SIZE_BYTES (100 * 1024)

#define USERNAME_QUERY_TIMEOUT 180 * 1000 // 3 minutes

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VersionNumber
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VersionNumber::VersionNumber(LPCSTR VersionStr)
	: m_fBeta(false),
	m_Major(0),
	m_Minor(0),
	m_Build(0)
{
	Set(VersionStr);
}

void VersionNumber::Set(LPCSTR VersionStr)
{
	char Text[256];
	strcpy_s(Text, sizeof(Text), "");

	sscanf_s(VersionStr, 
					"%d.%d.%d %s",
					&m_Major,
					&m_Minor,
					&m_Build,
					Text, sizeof(Text)
					);
	SetString(m_SpecialString, Text);
	m_fBeta = false;
	if(strstr(Text, "beta") != NULL)
	{
		m_fBeta = true;
	}
}

void VersionNumber::Set(const VersionNumber &r)
{
	m_fBeta = r.m_fBeta;

	m_Major = r.m_Major;
	m_Minor = r.m_Minor;
	m_Build = r.m_Build;

	m_SpecialString = r.m_SpecialString;
}

bool VersionNumber::IsGreater(const VersionNumber &r) const
{
	if(m_Major != r.m_Major)
		return m_Major > r.m_Major;
	if(m_Minor != r.m_Minor)
		return m_Minor > r.m_Minor;
	return m_Build > r.m_Build;
}

bool VersionNumber::IsDifferent(const VersionNumber &r) const
{
	return ((m_Major != r.m_Major) || (m_Minor != r.m_Minor) || (m_Build != r.m_Build));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VersionUpdater
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VersionUpdater::VersionUpdater()
	: m_pFileDownloader(NULL),
	m_SingleTone(VERSION_UPDATER_SINGLETON_NAME)
{
}

VersionUpdater::~VersionUpdater()
{
}

bool VersionUpdater::Open(LPCSTR ServerUrl)
{
#if false
	if(m_pFileDownloader)
		return false;

	if(m_SingleTone.Open())
	{
		// another instance is in progress
		return false;
	}
	if(!m_SingleTone.Create())
	{
		// Critcal failure?
		return false;
	}

	m_pFileDownloader = new FileDownloader(ServerUrl, 0);
	if(!m_pFileDownloader)
		return false;
#endif
	return true;
}

void VersionUpdater::Close()
{
	if(m_pFileDownloader)
	{
		delete m_pFileDownloader;
		m_pFileDownloader = NULL;
	}
	m_SingleTone.Destroy();
}

bool VersionUpdater::CheckForUpdate(bool fCheckForBetaVersion, _tstring &FileName, _tstring &Version, _tstring &UpdateVersionDescription, _tstring &Checksum, bool &rfConnectionError)
{
	rfConnectionError = false;

	if(!m_pFileDownloader)
		return false;

	FileDownloader::FileInfo FileInfoList[MAX_FILES_TO_LIST];
	size_t nFileInfos;
	_tstring ListChangeDate;
	int Res = m_pFileDownloader->ListFiles(FileInfoList, MAX_FILES_TO_LIST, &nFileInfos, ListChangeDate);
	if(Res != FD_SUCCESS)
	{
		rfConnectionError = true;
		return false;
	}
	
	index_t IndexOfMaxVersion = -1;
	VersionNumber MaxVersion("0.0.0");

	for(index_t i = 0; i < (index_t)nFileInfos; i++)
	{
		VersionNumber FileVersion(FileInfoList[i].Version);

		if(fCheckForBetaVersion || !FileVersion.IsBeta()) // filter beta version if not allowed
		{
			if(FileVersion.IsGreater(MaxVersion))
			{
				MaxVersion.Set(FileVersion);
				IndexOfMaxVersion = i;
			}
		}
	}
	if(IndexOfMaxVersion == -1)
		return false;
	
	VersionNumber CurrentVersion(VERSION_STRING);
	if(!MaxVersion.IsGreater(CurrentVersion))
		return false;
	SetString(FileName, FileInfoList[IndexOfMaxVersion].FileName);
	SetString(Version, FileInfoList[IndexOfMaxVersion].Version);
	
	FileDownloader::DetailedFileInfo DetFileInfo;
	Res = m_pFileDownloader->GetFileInformation(FileInfoList[IndexOfMaxVersion].FileName, DetFileInfo);
	if(Res != FD_SUCCESS)
		return false;
	SetString(Checksum, DetFileInfo.FileChecksum);

	// add '\r' to the description when needed
	LPSTR pRemainingString = DetFileInfo.Description;
	CHAR UpdatedString[MAX_FILE_DESCRIPTION_LEN * 2];
	memset(UpdatedString, '\0', sizeof(UpdatedString));
	while(pRemainingString)
	{
		LPSTR pNewLine = strchr(pRemainingString, '\n');
		if(pNewLine)
		{
			size_t Length =  strlen(UpdatedString);
			strncpy_s(UpdatedString + Length, sizeof(UpdatedString) - Length, pRemainingString, (int)(pNewLine - pRemainingString));
			if(pNewLine == pRemainingString || (pNewLine - 1)[0] != '\r')
			{
				strcat_s(UpdatedString, sizeof(UpdatedString), "\r");
			}
			strcat_s(UpdatedString, sizeof(UpdatedString), "\n");
			pRemainingString = pNewLine + 1;
		}
		else
		{
			strcat_s(UpdatedString, sizeof(UpdatedString), pRemainingString);
			break;
		}
	}
	SetString(UpdateVersionDescription, UpdatedString);

	return true;
}

bool VersionUpdater::DownloadUpdate(LPCTSTR FileName, IProgressInfoReceiver *piReceiver, _tstring &ChecksumString)
{
	if(!m_pFileDownloader)
		return false;

	_tstring Path;
	GetInstallationPath(INSTALLATION_FILENAME, Path, false);

	int Res = m_pFileDownloader->DownloadFile(FileName, Path.c_str(), DOWNLOAD_CHUNK_SIZE_BYTES, piReceiver);
	if(Res != FD_SUCCESS)
		return false;

	if(!GetCheckSum(Path.c_str(), ChecksumString))
		return false;

	return true;
}

void VersionUpdater::Break()
{
	if(m_pFileDownloader)
	{
		m_pFileDownloader->EndSession();
	}
}

bool VersionUpdater::RunUpgrader(LPCTSTR UserName)
{
	// first copy the executable
	_tstring ApplicationPath;
	ApplicationDataPath PathFinder(NULL);
	PathFinder.GetApplicationName(ApplicationPath);

	_tstring InstallationPath;
	GetInstallationPath(INSTALLER_APP_NAME, InstallationPath, false);

	if(!::CopyFile(ApplicationPath.c_str(), InstallationPath.c_str(), FALSE))
		return false;

	_tstring Args = _T("-upgrade");
	if(UserName && _tcslen(UserName) > 0)
	{
		Args += _T(" \"");
		Args += UserName;
		Args += _T("\"");
	}

	// then run the upgrader in a different process
	if(!Process::Create(InstallationPath.c_str(), Args.c_str(), _T("AquaLang upgrader")))
		return false;
	return true;
}

bool VersionUpdater::Upgrade(LPCTSTR UserName)
{
	UserNameIndicator UserNameHelper;
	if(!UserNameHelper.Open(UserName))
		return false;

	// find msiexec path
	TCHAR InstallerAppPath[MAX_PATH];
	HRESULT hr = SHGetFolderPath(NULL, CSIDL_SYSTEM, NULL, 0, InstallerAppPath);
	if(hr != S_OK)
		return false;
	_tcscat_s(InstallerAppPath, sizeof(InstallerAppPath) / sizeof(InstallerAppPath[0]), _T("\\msiexec.exe"));

	_tstring InstallationFilePath;
	GetInstallationPath(INSTALLATION_FILENAME, InstallationFilePath, false);

	TCHAR CommandSwitches[1024];
	_stprintf_s(
		CommandSwitches, sizeof(CommandSwitches) / sizeof(CommandSwitches[0]),
//		_T("/i \"%s\" /quiet"),
		_T("/i \"%s\" -qb"), // allow minimal ui for error reporting
		InstallationFilePath.c_str()
		);

//	::ShellExecute(NULL, _T("open"), InstallerAppPath, CommandSwitches, NULL, SW_SHOWNORMAL);

	if(!Process::Create(InstallerAppPath, CommandSwitches, _T("AquaLang version installation")))
		return false;

	if(!UserNameHelper.WaitForUserNameQuery(USERNAME_QUERY_TIMEOUT))
		return false;

	return true;
}

void VersionUpdater::DeleteInstallationFolder()
{
	_tstring InstallationPath;
	if(GetInstallationPath(_T(""), InstallationPath, true))
	{
		ApplicationDataPath PathFinder(NULL);
		ApplicationDataPath::DeleteDirectory(InstallationPath.c_str(), 0);
	}
	else
	{
		ApplicationDataPath PathFinder(NULL);
		PathFinder.ComposePath(_T(""), InstallationPath);

		::DeleteFile((InstallationPath + INSTALLATION_FILENAME).c_str());
		::DeleteFile((InstallationPath + INSTALLER_APP_NAME).c_str());
	}
}

bool VersionUpdater::GetInstallationPath(LPCTSTR FileName, _tstring &rPath, bool fMustBeSubPath)
{
	ApplicationDataPath PathFinder(NULL);
	PathFinder.ComposePath(_T(""), rPath);

	if(::CreateDirectory((rPath + INSTALLATION_SUBDIR).c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		rPath += INSTALLATION_SUBDIR;
		rPath += _T("\\");
	}
	else
	{
		if(fMustBeSubPath)
			return false;
	}
	rPath += FileName;
	return true;
}

bool VersionUpdater::GetCheckSum(LPCTSTR FileName, _tstring &ChecksumString)
{
	// calculate file checksum
	unsigned char digest[16];
	if(!MDFile(FileName, digest))
		return false;

	// convert checksum to string
	ChecksumString = _T("");
	for(int i = 0; i < sizeof(digest); i++)
	{
		TCHAR DigitStr[3];
		_stprintf_s(DigitStr, sizeof(DigitStr) / sizeof(DigitStr[0]), _T("%02X"), digest[i]);
		ChecksumString += DigitStr;
	}
	return true;
}