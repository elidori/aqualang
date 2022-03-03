#ifndef _VERSION_UPDATER_HEADER_
#define _VERSION_UPDATER_HEADER_

#include "ProcessUtils.h"

class VersionNumber
{
public:
	VersionNumber(LPCSTR VersionStr);
	void Set(LPCSTR VersionStr);
	void Set(const VersionNumber &r);

	bool IsBeta() const					{ return m_fBeta; }
	bool IsGreater(const VersionNumber &r) const;
	bool IsDifferent(const VersionNumber &r) const;
	LPCTSTR GetSpecialString() const	{ return m_SpecialString.c_str(); }
private:
	bool m_fBeta;
	int m_Major;
	int m_Minor;
	int m_Build;
	_tstring m_SpecialString;
};

class IProgressInfoReceiver;
class FileDownloader;

class VersionUpdater
{
public:
	VersionUpdater();
	virtual ~VersionUpdater();

	bool Open(LPCSTR ServerUrl);
	void Close();

	bool CheckForUpdate(bool fCheckForBetaVersion, _tstring &FileName, _tstring &Version, _tstring &UpdateVersionDescription, _tstring &Checksum, bool &rfConnectionError); // blocking
	bool DownloadUpdate(LPCTSTR FileName, IProgressInfoReceiver *piReceiver, _tstring &ChecksumString); // blocking

	void Break();

	static bool RunUpgrader(LPCTSTR UserName);

	static bool Upgrade(LPCTSTR UserName);

	static void DeleteInstallationFolder();
private:
	static bool GetInstallationPath(LPCTSTR FileName, _tstring &rPath, bool fMustBeSubPath);
	bool GetCheckSum(LPCTSTR FileName, _tstring &ChecksumString);
private:
	ProcessEvent m_SingleTone;
	FileDownloader *m_pFileDownloader;
};

#endif // _VERSION_UPDATER_HEADER_